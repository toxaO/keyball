#include <stdint.h>
#include "quantum.h"      // for eeprom_* wrappers
#include "eeprom.h"
#include "eeconfig.h"

#include "keyball.h"
#include "keyball_kbpf.h"

// clamp helper borrowed from keyball.c so this module can remain standalone
#define _CONSTRAIN(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// constants defined in keyball.c
extern const uint16_t CPI_MAX;
extern const uint8_t  SCROLL_DIV_MAX;

////////////////////////////////////////////////////////////////////////////////
// EEPROM layout configuration
//
// KBPF_EE_ADDR points to the area used to store keyball profiles.  It tries
// to reuse VIA's custom area when available and otherwise falls back to the
// end of QMK's own config region.
////////////////////////////////////////////////////////////////////////////////

#define KBPF_EE_SIZE (sizeof(keyball_profiles_t))

#ifndef KBPF_EE_ADDR
#  ifdef VIA_ENABLE
#    include "via.h"
// Use VIA's custom EEPROM range
#    define KBPF_EE_ADDR (VIA_EEPROM_CUSTOM_CONFIG_ADDR)
_Static_assert(VIA_EEPROM_CUSTOM_CONFIG_SIZE >= KBPF_EE_SIZE,
               "VIA custom area is too small for keyball profiles");
#  else
// Compatible with various generations of QMK's eeconfig layout
#    if defined(EECONFIG_END)
#      define KBPF_EE_ADDR (EECONFIG_END)
#    elif defined(EECONFIG_SIZE)
#      define KBPF_EE_ADDR (EECONFIG_SIZE)
#    elif defined(EECONFIG_USER)
#      define KBPF_EE_ADDR (EECONFIG_USER + sizeof(uint32_t))
#    elif defined(EECONFIG_KB)
#      define KBPF_EE_ADDR (EECONFIG_KB + sizeof(uint32_t))
#    else
// Fallback for very old or special environments
#      define KBPF_EE_ADDR (512)
#    endif
#  endif
#endif

// Global instance that holds all per-OS profile values.
// Other modules access it through the extern declaration in keyball.h.
keyball_profiles_t kbpf;

//-------------------------------------------------------------------------
// Helpers
//-------------------------------------------------------------------------
// Small clamps are duplicated here for clarity and to keep this module
// self-contained.
static inline uint16_t clamp_cpi(uint16_t c) {
  if (c < 100)  c = 100;
  if (c > CPI_MAX) c = CPI_MAX;
  return c;
}
static inline uint8_t clamp_sdiv(uint8_t v) {
  if (v > SCROLL_DIV_MAX) v = SCROLL_DIV_MAX;
  return v;
}

// Set swipe-related defaults for a given profile structure.
static void kbpf_set_swipe_defaults(keyball_profiles_t *p){
  p->step     = KB_SW_STEP;
  p->deadzone = KB_SW_DEADZONE;
  p->freeze   = (KB_SWIPE_FREEZE_POINTER ? 1 : 0);
  p->sw_rst_ms= KB_SW_RST_MS;
  p->sc_dz    = KB_SCROLL_DEADZONE;
  p->sc_hyst  = KB_SCROLL_HYST;
}

//-------------------------------------------------------------------------
// Public API
//-------------------------------------------------------------------------

void kbpf_defaults(void) {
  // Populate kbpf with build-time defaults so the device works even with
  // completely blank EEPROM.
  for (int i = 0; i < 8; ++i) {
    kbpf.cpi[i]  = KEYBALL_CPI_DEFAULT;
    kbpf.sdiv[i] = KEYBALL_SCROLL_DIV_DEFAULT;
    kbpf.inv[i]  = (KEYBALL_SCROLL_INVERT != 0);

    kbpf.mv_gain_lo_fp[i] = (uint8_t)_CONSTRAIN(KEYBALL_MOVE_GAIN_LO_FP, 1, 255);
    kbpf.mv_th1[i]        = (uint8_t)_CONSTRAIN(KEYBALL_MOVE_TH1, 0, 63);
    kbpf.mv_th2[i]        = (uint8_t)_CONSTRAIN(KEYBALL_MOVE_TH2, 1, 63);
    if (kbpf.mv_th1[i] >= kbpf.mv_th2[i]) kbpf.mv_th1[i] = kbpf.mv_th2[i] - 1;

    // 新スクロールパラメータの初期値
    // Windows 通常風: interval=120, value=1 / mac 通常風: interval=120, value=120
    // ここでは OS 固有分岐は行わず、両方に無難な初期値を与える（ユーザーがOS別に調整保存）
    kbpf.sc_interval[i] = 120; // 1..200 程度を想定
    kbpf.sc_value[i]    = 1;   // 1..200 程度を想定（macは調整で120へ）
  }
  kbpf.magic    = KBPF_MAGIC;
  kbpf.version  = KBPF_VER_CUR;
  kbpf.reserved = 0;
  kbpf_set_swipe_defaults(&kbpf);
}

// Ensure loaded data is sane and migrate from older versions when needed.
static void kbpf_validate(void) {
  if (kbpf.magic != KBPF_MAGIC ||
      (kbpf.version != 1 && kbpf.version != 2 && kbpf.version != 3 && kbpf.version != 4 && kbpf.version != KBPF_VER_CUR)) {
    // Corrupted or unknown layout -> fall back to defaults.
    kbpf_defaults();
    return;
  }
  for (int i = 0; i < 8; ++i) {
    kbpf.cpi[i]  = clamp_cpi(kbpf.cpi[i] ? kbpf.cpi[i] : KEYBALL_CPI_DEFAULT);
    kbpf.sdiv[i] = clamp_sdiv(kbpf.sdiv[i]);
    kbpf.inv[i]  = kbpf.inv[i] ? 1 : 0;
    // 範囲ガード（既存データや未初期化対策）
    if (kbpf.sc_interval[i] == 0) kbpf.sc_interval[i] = 120;
    if (kbpf.sc_value[i]    == 0) kbpf.sc_value[i]    = 1;
    if (kbpf.sc_interval[i] > 200) kbpf.sc_interval[i] = 200;
    if (kbpf.sc_value[i]    > 200) kbpf.sc_value[i]    = 200;
  }
  if (kbpf.version == 1) {
    // Migrate from v1 -> v3 by populating all new fields with defaults.
    kbpf_set_swipe_defaults(&kbpf);
    // v1 にはスクロール interval/value が無いのでデフォルト付与
    for (int i = 0; i < 8; ++i) { kbpf.sc_interval[i] = 120; kbpf.sc_value[i] = 1; }
    kbpf.version = KBPF_VER_CUR; // actual write happens on save
  } else if (kbpf.version == 2) {
    // v2 lacked scroll deadzone/hysteresis
    kbpf.sc_dz   = KB_SCROLL_DEADZONE;
    kbpf.sc_hyst = KB_SCROLL_HYST;
    // interval/value 新規
    for (int i = 0; i < 8; ++i) { kbpf.sc_interval[i] = 120; kbpf.sc_value[i] = 1; }
    kbpf.version = KBPF_VER_CUR;
  } else if (kbpf.version == 3) {
    // v3 lacked swipe reset delay
    kbpf.sw_rst_ms = KB_SW_RST_MS;
    // interval/value 新規
    for (int i = 0; i < 8; ++i) { kbpf.sc_interval[i] = 120; kbpf.sc_value[i] = 1; }
    kbpf.version   = KBPF_VER_CUR;
  } else if (kbpf.version == 4) {
    // v4 → v5 の追加: interval/value
    for (int i = 0; i < 8; ++i) {
      if (kbpf.sc_interval[i] == 0) kbpf.sc_interval[i] = 120;
      if (kbpf.sc_value[i] == 0)    kbpf.sc_value[i]    = 1;
    }
    kbpf.version = KBPF_VER_CUR;
  }
  // Range guard for current fields
  if (kbpf.step < 1 || kbpf.step > 2000) kbpf.step = KB_SW_STEP;
  if (kbpf.deadzone > 32)                kbpf.deadzone = KB_SW_DEADZONE;
  kbpf.freeze &= 0x01;
  if (kbpf.sw_rst_ms > 250) kbpf.sw_rst_ms = KB_SW_RST_MS;
  if (kbpf.sc_dz > 32)    kbpf.sc_dz   = KB_SCROLL_DEADZONE;
  if (kbpf.sc_hyst > 32)  kbpf.sc_hyst = KB_SCROLL_HYST;
}

void kbpf_after_load_fixup(void) {
  // Public wrapper in case a caller wants to re-run validation after
  // manual edits to kbpf.
  kbpf_validate();
}

void kbpf_read(void) {
  // Read block from EEPROM and sanitize it.
  eeprom_read_block(&kbpf, (void*)KBPF_EE_ADDR, KBPF_EE_SIZE);
  kbpf_validate();
}

void kbpf_write(void) {
  // Persist entire profile structure back to EEPROM.
  eeprom_update_block(&kbpf, (void*)KBPF_EE_ADDR, KBPF_EE_SIZE);
}

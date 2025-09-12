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
static inline uint8_t clamp_step(uint8_t v) {
  if (v > SCROLL_DIV_MAX) v = SCROLL_DIV_MAX;
  return v;
}

// Set swipe-related defaults for a given profile structure.
static void kbpf_set_swipe_defaults(keyball_profiles_t *p){
  p->swipe_step       = KB_SW_STEP;
  p->swipe_deadzone   = KB_SW_DEADZONE;
  p->swipe_freeze     = (KB_SWIPE_FREEZE_POINTER ? 1 : 0);
  p->swipe_reset_ms   = KB_SW_RST_MS;
  p->scroll_deadzone  = KB_SCROLL_DEADZONE;
  p->scroll_hysteresis= KB_SCROLL_HYST;
}

//-------------------------------------------------------------------------
// Public API
//-------------------------------------------------------------------------

void kbpf_defaults(void) {
  // Populate kbpf with build-time defaults so the device works even with
  // completely blank EEPROM.
  for (int i = 0; i < 8; ++i) {
    kbpf.cpi[i]          = KEYBALL_CPI_DEFAULT;
    kbpf.scroll_step[i]  = KEYBALL_SCROLL_STEP_DEFAULT;
    kbpf.scroll_invert[i]= (KEYBALL_SCROLL_INVERT != 0);

    kbpf.move_gain_lo_fp[i] = (uint8_t)_CONSTRAIN(KEYBALL_MOVE_GAIN_LO_FP, 1, 255);
    kbpf.move_th1[i]        = (uint8_t)_CONSTRAIN(KEYBALL_MOVE_TH1, 0, 63);
    kbpf.move_th2[i]        = (uint8_t)_CONSTRAIN(KEYBALL_MOVE_TH2, 1, 63);
    if (kbpf.move_th1[i] >= kbpf.move_th2[i]) kbpf.move_th1[i] = kbpf.move_th2[i] - 1;

    // 新スクロールパラメータの初期値
    // 既定: interval=120, value=1（Windows/Linux 通常風）。
    // macOS はプリセット切替キーで {120,120} に設定可能。
    kbpf.scroll_interval[i] = 120; // 1..200 程度を想定
    kbpf.scroll_value[i]     = 1;   // 1..200 程度を想定（macは調整で120へ）
    kbpf.scroll_preset[i]    = 0;   // 0:{120,1} / 1:{1,1} / 2:{120,120}
  }
  kbpf.magic    = KBPF_MAGIC;
  kbpf.version  = KBPF_VER_CUR;
  kbpf.reserved = 0;
  kbpf_set_swipe_defaults(&kbpf);
}

// Ensure loaded data is sane. 互換は持たず、異なる版はデフォルトに初期化。
static void kbpf_validate(void) {
  if (kbpf.magic != KBPF_MAGIC || kbpf.version != KBPF_VER_CUR) {
    kbpf_defaults();
    return;
  }
  for (int i = 0; i < 8; ++i) {
    kbpf.cpi[i]           = clamp_cpi(kbpf.cpi[i] ? kbpf.cpi[i] : KEYBALL_CPI_DEFAULT);
    kbpf.scroll_step[i]   = clamp_step(kbpf.scroll_step[i]);
    kbpf.scroll_invert[i] = kbpf.scroll_invert[i] ? 1 : 0;
    // 範囲ガード
    if (kbpf.scroll_interval[i] == 0) kbpf.scroll_interval[i] = 120;
    if (kbpf.scroll_value[i]    == 0) kbpf.scroll_value[i]    = 1;
    if (kbpf.scroll_interval[i] > 200) kbpf.scroll_interval[i] = 200;
    if (kbpf.scroll_value[i]    > 200) kbpf.scroll_value[i]    = 200;
  }
  // Range guard for swipe fields
  if (kbpf.swipe_step < 1 || kbpf.swipe_step > 2000) kbpf.swipe_step = KB_SW_STEP;
  if (kbpf.swipe_deadzone > 32)                      kbpf.swipe_deadzone = KB_SW_DEADZONE;
  kbpf.swipe_freeze &= 0x01;
  if (kbpf.swipe_reset_ms > 250) kbpf.swipe_reset_ms = KB_SW_RST_MS;
  if (kbpf.scroll_deadzone > 32)    kbpf.scroll_deadzone   = KB_SCROLL_DEADZONE;
  if (kbpf.scroll_hysteresis > 32)  kbpf.scroll_hysteresis = KB_SCROLL_HYST;
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

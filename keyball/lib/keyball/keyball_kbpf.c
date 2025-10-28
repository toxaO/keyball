#include <stdint.h>
#include "quantum.h"      // for eeprom_* wrappers
#include "eeprom.h"
#include "eeconfig.h"
#include "os_detection.h" // for OS_MACOS index

#include "keyball.h"
#include "keyball_kbpf.h"
#ifdef HAPTIC_ENABLE
#    include "drivers/haptic/drv2605l.h"
#endif

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
// 優先: QMK の EECONFIG_KB 領域に保存して Vial/VIA のカスタム領域と分離する
#  include "quantum/nvm/eeprom/nvm_eeprom_eeconfig_internal.h"
#  define KBPF_EE_ADDR ((uintptr_t)EECONFIG_KB_DATABLOCK)
   // キーボード用データ領域に十分なサイズが確保されていることを検査
_Static_assert(EECONFIG_KB_DATA_SIZE >= sizeof(keyball_profiles_t),
               "EECONFIG_KB_DATA_SIZE is too small for keyball profiles");
#endif

// （古い QMK 互換用途のフォールバックは不要。KBPF_EE_ADDR は上で確定）

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
  p->swipe_step         = KBPF_DEFAULT_SWIPE_STEP;
  p->swipe_deadzone     = KBPF_DEFAULT_SWIPE_DEADZONE;
  p->swipe_freeze       = KBPF_DEFAULT_SWIPE_FREEZE ? 1u : 0u;
  p->swipe_reset_ms     = KBPF_DEFAULT_SWIPE_RESET_MS;
  p->swipe_haptic_mode  = KBPF_DEFAULT_SWIPE_HAPTIC_MODE;
  p->scroll_deadzone    = KBPF_DEFAULT_SCROLL_DEADZONE;
  p->scroll_hysteresis  = KBPF_DEFAULT_SCROLL_HYSTERESIS;
}

//-------------------------------------------------------------------------
// Public API
//-------------------------------------------------------------------------

void kbpf_defaults(void) {
  // Populate kbpf with build-time defaults so the device works even with
  // completely blank EEPROM.
  uint8_t base_step     = KBPF_DEFAULT_SCROLL_STEP ? KBPF_DEFAULT_SCROLL_STEP : 1;
  uint8_t base_interval = KBPF_DEFAULT_SCROLL_INTERVAL ? KBPF_DEFAULT_SCROLL_INTERVAL : 1;
  uint8_t base_value    = KBPF_DEFAULT_SCROLL_VALUE ? KBPF_DEFAULT_SCROLL_VALUE : 1;
  uint8_t base_preset   = (KBPF_DEFAULT_SCROLL_PRESET <= 2) ? KBPF_DEFAULT_SCROLL_PRESET : 2;
  for (int i = 0; i < 8; ++i) {
    kbpf.cpi[i]           = KBPF_DEFAULT_CPI;
    kbpf.scroll_step[i]   = clamp_step(base_step);
    kbpf.scroll_invert[i] = KBPF_DEFAULT_SCROLL_INVERT ? 1u : 0u;

    kbpf.move_gain_lo_fp[i] = (uint8_t)_CONSTRAIN(KBPF_DEFAULT_MOVE_GAIN_LO_FP, 1, 255);
    uint8_t th2_default      = (uint8_t)_CONSTRAIN(KBPF_DEFAULT_MOVE_TH2, 1, 63);
    uint8_t th1_default      = (uint8_t)_CONSTRAIN(KBPF_DEFAULT_MOVE_TH1, 0, th2_default ? th2_default - 1 : 0);
    kbpf.move_th2[i] = th2_default;
    kbpf.move_th1[i] = th1_default;

    // Scroll behaviour presets (OS-specific slots)
    kbpf.scroll_interval[i] = base_interval;
    kbpf.scroll_value[i]    = base_value;
    kbpf.scroll_preset[i]   = base_preset;
  }
  // macOS スロット(OS_MACOS=3)の既定は "MAC" とする
  {
    uint8_t m = (uint8_t)OS_MACOS; // 念のため範囲ガード
    if (m < 8) {
      uint8_t interval_mac = KBPF_DEFAULT_SCROLL_INTERVAL_MAC ? KBPF_DEFAULT_SCROLL_INTERVAL_MAC : base_interval;
      uint8_t value_mac    = KBPF_DEFAULT_SCROLL_VALUE_MAC ? KBPF_DEFAULT_SCROLL_VALUE_MAC : base_value;
      uint8_t preset_mac   = (KBPF_DEFAULT_SCROLL_PRESET_MAC <= 2) ? KBPF_DEFAULT_SCROLL_PRESET_MAC : base_preset;
      kbpf.scroll_interval[m] = interval_mac;
      kbpf.scroll_value[m]    = value_mac;
      kbpf.scroll_preset[m]   = preset_mac;
    }
  }
  kbpf.magic    = KBPF_MAGIC;
  kbpf.version  = KBPF_VER_CUR;
  kbpf.reserved = 0;
  // AML defaults
  kbpf.aml_enable    = KBPF_DEFAULT_AML_ENABLE ? 1u : 0u;
  kbpf.aml_layer     = KBPF_DEFAULT_AML_LAYER;
  kbpf.aml_timeout   = KBPF_DEFAULT_AML_TIMEOUT;
  kbpf.aml_threshold = KBPF_DEFAULT_AML_THRESHOLD;
  // Scroll snap: 既定は Vertical
#if KEYBALL_SCROLLSNAP_ENABLE == 2
  kbpf.scrollsnap_mode   = KBPF_DEFAULT_SCROLLSNAP_MODE;
  kbpf.scrollsnap_thr    = KBPF_DEFAULT_SCROLLSNAP_THR;
  kbpf.scrollsnap_rst_ms = KBPF_DEFAULT_SCROLLSNAP_RESET_MS;
#else
  kbpf.scrollsnap_mode = 0;
#endif
  kbpf_set_swipe_defaults(&kbpf);
  // Additional globals
  kbpf.scroll_hor_gain_pct = KBPF_DEFAULT_SCROLL_HOR_GAIN_PCT;
  kbpf.scroll_layer_enable = KBPF_DEFAULT_SCROLL_LAYER_ENABLE ? 1u : 0u;
  kbpf.scroll_layer_index  = (uint8_t)_CONSTRAIN(KBPF_DEFAULT_SCROLL_LAYER_INDEX, 0, 31);
  kbpf.default_layer       = (uint8_t)_CONSTRAIN(KBPF_DEFAULT_LAYER, 0, 31);
  kbpf.move_deadzone       = (uint8_t)_CONSTRAIN(KBPF_DEFAULT_MOVE_DEADZONE, 0, 32);
}

// Ensure loaded data is sane. 互換は持たず、異なる版はデフォルトに初期化。
static void kbpf_validate(void) {
  if (kbpf.magic != KBPF_MAGIC || kbpf.version != KBPF_VER_CUR) {
    kbpf_defaults();
    return;
  }
  for (int i = 0; i < 8; ++i) {
    kbpf.cpi[i]           = clamp_cpi(kbpf.cpi[i] ? kbpf.cpi[i] : KBPF_DEFAULT_CPI);
    uint8_t fallback_step = KBPF_DEFAULT_SCROLL_STEP ? KBPF_DEFAULT_SCROLL_STEP : 1;
    kbpf.scroll_step[i]   = clamp_step(kbpf.scroll_step[i] ? kbpf.scroll_step[i] : fallback_step);
    if (kbpf.scroll_step[i] == 0) kbpf.scroll_step[i] = 1;
    kbpf.scroll_invert[i] = kbpf.scroll_invert[i] ? 1 : 0;
    kbpf.move_gain_lo_fp[i] = (uint8_t)_CONSTRAIN(kbpf.move_gain_lo_fp[i] ? kbpf.move_gain_lo_fp[i] : KBPF_DEFAULT_MOVE_GAIN_LO_FP, 1, 255);
    kbpf.move_th2[i]        = (uint8_t)_CONSTRAIN(kbpf.move_th2[i] ? kbpf.move_th2[i] : KBPF_DEFAULT_MOVE_TH2, 1, 63);
    kbpf.move_th1[i]        = (uint8_t)_CONSTRAIN(kbpf.move_th1[i], 0, kbpf.move_th2[i] ? kbpf.move_th2[i] - 1 : 0);
    // 範囲ガード
    if (kbpf.scroll_interval[i] == 0) kbpf.scroll_interval[i] = KBPF_DEFAULT_SCROLL_INTERVAL;
    if (kbpf.scroll_interval[i] > 200) kbpf.scroll_interval[i] = 200;
    if (kbpf.scroll_value[i] == 0) kbpf.scroll_value[i] = KBPF_DEFAULT_SCROLL_VALUE;
    if (kbpf.scroll_value[i]    > 200) kbpf.scroll_value[i]    = 200;
    if (kbpf.scroll_preset[i] > 2u) kbpf.scroll_preset[i] = KBPF_DEFAULT_SCROLL_PRESET;
  }
  // Range guard for swipe fields
  if (kbpf.swipe_step < 1 || kbpf.swipe_step > 2000) kbpf.swipe_step = KBPF_DEFAULT_SWIPE_STEP;
  if (kbpf.swipe_deadzone > 32)                      kbpf.swipe_deadzone = KBPF_DEFAULT_SWIPE_DEADZONE;
  kbpf.swipe_freeze &= 0x01;
  if (kbpf.swipe_reset_ms > 250) kbpf.swipe_reset_ms = KBPF_DEFAULT_SWIPE_RESET_MS;
#ifdef HAPTIC_ENABLE
  if (kbpf.swipe_haptic_mode < 1u || kbpf.swipe_haptic_mode >= (uint8_t)DRV2605L_EFFECT_COUNT) {
    uint8_t fallback = KBPF_DEFAULT_SWIPE_HAPTIC_MODE;
    if (fallback < 1u || fallback >= (uint8_t)DRV2605L_EFFECT_COUNT) {
      fallback = DRV2605L_DEFAULT_MODE;
    }
    kbpf.swipe_haptic_mode = fallback;
  }
#else
  kbpf.swipe_haptic_mode = 0u;
#endif
  if (kbpf.scroll_deadzone > 32)    kbpf.scroll_deadzone   = KBPF_DEFAULT_SCROLL_DEADZONE;
  if (kbpf.scroll_hysteresis > 32)  kbpf.scroll_hysteresis = KBPF_DEFAULT_SCROLL_HYSTERESIS;
  // Move deadzone clamp
  if (kbpf.move_deadzone > 32) kbpf.move_deadzone = KBPF_DEFAULT_MOVE_DEADZONE;
  // AML
  kbpf.aml_enable = kbpf.aml_enable ? 1u : 0u;
  // layer は 0..31 程度（0xFFは未設定とみなす）
  if (kbpf.aml_layer != 0xFFu && kbpf.aml_layer > 31u) kbpf.aml_layer = KBPF_DEFAULT_AML_LAYER;
  // timeout: allow 100..9500 or special 60000 (HOLD)
  if (kbpf.aml_timeout == 60000u) {
    // keep
  } else {
    if (kbpf.aml_timeout < 100u) kbpf.aml_timeout = KBPF_DEFAULT_AML_TIMEOUT;
    if (kbpf.aml_timeout > 9500u) kbpf.aml_timeout = 9500u;
  }
  // threshold clamp (1..100 reasonable)
  if (kbpf.aml_threshold < 50u)  kbpf.aml_threshold = KBPF_DEFAULT_AML_THRESHOLD;
  if (kbpf.aml_threshold > 1000u) kbpf.aml_threshold = 1000u;
  // Scroll snap
  if (kbpf.scrollsnap_mode > 2u) kbpf.scrollsnap_mode = KBPF_DEFAULT_SCROLLSNAP_MODE;
  // Scroll snap params
  if (kbpf.scrollsnap_thr > 500u || kbpf.scrollsnap_thr == 0u) kbpf.scrollsnap_thr = KBPF_DEFAULT_SCROLLSNAP_THR;
  // 0 allows immediate FREE disable (never switch). typical 0..12
  if (kbpf.scrollsnap_rst_ms < 20u || kbpf.scrollsnap_rst_ms > 5000u) kbpf.scrollsnap_rst_ms = KBPF_DEFAULT_SCROLLSNAP_RESET_MS;
  // 水平スクロールゲイン（%）: 1..100 にクランプ
  if (kbpf.scroll_hor_gain_pct < 1u) kbpf.scroll_hor_gain_pct = KBPF_DEFAULT_SCROLL_HOR_GAIN_PCT;
  if (kbpf.scroll_hor_gain_pct > 100u) kbpf.scroll_hor_gain_pct = 100u;
  // Scroll layer settings
  kbpf.scroll_layer_enable = kbpf.scroll_layer_enable ? 1u : 0u;
  if (kbpf.scroll_layer_index > 31u) kbpf.scroll_layer_index = KBPF_DEFAULT_SCROLL_LAYER_INDEX;
  // Default layer: clamp 0..31
  if (kbpf.default_layer > 31u) kbpf.default_layer = KBPF_DEFAULT_LAYER;
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

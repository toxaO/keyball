#include <stdint.h>
#include "quantum.h"      // for eeprom_* wrappers
#include "eeprom.h"
#include "eeconfig.h"
#include "os_detection.h" // for OS_MACOS index

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
    // 既定: 各OSスロットは "fine"（微細）= {interval=1, value=1}
    // macOS スロットのみ後段で {120,120} に上書きする。
    kbpf.scroll_interval[i] = 1;   // fine 既定
    kbpf.scroll_value[i]     = 1;   // fine 既定
    kbpf.scroll_preset[i]    = 1;   // 0:{120,1} / 1:{1,1}(fine) / 2:{120,120}
  }
  // macOS スロット(OS_MACOS=3)の既定は "MAC" とする
  {
    uint8_t m = (uint8_t)OS_MACOS; // 念のため範囲ガード
    if (m < 8) {
      kbpf.scroll_interval[m] = 120;
      kbpf.scroll_value[m]    = 120;
      kbpf.scroll_preset[m]   = 2; // MAC 固定
    }
  }
  kbpf.magic    = KBPF_MAGIC;
  kbpf.version  = KBPF_VER_CUR;
  kbpf.reserved = 0;
  // AML: 初期は未設定（レイヤはOS依存とするため0xFFを入れておく）、無効化
  kbpf.aml_enable  = 0;
  kbpf.aml_layer   = 0xFFu; // sentinel: unset
  kbpf.aml_timeout = 3000;  // ms (default)
  // Scroll snap: 既定は Vertical
#if KEYBALL_SCROLLSNAP_ENABLE == 2
  kbpf.scrollsnap_mode = KEYBALL_SCROLLSNAP_MODE_VERTICAL;
#else
  kbpf.scrollsnap_mode = 0;
#endif
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
  // AML
  kbpf.aml_enable &= 1u;
  // layer は 0..31 程度（0xFFは未設定とみなす）
  if (kbpf.aml_layer != 0xFFu && kbpf.aml_layer > 31u) kbpf.aml_layer = 0;
  if (kbpf.aml_timeout < 300u) kbpf.aml_timeout = 300u;
  if (kbpf.aml_timeout > 5000u) kbpf.aml_timeout = 3000u;
  // Scroll snap
  if (kbpf.scrollsnap_mode > 2u) kbpf.scrollsnap_mode = 0u;
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

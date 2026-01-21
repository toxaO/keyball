/*
   Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
   */

#include <stdint.h>
#include "quantum.h"
#include "print.h"
#ifdef SPLIT_KEYBOARD
#    include "transactions.h"
#endif

#ifdef HAPTIC_ENABLE
#    include "haptic.h"
#    ifdef HAPTIC_DRV2605L
#        include "drivers/haptic/drv2605l.h"
#    endif
#    if defined(SPLIT_KEYBOARD) && defined(SPLIT_HAPTIC_ENABLE)
extern uint8_t split_haptic_play;
#    endif
bool get_haptic_enabled_key(uint16_t keycode, keyrecord_t *record) {
  (void)keycode;
  (void)record;
  // マウスクリックでの自動ハプティックは無効化
  return false;
}
#endif

#include "keyball.h"
#include "keyball_swipe.h"
#include "drivers/sensors/pmw33xx_common.h"
#if defined(POINTING_DEVICE_DRIVER_pmw3360) || defined(POINTING_DEVICE_DRIVER_pmw3389)
bool pmw33xx_check_signature(uint8_t sensor);
#endif
#include "pointing_device.h"
#include "os_detection.h"
#include "timer.h"
 

#ifdef DYNAMIC_KEYMAP_ENABLE
#    include "dynamic_keymap.h"
#endif

#if defined(VIAL_ENABLE) || defined(VIA_ENABLE)
#    include "via.h"
#    include "keyball_kbpf.h" // kbpf_write() を使用して初期レイアウト矯正の一度きり適用を永続化
#endif


// Anything above this value makes the cursor fly across the screen.
const uint16_t CPI_MAX        = 4000;
const uint8_t SCROLL_DIV_MAX = 7; // ST は 1..7 の 7 段階（4 が中心）

#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
const uint16_t AML_TIMEOUT_MIN = 100;    // ms
const uint32_t AML_TIMEOUT_MAX = 120000; // allow HOLD special (note: larger than uint16_t)
const uint16_t AML_TIMEOUT_QU  = 100;    // base quantization (OLED側で動的ステップ)
#endif

static const char BL = '\xB0'; // Blank indicator character


// OS 検出用
static uint8_t g_os_idx = 0;      // 決定した OS スロット
static bool    g_os_idx_init = false;



//////////////////////////////////////////////////////////////////////////////////////////
// old keyball config
keyball_t keyball = {
  .this_have_ball = false,
  .that_enable    = false,
  .that_have_ball = false,

  .this_motion = {0},
  .that_motion = {0},

  .scroll_mode = false,

  .pressing_keys = { BL, BL, BL, BL, BL, BL, 0 },
};

#ifdef HAPTIC_ENABLE
static uint8_t keyball_haptic_normalize_effect(uint8_t effect) {
#    ifdef HAPTIC_DRV2605L
  if (effect < 1u || effect >= (uint8_t)DRV2605L_EFFECT_COUNT) {
    effect = DRV2605L_DEFAULT_MODE;
  }
#    endif
  return effect;
}

static void keyball_haptic_play_local(uint8_t effect) {
#    ifdef HAPTIC_DRV2605L
  drv2605l_pulse(effect);
#    else
  (void)effect;
  haptic_play();
#    endif
}

static void keyball_haptic_queue_remote(uint8_t effect) {
#    ifdef SPLIT_KEYBOARD
  if (is_keyboard_master()) {
#        if defined(SPLIT_HAPTIC_ENABLE)
    split_haptic_play = effect;
#        endif
  } else {
    keyball_request_remote_haptic(effect);
  }
#    else
  (void)effect;
#    endif
}

void keyball_haptic_play_side(uint8_t effect, keyball_haptic_side_t sides) {
  if (sides == KEYBALL_HAPTIC_SIDE_NONE) {
    return;
  }
  if (!haptic_get_enable()) {
    return;
  }
  uint8_t normalized = keyball_haptic_normalize_effect(effect);
  bool     want_left  = (sides & KEYBALL_HAPTIC_SIDE_LEFT) != 0;
  bool     want_right = (sides & KEYBALL_HAPTIC_SIDE_RIGHT) != 0;
  bool     local_is_left = is_keyboard_left();

  if ((local_is_left && want_left) || (!local_is_left && want_right)) {
    keyball_haptic_play_local(normalized);
  }
#    ifdef SPLIT_KEYBOARD
  bool remote_needed = (!local_is_left && want_left) || (local_is_left && want_right);
  if (remote_needed) {
    keyball_haptic_queue_remote(normalized);
  }
#    endif
}

void keyball_haptic_play_left(uint8_t effect) {
  keyball_haptic_play_side(effect, KEYBALL_HAPTIC_SIDE_LEFT);
}

void keyball_haptic_play_right(uint8_t effect) {
  keyball_haptic_play_side(effect, KEYBALL_HAPTIC_SIDE_RIGHT);
}

void keyball_haptic_play_both(uint8_t effect) {
  keyball_haptic_play_side(effect, KEYBALL_HAPTIC_SIDE_BOTH);
}

static layer_state_t s_layer_haptic_state = 0;
static layer_state_t s_layer_haptic_default = 0;
static uint8_t       s_layer_haptic_last = 0xFFu;

static void keyball_layer_haptic_try_trigger(void) {
  uint8_t highest = get_highest_layer(s_layer_haptic_state | s_layer_haptic_default);
  if (s_layer_haptic_last == highest) {
    return;
  }
  s_layer_haptic_last = highest;
  if (!haptic_get_enable()) {
    return;
  }
  if (highest >= KEYBALL_LAYER_HAPTIC_SLOTS) {
    return;
  }
  keyball_layer_haptic_entry_t *entry = &kbpf.layer_haptic[highest];
  if ((entry->enable_mask & KEYBALL_LAYER_HAPTIC_ENABLE_LEFT) != 0u) {
    keyball_haptic_play_left(entry->left_effect);
  }
  if ((entry->enable_mask & KEYBALL_LAYER_HAPTIC_ENABLE_RIGHT) != 0u) {
    keyball_haptic_play_right(entry->right_effect);
  }
}

static uint8_t s_mod_haptic_last = 0;

static void keyball_mod_haptic_check(void) {
  uint8_t mods = get_mods() | get_oneshot_mods() | get_oneshot_locked_mods();
  uint8_t newly = (uint8_t)(mods & (uint8_t)~s_mod_haptic_last);
  s_mod_haptic_last = mods;
  if (newly == 0u || !haptic_get_enable()) {
    return;
  }
  static const uint8_t mod_bits[KEYBALL_MOD_HAPTIC_SLOTS] = {
      MOD_LSFT, MOD_LCTL, MOD_LGUI, MOD_LALT,
      MOD_RSFT, MOD_RCTL, MOD_RGUI, MOD_RALT,
  };
  for (uint8_t i = 0; i < KEYBALL_MOD_HAPTIC_SLOTS; ++i) {
    if ((newly & mod_bits[i]) == 0u) {
      continue;
    }
    keyball_layer_haptic_entry_t *entry = &kbpf.mod_haptic[i];
    if ((entry->enable_mask & KEYBALL_LAYER_HAPTIC_ENABLE_LEFT) != 0u) {
      keyball_haptic_play_left(entry->left_effect);
    }
    if ((entry->enable_mask & KEYBALL_LAYER_HAPTIC_ENABLE_RIGHT) != 0u) {
      keyball_haptic_play_right(entry->right_effect);
    }
  }
}

void keyball_layer_haptic_init(layer_state_t layer_state, layer_state_t default_layer_state) {
  s_layer_haptic_state   = layer_state;
  s_layer_haptic_default = default_layer_state;
  s_layer_haptic_last    = get_highest_layer(layer_state | default_layer_state);
}

void keyball_layer_haptic_on_layer_state(layer_state_t state) {
  s_layer_haptic_state = state;
  keyball_layer_haptic_try_trigger();
}

void keyball_layer_haptic_on_default_layer_state(layer_state_t state) {
  s_layer_haptic_default = state;
  keyball_layer_haptic_try_trigger();
}
#endif

#ifdef RGBLIGHT_ENABLE
static layer_state_t s_layer_led_state = 0;
static layer_state_t s_layer_led_default = 0;
static uint8_t       s_layer_led_last = 0xFFu;
static uint8_t       s_layer_led_last_index = 0;
static bool          s_layer_led_last_on = false;

static void keyball_layer_led_apply(bool force) {
  uint8_t highest = get_highest_layer(s_layer_led_state | s_layer_led_default);
  if (!force && s_layer_led_last == highest) {
    return;
  }
  if (s_layer_led_last_on) {
    keyball_led_set_hsv_at(0, 0, 0, s_layer_led_last_index);
    s_layer_led_last_on = false;
  }
  s_layer_led_last = highest;
  if (highest >= KEYBALL_LAYER_LED_SLOTS) {
    return;
  }
  keyball_layer_led_entry_t *entry = &kbpf.layer_led[highest];
  if (entry->val == 0u) {
    return;
  }
  keyball_led_set_hsv_at(entry->hue, entry->sat, entry->val, entry->index);
  s_layer_led_last_index = entry->index;
  s_layer_led_last_on = true;
}

void keyball_layer_led_init(layer_state_t layer_state, layer_state_t default_layer_state) {
  s_layer_led_state   = layer_state;
  s_layer_led_default = default_layer_state;
  s_layer_led_last    = 0xFFu;
  s_layer_led_last_on = false;
  keyball_layer_led_apply(true);
}

void keyball_layer_led_on_layer_state(layer_state_t state) {
  s_layer_led_state = state;
  keyball_layer_led_apply(false);
}

void keyball_layer_led_on_default_layer_state(layer_state_t state) {
  s_layer_led_default = state;
  keyball_layer_led_apply(false);
}

void keyball_layer_led_refresh(void) {
  keyball_layer_led_apply(true);
}
#endif

//////////////////////////////////////////////////////////////////////////////
// Hook points

__attribute__((weak)) void keyball_on_adjust_layout(keyball_adjust_t v) {}

//////////////////////////////////////////////////////////////////////////////
// Static utilities


static inline uint8_t decide_os_idx(void) {
  uint8_t raw = (uint8_t)detected_host_os();
  uint8_t idx = (raw < 8) ? raw : 0;  // 範囲外は 0 に丸め
  dprintf("keyball: detected_host_os raw=%u -> idx=%u\n", raw, idx);
  return idx;
}

static inline uint8_t osi(void) {
  if (!g_os_idx_init) {
    g_os_idx = decide_os_idx();
    g_os_idx_init = true;
  }
  return g_os_idx;
}

uint8_t keyball_os_idx(void) {
  return osi();
}

// KB-level: oneshot対応付き tap
static void tap_code16_with_oneshot_kb(uint16_t keycode) {
  uint8_t osm = get_oneshot_mods();
  if (osm) {
    add_weak_mods(osm);
    send_keyboard_report();
  }
  tap_code16(keycode);
  if (osm) {
    del_weak_mods(osm);
    send_keyboard_report();
    clear_oneshot_mods();
  }
}

// KB-level OS dependent tap helper (with oneshot support)
void tap_code16_os(uint16_t win, uint16_t mac, uint16_t ios, uint16_t linux, uint16_t unsure) {
  switch (detected_host_os()) {
    case OS_WINDOWS: if (win   != KC_NO) tap_code16_with_oneshot_kb(win);    break;
    case OS_MACOS:   if (mac   != KC_NO) tap_code16_with_oneshot_kb(mac);    break;
    case OS_IOS:     if (ios   != KC_NO) tap_code16_with_oneshot_kb(ios);    break;
    case OS_LINUX:   if (linux != KC_NO) tap_code16_with_oneshot_kb(linux);  break;
    case OS_UNSURE:  if (unsure!= KC_NO) tap_code16_with_oneshot_kb(unsure); break;
  }
}

static inline uint16_t clamp_cpi(uint16_t c) {
  if (c < 100)  c = 100;
  if (c > CPI_MAX) c = CPI_MAX;
  return c;
}
//////////////////////////////////////////////////////////////////////////////
// Pointing device driver

// 両手キーボードでのポインティングデバイス処理
static void motion_to_mouse(report_mouse_t *report, report_mouse_t *output, bool is_left, bool as_scroll) {
  if (keyball_swipe_is_active()) {
    keyball_swipe_apply(report, output, is_left);
    if (!kbpf.swipe_freeze) {
      keyball_on_apply_motion_to_mouse_move(report, output, is_left);
    }
  } else if (as_scroll) {
    keyball_on_apply_motion_to_mouse_scroll(report, output, is_left);
  } else {
    keyball_on_apply_motion_to_mouse_move(report, output, is_left);
  }
  report->x = 0;
  report->y = 0;
}

report_mouse_t pointing_device_task_combined_kb(report_mouse_t left_report, report_mouse_t right_report) {
  report_mouse_t output = {0};
  report_mouse_t *this_report = is_keyboard_left() ? &left_report : &right_report;
  report_mouse_t *that_report = is_keyboard_left() ? &right_report : &left_report;
  motion_to_mouse(this_report, &output, is_keyboard_left(), keyball.scroll_mode);
  motion_to_mouse(that_report, &output, !is_keyboard_left(), keyball.scroll_mode ^ keyball.this_have_ball);
  // store mouse report for OLED.
  keyball.last_mouse = output;
  return output;
}

//////////////////////////////////////////////////////////////////////////////
// Split RPC

#ifdef SPLIT_KEYBOARD

#    ifdef HAPTIC_ENABLE
typedef struct {
  uint8_t effect;
} keyball_haptic_sync_packet_t;

static void rpc_haptic_sync_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
  (void)out_buflen;
  (void)out_data;
  if (in_buflen < sizeof(keyball_haptic_sync_packet_t)) {
    return;
  }
  if (!haptic_get_enable()) {
    return;
  }

  const keyball_haptic_sync_packet_t *packet = (const keyball_haptic_sync_packet_t *)in_data;
#        ifdef HAPTIC_DRV2605L
  uint8_t effect = packet->effect;
  if (effect < 1u || effect >= (uint8_t)DRV2605L_EFFECT_COUNT) {
    effect = DRV2605L_DEFAULT_MODE;
  }
  drv2605l_pulse(effect);
#        else
  (void)packet;
  haptic_play();
#        endif
}
#    endif

static void rpc_get_info_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
  keyball_info_t info = {
    .ballcnt = keyball.this_have_ball ? 1 : 0,
  };
  *(keyball_info_t *)out_data = info;
  keyball_on_adjust_layout(KEYBALL_ADJUST_SECONDARY);
}

static void rpc_get_info_invoke(void) {
  static bool     negotiated = false;
  static uint32_t last_sync  = 0;
  static int      round      = 0;
  uint32_t        now        = timer_read32();
  if (negotiated || TIMER_DIFF_32(now, last_sync) < KEYBALL_TX_GETINFO_INTERVAL) {
    return;
  }
  last_sync = now;
  round++;
  keyball_info_t recv = {0};
  if (!transaction_rpc_exec(KEYBALL_GET_INFO, 0, NULL, sizeof(recv), &recv)) {
    if (round < KEYBALL_TX_GETINFO_MAXTRY) {
      dprintf("keyball:rpc_get_info_invoke: missed #%d\n", round);
      return;
    }
  }
  negotiated             = true;
  keyball.that_enable    = true;
  keyball.that_have_ball = recv.ballcnt > 0;
  dprintf("keyball:rpc_get_info_invoke: negotiated #%d %d\n", round, keyball.that_have_ball);

  // split keyboard negotiation completed.

  keyball_on_adjust_layout(KEYBALL_ADJUST_PRIMARY);
}

#endif

#if defined(HAPTIC_ENABLE) && defined(SPLIT_KEYBOARD)
void keyball_request_remote_haptic(uint8_t effect) {
  if (is_keyboard_master()) {
    return;
  }
  keyball_haptic_sync_packet_t packet = {
    .effect = effect,
  };
  (void)transaction_rpc_send(KEYBALL_SYNC_HAPTIC, sizeof(packet), &packet);
}
#else
void keyball_request_remote_haptic(uint8_t effect) {
  (void)effect;
}
#endif

//////////////////////////////////////////////////////////////////////////////
// OLED utility

#ifdef OLED_ENABLE
// clang-format off
const char PROGMEM code_to_name[] = {
  'a', 'b', 'c', 'd', 'e', 'f',  'g', 'h', 'i',  'j',
  'k', 'l', 'm', 'n', 'o', 'p',  'q', 'r', 's',  't',
  'u', 'v', 'w', 'x', 'y', 'z',  '1', '2', '3',  '4',
  '5', '6', '7', '8', '9', '0',  'R', 'E', 'B',  'T',
  '_', '-', '=', '[', ']', '\\', '#', ';', '\'', '`',
  ',', '.', '/',
};
// clang-format on
#endif


//////////////////////////////////////////////////////////////////////////////
// Public API functions

uint16_t keyball_get_cpi(void) {
  return clamp_cpi(kbpf.cpi[osi()]);
}


void keyball_set_cpi(uint16_t cpi) {
  cpi = clamp_cpi(cpi);
  uint8_t i = osi();
  kbpf.cpi[i] = cpi;
  dprintf("keyball: cpi set OS=%u -> %u\n", i, cpi);
  pointing_device_set_cpi_on_side(true,  cpi);
  pointing_device_set_cpi_on_side(false, cpi);
}

//////////////////////////////////////////////////////////////////////////////
// Keyboard hooks

void keyboard_post_init_kb(void) {
  debug_enable = true;
#ifdef SPLIT_KEYBOARD
  // register transaction handlers on secondary.
  if (!is_keyboard_master()) {
    transaction_register_rpc(KEYBALL_GET_INFO, rpc_get_info_handler);
    // その他のRPCは下で両側登録する
  }
#    ifdef HAPTIC_ENABLE
  transaction_register_rpc(KEYBALL_SYNC_HAPTIC, rpc_haptic_sync_handler);
#    endif
#endif

  // QMK 0.30.x 以降の公式 PMW33xx ドライバでは pmw33xx_init_ok は提供されない。
  // 代わりに pointing_device の初期化ステータスでボール有無を判定する。
  // QMKの世代差異に対応: vial-qmk 等の古い世代には status API が無い
#if defined(POINTING_DEVICE_STATUS_SUCCESS)
  keyball.this_have_ball = (pointing_device_get_status() == POINTING_DEVICE_STATUS_SUCCESS);
#else
#  if defined(POINTING_DEVICE_DRIVER_pmw3360) || defined(POINTING_DEVICE_DRIVER_pmw3389)
  keyball.this_have_ball = pmw33xx_check_signature(0);
#  else
  keyball.this_have_ball = true;
#  endif
#endif
  kbpf_defaults();        // まず既定値
  kbpf_read();            // EEPROMから上書き
                          // kbpf_after_load_fixup(); // 旧版からの移行処理

#ifdef HAPTIC_ENABLE
  haptic_set_mode(kbpf.swipe_haptic_mode);
  keyball_swipe_haptic_reset_sequence();
#endif

  keyball_set_cpi(keyball_get_cpi());
  keyball_set_scroll_div(keyball_get_scroll_div());
  g_move_gain_lo_fp = kbpf.move_gain_lo_fp[osi()];
  g_move_th1        = kbpf.move_th1[osi()];
  g_move_th2        = kbpf.move_th2[osi()];
  // 起動時にデフォルトレイヤーを適用
  default_layer_set((uint32_t)1u << kbpf.default_layer);
#ifdef HAPTIC_ENABLE
  keyball_layer_haptic_init(layer_state, default_layer_state);
#endif
#ifdef RGBLIGHT_ENABLE
  keyball_layer_led_init(layer_state, default_layer_state);
#endif
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
  // Apply persisted AML settings if available
  set_auto_mouse_enable(kbpf.aml_enable ? true : false);
  if (kbpf.aml_timeout >= 60000u) {
    set_auto_mouse_timeout(65535);
  } else {
    set_auto_mouse_timeout(kbpf.aml_timeout);
  }
  if (kbpf.aml_layer != 0xFFu) {
    set_auto_mouse_layer(kbpf.aml_layer);
  }
#endif
#if KEYBALL_SCROLLSNAP_ENABLE == 2
  keyball_set_scrollsnap_mode((keyball_scrollsnap_mode_t)kbpf.scrollsnap_mode);
#endif
    keyball_on_adjust_layout(KEYBALL_ADJUST_PENDING);
    keyboard_post_init_user();

    keyball_refresh_scroll_layer();
}

void matrix_scan_kb(void) {
#ifdef HAPTIC_ENABLE
  keyball_mod_haptic_check();
#endif
  matrix_scan_user();
}

// ---------------------------------------------------------------------------
// QMK Auto Mouse Layer: activation override
// QMK側のweakシンボルを上書きし、kbpf.aml_thresholdに基づいた発火条件へ置き換える。
// 既存のDebounce/Delay/Timeoutやキーイベント連携はQMK側ロジックのまま利用する。
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
__attribute__((used)) bool auto_mouse_activation(report_mouse_t mouse_report) {
#ifdef KEYBALL_AML_DISABLE_ACTIVATION
  // デバッグ用: 有効化判定を完全に無効化（確認用）
  return false;
#endif
  // Accumulate movement within a short window and compare against threshold
  static uint32_t last_ts = 0;
  static uint32_t acc = 0;
  uint32_t now = timer_read32();
  if (TIMER_DIFF_32(now, last_ts) > KEYBALL_AML_ACC_RESET_MS) {
    acc = 0; // reset accumulation after idle
  }
  last_ts = now;

  int16_t ax = mouse_report.x; if (ax < 0) ax = -ax;
  int16_t ay = mouse_report.y; if (ay < 0) ay = -ay;
  uint16_t mag = (ax > ay) ? (uint16_t)ax : (uint16_t)ay; // use max(|x|,|y|)
  // ノイズ抑制: 小さい入力は無視
  if (mag >= KEYBALL_AML_ACC_MIN_UNIT) {
    // スケール: 積算を遅らせる（div で割る、上限を設ける）
    uint16_t scaled = (uint16_t)(mag / KEYBALL_AML_ACC_DIV);
    if (scaled == 0) scaled = 1;
    if (scaled > 50) scaled = 50; // 1サンプルの寄与上限
    acc += (uint32_t)scaled;
  }
  uint16_t th = kbpf.aml_threshold ? kbpf.aml_threshold : 100;
  if (acc >= (uint32_t)th) {
    acc = 0; // consume
    return true;
  }
  return false;
}
#endif

#ifdef SPLIT_KEYBOARD
void housekeeping_task_kb(void) {
  if (is_keyboard_master()) {
    rpc_get_info_invoke();
  }
}
#endif

#ifdef OLED_ENABLE
static void pressing_keys_update(uint16_t keycode, keyrecord_t *record) {
  // Process only valid keycodes.
  if (keycode >= 4 && keycode < 57) {
    char value = pgm_read_byte(code_to_name + keycode - 4);
    char where = BL;
    if (!record->event.pressed) {
      // Swap `value` and `where` when releasing.
      where = value;
      value = BL;
    }
    // Rewrite the last `where` of pressing_keys to `value` .
    for (int i = 0; i < KEYBALL_OLED_MAX_PRESSING_KEYCODES; i++) {
      if (keyball.pressing_keys[i] == where) {
        keyball.pressing_keys[i] = value;
        break;
      }
    }
  }
}
#else
static inline void pressing_keys_update(uint16_t keycode, keyrecord_t *record) { (void)keycode; (void)record; }
#endif



#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
bool is_mouse_record_kb(uint16_t keycode, keyrecord_t* record) {
  switch (keycode) {
    case SCRL_MO:
      return true;
  }
  return is_mouse_record_user(keycode, record);
}
#endif

static bool s_swipe_has_origin = false;
static keypos_t s_swipe_origin;

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
  // store last keycode, row, and col for OLED
  keyball.last_kc  = keycode;
  keyball.last_pos = record->event.key;
  keyball.last_layer = get_highest_layer(layer_state);
  keyball.last_mods  = get_mods();

  pressing_keys_update(keycode, record);

  // ---- Debug: log key events (row/col/layer/keycode) ----
#ifdef DYNAMIC_KEYMAP_ENABLE
  {
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    uint8_t hl  = get_highest_layer(layer_state);
    uint16_t assigned = dynamic_keymap_get_keycode(hl, row, col);
    uprintf("EV kc=%04X assigned=%04X pressed=%u row=%u col=%u layer=%u mods=%02X os=%u\n",
            (unsigned)keycode,
            (unsigned)assigned,
            (unsigned)(record->event.pressed ? 1u : 0u),
            (unsigned)row,
            (unsigned)col,
            (unsigned)hl,
            (unsigned)get_mods(),
            (unsigned)detected_host_os());
    (void)assigned; (void)row; (void)col; (void)hl; // quiet unused when uprintf is stubbed
  }
#endif

  // OLEDデバッグUI用の矢印キー処理（消費したらホストへ送らない）
  if (keyball_oled_handle_ui_key(keycode, record)) {
    return false;
  }

  bool was_swipe_active = keyball_swipe_is_active();
  bool user_result = process_record_user(keycode, record);

  // スワイプの起点キー（物理位置）を記録（ユーザがbeginを呼んだ直後）
  if (record->event.pressed) {
    if (!was_swipe_active && keyball_swipe_is_active() && !s_swipe_has_origin) {
      s_swipe_origin = record->event.key;
      s_swipe_has_origin = true;
    }
  } else {
    // キー解放時に、起点の物理位置と一致したら必ずスワイプを終了
    if (s_swipe_has_origin && keyball_swipe_is_active()) {
      if (record->event.key.row == s_swipe_origin.row && record->event.key.col == s_swipe_origin.col) {
        keyball_swipe_end();
        s_swipe_has_origin = false;
      }
    }
  }

  if (!user_result) {
    return false;
  }

  // strip QK_MODS part.
  if (keycode >= QK_MODS && keycode <= QK_MODS_MAX) {
    keycode &= 0xff;
  }

  switch (keycode) {
#ifndef MOUSEKEY_ENABLE
    // process KC_MS_BTN1~8 by myself
    // See process_action() in quantum/action.c for details.
    case KC_MS_BTN1 ... KC_MS_BTN8: {
                                      extern void register_mouse(uint8_t mouse_keycode, bool pressed);
                                      register_mouse(keycode, record->event.pressed);
                                      // to apply QK_MODS actions, allow to process others.
                                      return true;
                                    }
#endif
  }

  return keyball_process_keycode(keycode, record);
}

// Disable functions keycode_config() and mod_config() in keycode_config.c to
// reduce size.  These functions are provided for customizing magic keycode.
// These two functions are mostly unnecessary if `MAGIC_KEYCODE_ENABLE = no` is
// set.
//
// If `MAGIC_KEYCODE_ENABLE = no` and you want to keep these two functions as
// they are, define the macro KEYBALL_KEEP_MAGIC_FUNCTIONS.
//
// See: https://docs.qmk.fm/#/squeezing_avr?id=magic-functions
//
#if !defined(MAGIC_KEYCODE_ENABLE) && !defined(KEYBALL_KEEP_MAGIC_FUNCTIONS)

uint16_t keycode_config(uint16_t keycode) {
  return keycode;
}

uint8_t mod_config(uint8_t mod) {
  return mod;
}

#endif

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

#include "keyball.h"
#include "drivers/sensors/pmw33xx_common.h"
#include "pointing_device.h"
#include "os_detection.h"
#include "timer.h"

#ifdef DYNAMIC_KEYMAP_ENABLE
#    include "dynamic_keymap.h"
#endif

#if defined(VIAL_ENABLE) || defined(VIA_ENABLE)
#    include "via.h"
#endif


// Anything above this value makes the cursor fly across the screen.
const uint16_t CPI_MAX        = 4000;
const uint8_t SCROLL_DIV_MAX = 7; // ST は 1..7 の 7 段階（4 が中心）

#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
const uint16_t AML_TIMEOUT_MIN = 300;
const uint16_t AML_TIMEOUT_MAX = 3000;
const uint16_t AML_TIMEOUT_QU  = 50;   // Quantization Unit
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

#    if (defined(VIA_ENABLE) || defined(VIAL_ENABLE))
  // レイアウト下位2bitが 0(None) のときだけ、既定を Right(=1) に設定。
  // これにより初期表示が None/Dual にならないようにする。
  uint32_t curr = via_get_layout_options();
  if ( (curr & 0x3u) == 0u ) {
    uint32_t next = (curr & ~0x3u) | 0x01u; // Right
    via_set_layout_options(next);
  }
#    endif

  keyball_on_adjust_layout(KEYBALL_ADJUST_PRIMARY);
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
  }
#endif

  // QMK 0.30.x 以降の公式 PMW33xx ドライバでは pmw33xx_init_ok は提供されない。
  // 代わりに pointing_device の初期化ステータスでボール有無を判定する。
  // QMKの世代差異に対応: vial-qmk 等の古い世代には status API が無い
#if defined(POINTING_DEVICE_STATUS_SUCCESS)
  keyball.this_have_ball = (pointing_device_get_status() == POINTING_DEVICE_STATUS_SUCCESS);
#else
  keyball.this_have_ball = true;
#endif
  kbpf_defaults();        // まず既定値
  kbpf_read();            // EEPROMから上書き
                          // kbpf_after_load_fixup(); // 旧版からの移行処理

  keyball_set_cpi(keyball_get_cpi());
  keyball_set_scroll_div(keyball_get_scroll_div());
  g_move_gain_lo_fp = kbpf.move_gain_lo_fp[osi()];
  g_move_th1        = kbpf.move_th1[osi()];
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
  // Apply persisted AML settings if available
  set_auto_mouse_enable(kbpf.aml_enable ? true : false);
  set_auto_mouse_timeout(kbpf.aml_timeout);
  if (kbpf.aml_layer != 0xFFu) {
    set_auto_mouse_layer(kbpf.aml_layer);
  }
#endif
#if KEYBALL_SCROLLSNAP_ENABLE == 2
  keyball_set_scrollsnap_mode((keyball_scrollsnap_mode_t)kbpf.scrollsnap_mode);
#endif
  keyball_on_adjust_layout(KEYBALL_ADJUST_PENDING);
  keyboard_post_init_user();
}

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

  pressing_keys_update(keycode, record);

  // ---- Debug: log key events (row/col/layer/keycode) ----
  {
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    uint8_t hl  = get_highest_layer(layer_state);
#    ifdef DYNAMIC_KEYMAP_ENABLE
    uint16_t assigned = dynamic_keymap_get_keycode(hl, row, col);
#    else
    uint16_t assigned = pgm_read_word(&keymaps[hl][row][col]);
#    endif
    uprintf("EV kc=%04X assigned=%04X pressed=%u row=%u col=%u layer=%u mods=%02X os=%u\n",
            (unsigned)keycode,
            (unsigned)assigned,
            (unsigned)(record->event.pressed ? 1u : 0u),
            (unsigned)row,
            (unsigned)col,
            (unsigned)hl,
            (unsigned)get_mods(),
            (unsigned)detected_host_os());
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

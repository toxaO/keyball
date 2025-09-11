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
#include "oled_driver.h"


// Anything above this value makes the cursor fly across the screen.
const uint16_t CPI_MAX        = 4000;
const uint8_t SCROLL_DIV_MAX = 7;

#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
const uint16_t AML_TIMEOUT_MIN = 100;
const uint16_t AML_TIMEOUT_MAX = 1000;
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

#ifdef OLED_ENABLE
static char to_1x(uint8_t x) {
  x &= 0x0f;
  return x < 10 ? x + '0' : x + 'a' - 10;
}
#endif

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
    if (!kbpf.freeze) {
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

#    ifdef VIA_ENABLE
  // adjust VIA layout options according to current combination.
  uint8_t  layouts = (keyball.this_have_ball ? (is_keyboard_left() ? 0x02 : 0x01) : 0x00) | (keyball.that_have_ball ? (is_keyboard_left() ? 0x01 : 0x02) : 0x00);
  uint32_t curr    = via_get_layout_options();
  uint32_t next    = (curr & ~0x3) | layouts;
  if (next != curr) {
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

void keyball_oled_render_ballinfo(void) {
#ifdef OLED_ENABLE
  oled_write_P(PSTR("CPI:"), false);
  {
    char b[6];
    snprintf(b, sizeof b, "%4u", (unsigned)keyball_get_cpi());
    oled_write(b, false);
  }
  oled_write_P(PSTR(" DIV:"), false);
  {
    char b[4];
    snprintf(b, sizeof b, "%u", (unsigned)keyball_get_scroll_div());
    oled_write(b, false);
  }
  oled_write_P(PSTR(" INV:"), false);
  oled_write_char(kbpf.inv[osi()] ? '1' : '0', false);
  oled_write_ln_P(PSTR(""), false);

#endif
}

void keyball_oled_render_keyinfo(void) {
#ifdef OLED_ENABLE
  // Format: `Key :  R{row}  C{col} K{kc} {name}{name}{name}`
  //
  // Where `kc` is lower 8 bit of keycode.
  // Where `name`s are readable labels for pressing keys, valid between 4 and 56.
  //
  // `row`, `col`, and `kc` indicates the last processed key,
  // but `name`s indicate unreleased keys in best effort.
  //
  // It is aligned to fit with output of keyball_oled_render_ballinfo().
  // For example:
  //
  //     Key :  R2  C3 K06 abc
  //     Ball:   0   0   0   0

  // "Key" Label
  oled_write_P(PSTR("Key \xB1"), false);

  // Row and column
  oled_write_char('\xB8', false);
  oled_write_char(to_1x(keyball.last_pos.row), false);
  oled_write_char('\xB9', false);
  oled_write_char(to_1x(keyball.last_pos.col), false);

  // Keycode
  oled_write_P(PSTR("\xBA\xBB"), false);
  oled_write_char(to_1x(keyball.last_kc >> 4), false);
  oled_write_char(to_1x(keyball.last_kc), false);

  // Pressing keys
  oled_write_P(PSTR("  "), false);
  oled_write(keyball.pressing_keys, false);
#endif
}

void keyball_oled_render_layerinfo(void) {
#ifdef OLED_ENABLE
  // Format: `Layer:{layer state}`
  //
  // Output example:
  //
  //     Layer:-23------------
  //
  oled_write_P(PSTR("L\xB6\xB7r\xB1"), false);
  for (uint8_t i = 1; i < 8; i++) {
    oled_write_char((layer_state_is(i) ? to_1x(i) : BL), false);
  }
  oled_write_char(' ', false);

#    ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
  oled_write_P(PSTR("\xC2\xC3"), false);
  if (get_auto_mouse_enable()) {
    oled_write_P(LFSTR_ON, false);
  } else {
    oled_write_P(LFSTR_OFF, false);
  }
  {
    char b[8];
    unsigned v = (unsigned)(get_auto_mouse_timeout() / 10);
    snprintf(b, sizeof b, "%u0", v);
    oled_write(b, false);
  }
#    else
  oled_write_P(PSTR("\xC2\xC3\xB4\xB5 ---"), false);
#    endif
#endif
}

void keyball_oled_render_ballsubinfo(void) {
#ifdef OLED_ENABLE
  char b[20];
  snprintf(b, sizeof b, "GL:%3ld  TH1:%2d", (long)g_move_gain_lo_fp, (int)g_move_th1);
  oled_write_ln(b, false);
#endif
}

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

  keyball.this_have_ball = pmw33xx_init_ok;
  kbpf_defaults();        // まず既定値
  kbpf_read();            // EEPROMから上書き
                          // kbpf_after_load_fixup(); // 旧版からの移行処理

  keyball_set_cpi(keyball_get_cpi());
  keyball_set_scroll_div(keyball_get_scroll_div());
  g_move_gain_lo_fp = kbpf.mv_gain_lo_fp[osi()];
  g_move_th1        = kbpf.mv_th1[osi()];
  g_scroll_deadzone   = kbpf.sc_dz;
  g_scroll_hysteresis = kbpf.sc_hyst;
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

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
  // store last keycode, row, and col for OLED
  keyball.last_kc  = keycode;
  keyball.last_pos = record->event.key;

  pressing_keys_update(keycode, record);

  if (!process_record_user(keycode, record)) {
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

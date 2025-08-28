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
#include <stdint.h>
#include "quantum.h"
#ifdef SPLIT_KEYBOARD
#    include "transactions.h"
#endif

#include "keyball.h"
#include "drivers/sensors/pmw33xx_common.h"
#include "pointing_device.h"

#include "os_detection.h"
#include "eeprom.h"
#include "eeconfig.h"

#include <string.h>

#define _CONSTRAIN(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
#define CONSTRAIN_XY(val)      (mouse_xy_report_t) _CONSTRAIN(val, MOUSE_REPORT_XY_MIN, MOUSE_REPORT_XY_MAX)
#define CONSTRAIN_HV(val)      (mouse_hv_report_t) _CONSTRAIN(val, MOUSE_REPORT_HV_MIN, MOUSE_REPORT_HV_MAX)

const uint16_t CPI_DEFAULT    = KEYBALL_CPI_DEFAULT;
// Anything above this value makes the cursor fly across the screen.
const uint16_t CPI_MAX        = 3000 + 1;
const uint8_t SCROLL_DIV_MAX = 7;

const uint16_t AML_TIMEOUT_MIN = 100;
const uint16_t AML_TIMEOUT_MAX = 1000;
const uint16_t AML_TIMEOUT_QU  = 50;   // Quantization Unit

static const char BL = '\xB0'; // Blank indicator character
// static const char LFSTR_ON[] PROGMEM = "\xB2\xB3";
// static const char LFSTR_OFF[] PROGMEM = "\xB4\xB5";

//////////////////////////////////////////////////////////////////////////////////////////
// new keyball profiles
// ---- Keyball専用 EEPROM ブロック（VIA不使用前提）----
typedef struct __attribute__((packed)) {
    uint32_t magic;     // 'KBP1'
    uint16_t version;   // 1
    uint16_t reserved;  // 0
    uint16_t cpi[8];    // 100..CPI_MAX
    uint8_t  sdiv[8];   // 1..SCROLL_DIV_MAX
    uint8_t  inv[8];    // 0/1
} keyball_profiles_t;

#define KBPF_MAGIC   0x4B425031u /* 'KBP1' */
#define KBPF_VERSION 1

_Static_assert(sizeof(keyball_profiles_t) == 40, "keyball_profiles_t must be 40 bytes");

// ---- 保存ブロックサイズ ----
#define KBPF_EE_SIZE (sizeof(keyball_profiles_t))

// ---- 保存先アドレス決定（VIA優先 → eeconfig系 → 最終フォールバック）----
#ifndef KBPF_EE_ADDR   // ← config.h で手動指定があればそれを使う
#  ifdef VIA_ENABLE
#    include "via.h"
     // VIA のカスタム領域を使用
#    define KBPF_EE_ADDR (VIA_EEPROM_CUSTOM_CONFIG_ADDR)
     _Static_assert(VIA_EEPROM_CUSTOM_CONFIG_SIZE >= KBPF_EE_SIZE,
                    "VIA custom area is too small for keyball profiles");
#  else
     // QMK の世代差を全部カバー
#    if defined(EECONFIG_END)
#      define KBPF_EE_ADDR (EECONFIG_END)
#    elif defined(EECONFIG_SIZE)
#      define KBPF_EE_ADDR (EECONFIG_SIZE)
#    elif defined(EECONFIG_USER)
       // ユーザ 32bit の直後
#      define KBPF_EE_ADDR (EECONFIG_USER + sizeof(uint32_t))
#    elif defined(EECONFIG_KB)
       // キーボード 32bit の直後
#      define KBPF_EE_ADDR (EECONFIG_KB + sizeof(uint32_t))
#    else
       // 最終フォールバック（古い/特殊環境用）。必要に応じて config.h で上書き推奨。
#      define KBPF_EE_ADDR (512)  // 例: AVR 1KB EEPROM想定で中腹に配置
#    endif
#  endif
#endif

static keyball_profiles_t kbpf;

//////////////////////////////////////////////////////////////////////////////////////////
// old keyball config
keyball_t keyball = {
    .this_have_ball = false,
    .that_enable    = false,
    .that_have_ball = false,

    .this_motion = {0},
    .that_motion = {0},

    .cpi_value   = 0,

    .scroll_mode = false,
    .scroll_div  = 0,

    .pressing_keys = { BL, BL, BL, BL, BL, BL, 0 },
};

//////////////////////////////////////////////////////////////////////////////
// Hook points

__attribute__((weak)) void keyball_on_adjust_layout(keyball_adjust_t v) {}

//////////////////////////////////////////////////////////////////////////////
// Static utilities

// clip2int8 clips an integer fit into int8_t.
static inline int8_t clip2int8(int16_t v) {
    return (v) < -127 ? -127 : (v) > 127 ? 127 : (int8_t)v;
}

#ifdef OLED_ENABLE
// static const char *format_4d(int16_t d) {
//     static char buf[5] = {0}; // max width (4) + NUL (1)
//     char        lead   = ' ';

//     if (d > 9999) {
//         d    = 9999;
//     } else if (d < -9999) {
//         d    = -9999;
//     }

//     if (d < 0) {
//         d    = -d;
//         lead = '-';
//     }
//     buf[3] = (d % 10) + '0';
//     d /= 10;
//     if (d == 0) {
//         buf[2] = lead;
//         lead   = ' ';
//     } else {
//         buf[2] = (d % 10) + '0';
//         d /= 10;
//     }
//     if (d == 0) {
//         buf[1] = lead;
//         lead   = ' ';
//     } else {
//         buf[1] = (d % 10) + '0';
//         d /= 10;
//     }
//     buf[0] = lead;
//     return buf;
// }

// static const char *format_cpi(int16_t d) {
//     static char buf[10] = {0};

//     sprintf(buf, "%5d", d);
//     return buf;
// }

static char to_1x(uint8_t x) {
    x &= 0x0f;
    return x < 10 ? x + '0' : x + 'a' - 10;
}
#endif

static void add_cpi(int16_t delta) {
    int16_t v = keyball_get_cpi() + delta;
    keyball_set_cpi(v < 1 ? 1 : v);
}

// static void add_scroll_div(int8_t delta) {
//     int8_t v = keyball_get_scroll_div() + delta;
//     keyball_set_scroll_div(v < 1 ? 1 : v);
// }

//////////////////////////////////////////////////////////////////////////////////////////
// config relative utilities

static inline uint8_t osi(void) {
    uint8_t i = (uint8_t)detected_host_os();
    return (i < 8) ? i : 0; // 範囲外は0
}
static inline uint16_t clamp_cpi(uint16_t c) {
    if (c < 100)  c = 100;
    if (c > CPI_MAX) c = CPI_MAX;
    return c;
}
static inline uint8_t clamp_sdiv(uint8_t v) {
    if (v < 1) v = 1;
    if (v > SCROLL_DIV_MAX) v = SCROLL_DIV_MAX;
    return v;
}

static void kb_profiles_defaults(void) {
    for (int i = 0; i < 8; ++i) {
        kbpf.cpi[i]  = KEYBALL_CPI_DEFAULT;
        kbpf.sdiv[i] = KEYBALL_SCROLL_DIV_DEFAULT;
        kbpf.inv[i]  = (KEYBALL_SCROLL_INVERT != 0);
    }
    kbpf.magic   = KBPF_MAGIC;
    kbpf.version = KBPF_VERSION;
    kbpf.reserved= 0;
}

static void kb_profiles_validate(void) {
    if (kbpf.magic != KBPF_MAGIC || kbpf.version != KBPF_VERSION) {
        kb_profiles_defaults();
        return;
    }
    for (int i = 0; i < 8; ++i) {
        kbpf.cpi[i]  = clamp_cpi(kbpf.cpi[i] ? kbpf.cpi[i] : KEYBALL_CPI_DEFAULT);
        kbpf.sdiv[i] = clamp_sdiv(kbpf.sdiv[i] ? kbpf.sdiv[i] : KEYBALL_SCROLL_DIV_DEFAULT);
        kbpf.inv[i]  = kbpf.inv[i] ? 1 : 0;
    }
}

static void kb_profiles_read(void) {
    // NG: eeprom_read_block(&kbpf, (void*)EECONFIG_KEYBALL_PROFILES_ADDR, sizeof(kbpf));
    eeprom_read_block(&kbpf, (void*)KBPF_EE_ADDR, KBPF_EE_SIZE);
    kb_profiles_validate();
}

static void kb_profiles_write(void) {
    // NG: eeprom_update_block(&kbpf, (void*)EECONFIG_KEYBALL_PROFILES_ADDR, sizeof(kbpf));
    eeprom_update_block(&kbpf, (void*)KBPF_EE_ADDR, KBPF_EE_SIZE);
}

//////////////////////////////////////////////////////////////////////////////
// Pointing device driver

void pointing_device_init_kb(void) {

}

__attribute__((weak)) void keyball_on_apply_motion_to_mouse_move(report_mouse_t *report, report_mouse_t *output, bool is_left) {
// #if KEYBALL_MODEL == 61 || KEYBALL_MODEL == 39 || KEYBALL_MODEL == 147 || KEYBALL_MODEL == 44
//     output->x = clip2int8(report->y);
//     output->y = clip2int8(report->x);
//     if (is_left) {
//         output->x = -output->x;
//         output->y = -output->y;
//     }
// #else
// #    error("unknown Keyball model")
// #endif
    output->x = report->x;
    output->y = report->y;

}

// 商/余り方式でスクロールを安定化（WHEEL_DELTA=120 を固定使用）
__attribute__((weak))
void keyball_on_apply_motion_to_mouse_scroll(report_mouse_t *report,
                                             report_mouse_t *output,
                                             bool is_left) {
    // ★ switchの外で宣言・初期化（必ずここから値を作る）
    int16_t out_x = 0;
    int16_t out_y = 0;

    switch (detected_host_os()) {
      case OS_MACOS: {
        // ブロックで囲むこと！（case直後の宣言を合法にする）
        enum { HIRES_DELTA = 120 };
        static int16_t acc_x = 0, acc_y = 0;
        static uint8_t last_sdiv = 0;        // 任意: 感度変更時に余りをリセット

        int16_t sx = (int16_t)report->x;
        int16_t sy = (int16_t)report->y;
        uint8_t sdiv = keyball_get_scroll_div();
        if (sdiv != last_sdiv) {             // 感度を変えたフレームの跳ねを抑制
            acc_x = 0; acc_y = 0;
            last_sdiv = sdiv;
        }

        acc_x += sx * sdiv;
        acc_y += sy * sdiv;

        out_x = acc_x / HIRES_DELTA;
        out_y = acc_y / HIRES_DELTA;

        acc_x -= out_x * HIRES_DELTA;
        acc_y -= out_y * HIRES_DELTA;
        break;
      }

      default: {
        // Windows/Linux 等はそのまま（必要なら sdiv を掛ける）
        uint8_t sdiv = keyball_get_scroll_div();
        out_x = (int16_t)report->x * sdiv;
        out_y = (int16_t)report->y * sdiv;
        break;
      }
    }

    // ---- モデルに合わせてホイールに反映（従来ロジック）----
#if KEYBALL_MODEL == 61 || KEYBALL_MODEL == 39 || KEYBALL_MODEL == 147 || KEYBALL_MODEL == 44
    output->h = -CONSTRAIN_HV(out_x);
    output->v =  CONSTRAIN_HV(out_y);
    if (is_left) {
        output->h = -output->h;
        output->v = -output->v;
    }
#else
#   error("unknown Keyball model")
#endif

    // ---- スナップ処理 ----
#if KEYBALL_SCROLLSNAP_ENABLE == 1
    uint32_t now = timer_read32();
    if (output->h != 0 || output->v != 0) {
        keyball.scroll_snap_last = now;
    } else if (TIMER_DIFF_32(now, keyball.scroll_snap_last) >= KEYBALL_SCROLLSNAP_RESET_TIMER) {
        keyball.scroll_snap_tension_h = 0;
    }
    if (abs(keyball.scroll_snap_tension_h) < KEYBALL_SCROLLSNAP_TENSION_THRESHOLD) {
        keyball.scroll_snap_tension_h += out_y;  // ★ out_y は必ず代入済み
        output->h = 0;
    }
#elif KEYBALL_SCROLLSNAP_ENABLE == 2
    switch (keyball_get_scrollsnap_mode()) {
        case KEYBALL_SCROLLSNAP_MODE_VERTICAL:   output->h = 0; break;
        case KEYBALL_SCROLLSNAP_MODE_HORIZONTAL: output->v = 0; break;
        default: break;
    }
#endif

// #if KEYBALL_SCROLL_INVERT == 1
//     output->h = -output->h;
//     output->v = -output->v;
// #endif

//////////////////////////////////////////////////////////////////////////////////////////
    // new invert
    // ランタイム反転（OS別）
    if (kbpf.inv[osi()]) {
        output->h = -output->h;
        output->v = -output->v;
    }

}
// old
// __attribute__((weak))
// void keyball_on_apply_motion_to_mouse_scroll(report_mouse_t *report,
//                                              report_mouse_t *output,
//                                              bool is_left) {
//     switch(detected_host_os()){
//       case OS_MACOS:
//         // 120 に固定（Windows の WHEEL_DELTA と同値）
//         enum { HIRES_DELTA = 120 };

//         // フレーム間で保持する蓄積バッファ
//         static int16_t acc_x = 0;
//         static int16_t acc_y = 0;

//         // 入力（必要なら係数で増減調整）
//         int16_t sx = (int16_t)report->x;
//         int16_t sy = (int16_t)report->y;

//         // お好みで感度調整したい場合は scroll_div を掛ける（1〜7）
//         uint8_t sdiv = keyball_get_scroll_div();  // 既存関数（1 が最小）
//         acc_x += sx * sdiv;
//         acc_y += sy * sdiv;

//         // 120 で割って「送る量（商）」を取り出し、余りは保持
//         int16_t out_x = acc_x / HIRES_DELTA;
//         int16_t out_y = acc_y / HIRES_DELTA;
//         acc_x -= out_x * HIRES_DELTA;
//         acc_y -= out_y * HIRES_DELTA;

//         break;

//       default:
//         // consume motion of trackball.
//         int16_t x = report->x;
//         int16_t y = report->y;

//     }

//       output->h = -CONSTRAIN_HV(out_x);
//       output->v =  CONSTRAIN_HV(out_y);
//       if (is_left) {
//           output->h = -output->h;
//           output->v = -output->v;
//       }

//     // ---- スナップ処理（既存）----
// #if KEYBALL_SCROLLSNAP_ENABLE == 1
//     uint32_t now = timer_read32();
//     if (output->h != 0 || output->v != 0) {
//         keyball.scroll_snap_last = now;
//     } else if (TIMER_DIFF_32(now, keyball.scroll_snap_last) >= KEYBALL_SCROLLSNAP_RESET_TIMER) {
//         keyball.scroll_snap_tension_h = 0;
//     }
//     if (abs(keyball.scroll_snap_tension_h) < KEYBALL_SCROLLSNAP_TENSION_THRESHOLD) {
//         // 旧実装互換：張力計算は今回の「商」を使う
//         keyball.scroll_snap_tension_h += out_y;
//         output->h = 0;
//     }
// #elif KEYBALL_SCROLLSNAP_ENABLE == 2
//     switch (keyball_get_scrollsnap_mode()) {
//         case KEYBALL_SCROLLSNAP_MODE_VERTICAL:   output->h = 0; break;
//         case KEYBALL_SCROLLSNAP_MODE_HORIZONTAL: output->v = 0; break;
//         default: break;
//     }
// #endif

// #if KEYBALL_SCROLL_INVERT == 1
//     output->h = -output->h;
//     output->v = -output->v;
// #endif
// }

// __attribute__((weak)) void keyball_on_apply_motion_to_mouse_scroll(report_mouse_t *report, report_mouse_t *output, bool is_left) {
//     // consume motion of trackball.
//     int16_t x = report->x * keyball.scroll_div;
//     int16_t y = report->y * keyball.scroll_div;

//     // apply to mouse report.
// #if KEYBALL_MODEL == 61 || KEYBALL_MODEL == 39 || KEYBALL_MODEL == 147 || KEYBALL_MODEL == 44
//     output->h = -CONSTRAIN_HV(x);
//     output->v = CONSTRAIN_HV(y);
//     if (is_left) {
//         output->h = -output->h;
//         output->v = -output->v;
//     }

// #else
// #    error("unknown Keyball model")
// #endif

//     // Scroll snapping
// #if KEYBALL_SCROLLSNAP_ENABLE == 1
//     // Old behavior up to 1.3.2)
//     uint32_t now = timer_read32();
//     if (output->h != 0 || output->v != 0) {
//         keyball.scroll_snap_last = now;
//     } else if (TIMER_DIFF_32(now, keyball.scroll_snap_last) >= KEYBALL_SCROLLSNAP_RESET_TIMER) {
//         keyball.scroll_snap_tension_h = 0;
//     }
//     if (abs(keyball.scroll_snap_tension_h) < KEYBALL_SCROLLSNAP_TENSION_THRESHOLD) {
//         keyball.scroll_snap_tension_h += y;
//         output->h = 0;
//     }
// #elif KEYBALL_SCROLLSNAP_ENABLE == 2
//     // New behavior
//     switch (keyball_get_scrollsnap_mode()) {
//         case KEYBALL_SCROLLSNAP_MODE_VERTICAL:
//             output->h = 0;
//             break;
//         case KEYBALL_SCROLLSNAP_MODE_HORIZONTAL:
//             output->v = 0;
//             break;
//         default:
//             // pass by without doing anything
//             break;
//     }
// #endif
// }

static void motion_to_mouse(report_mouse_t *report, report_mouse_t *output, bool is_left, bool as_scroll) {
    if (as_scroll) {
        keyball_on_apply_motion_to_mouse_scroll(report, output, is_left);
    } else {
        keyball_on_apply_motion_to_mouse_move(report, output, is_left);
    }

    // clear motion
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
    //////////////////////////////////////////////////////////////////////////////////////////
    //old

    // // Format: `Ball:{mouse x}{mouse y}{mouse h}{mouse v}`
    // //
    // // Output example:
    // //
    // //     Ball: -12  34   0   0

    // // 1st line, "Ball" label, mouse x, y, h, and v.
    // oled_write_P(PSTR("Ball\xB1"), false);
    // oled_write(format_4d(keyball.last_mouse.x), false);
    // oled_write(format_4d(keyball.last_mouse.y), false);
    // oled_write(format_4d(keyball.last_mouse.h), false);
    // oled_write(format_4d(keyball.last_mouse.v), false);

    // // 2nd line, empty label and CPI
    // oled_write_P(PSTR("    \xB1\xBC\xBD"), false);
    // oled_write(format_cpi(keyball_get_cpi()), false);
    // oled_write_char(' ', false);

    // // indicate scroll snap mode: "VT" (vertical), "HN" (horiozntal), and "SCR" (free)
// #if 1 && KEYBALL_SCROLLSNAP_ENABLE == 2
    // switch (keyball_get_scrollsnap_mode()) {
    //     case KEYBALL_SCROLLSNAP_MODE_VERTICAL:
    //         oled_write_P(PSTR("VT"), false);
    //         break;
    //     case KEYBALL_SCROLLSNAP_MODE_HORIZONTAL:
    //         oled_write_P(PSTR("HO"), false);
    //         break;
    //     default:
    //         oled_write_P(PSTR("\xBE\xBF"), false);
    //         break;
    // }
// #else
    // oled_write_P(PSTR("\xBE\xBF"), false);
// #endif
    // // indicate scroll mode: on/off
    // if (keyball.scroll_mode) {
    //     oled_write_P(LFSTR_ON, false);
    // } else {
    //     oled_write_P(LFSTR_OFF, false);
    // }

    // // indicate scroll divider:
    // oled_write_P(PSTR(" \xC0\xC1"), false);
    // // oled_write_char('0' + keyball_get_scroll_div(), false);
    // {
    // char buf[4];
    // snprintf(buf, sizeof(buf), "%u", (unsigned)keyball_get_scroll_div());
    // oled_write(buf, false);
    // }

    /////////////////////////////////////////////////////////////////////////////////////////
    // new
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

void keyball_oled_render_ballsubinfo(void) {
#ifdef OLED_ENABLE
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

    oled_write(format_4d(get_auto_mouse_timeout() / 10) + 1, false);
    oled_write_char('0', false);
#    else
    oled_write_P(PSTR("\xC2\xC3\xB4\xB5 ---"), false);
#    endif
#endif
}

//////////////////////////////////////////////////////////////////////////////
// Public API functions

bool keyball_get_scroll_mode(void) {
    return keyball.scroll_mode;
}

void keyball_set_scroll_mode(bool mode) {
    if (mode != keyball.scroll_mode) {
        keyball.scroll_mode_changed = timer_read32();
    }
    keyball.scroll_mode = mode;
}

keyball_scrollsnap_mode_t keyball_get_scrollsnap_mode(void) {
#if KEYBALL_SCROLLSNAP_ENABLE == 2
    return keyball.scrollsnap_mode;
#else
    return 0;
#endif
}

void keyball_set_scrollsnap_mode(keyball_scrollsnap_mode_t mode) {
#if KEYBALL_SCROLLSNAP_ENABLE == 2
    keyball.scrollsnap_mode = mode;
#endif
}

uint16_t keyball_get_cpi(void) {
    return clamp_cpi(kbpf.cpi[osi()]);
}
void keyball_set_cpi(uint16_t cpi) {
    cpi = clamp_cpi(cpi);
    kbpf.cpi[osi()] = cpi;
    keyball.cpi_value = cpi; // 互換
    dprintf("set cpi(OS=%u): %u\n", osi(), cpi);
    pointing_device_set_cpi_on_side(true,  keyball.cpi_value);
    pointing_device_set_cpi_on_side(false, keyball.cpi_value);
}

uint8_t keyball_get_scroll_div(void) {
    return clamp_sdiv(kbpf.sdiv[osi()]);
}
void keyball_set_scroll_div(uint8_t div) {
    div = clamp_sdiv(div);
    kbpf.sdiv[osi()] = div;
    keyball.scroll_div = div; // 互換
}

//////////////////////////////////////////////////////////////////////////////////////////
// old getter and setter
// uint8_t keyball_get_scroll_div(void) {
//     uint8_t v = keyball.scroll_div;
//     // v==0 のときにデフォルトを使いたい設計ならここで決め打ち
//     // （デフォルトが 0 でも落ちないように 1 を最低保証）
// #if defined(KEYBALL_SCROLL_DIV_DEFAULT)
//     if (v < 1) v = (KEYBALL_SCROLL_DIV_DEFAULT < 1) ? 1 : KEYBALL_SCROLL_DIV_DEFAULT;
// #else
//     if (v < 1) v = 1;
// #endif
//     if (v > SCROLL_DIV_MAX) v = SCROLL_DIV_MAX;
//     return v;
// }

// void keyball_set_scroll_div(uint8_t div) {
//     if (div < 1) div = 1;
//     if (div > SCROLL_DIV_MAX) div = SCROLL_DIV_MAX;
//     keyball.scroll_div = div;
// }


// uint16_t keyball_get_cpi(void) {
//     return keyball.cpi_value == 0 ? CPI_DEFAULT : keyball.cpi_value;
// }

// void keyball_set_cpi(uint16_t cpi) {
//     if (cpi > CPI_MAX + 1) {
//         cpi = CPI_MAX;
//     }

//     keyball.cpi_value   = cpi;
//     dprintf("set cpi: %u\n", keyball.cpi_value);
//     pointing_device_set_cpi_on_side(true, keyball.cpi_value);
//     pointing_device_set_cpi_on_side(false, keyball.cpi_value);
//     dprintf("actual after cpi: %u\n", pointing_device_get_cpi());
// }

//////////////////////////////////////////////////////////////////////////////
// Keyboard hooks

void keyboard_post_init_kb(void) {
#ifdef SPLIT_KEYBOARD
    // register transaction handlers on secondary.
    if (!is_keyboard_master()) {
        transaction_register_rpc(KEYBALL_GET_INFO, rpc_get_info_handler);
    }
#endif

    keyball.this_have_ball = pmw33xx_init_ok;
    //////////////////////////////////////////////////////////////////////////////////////////
    // new config
    kb_profiles_defaults(); // まず既定値
    kb_profiles_read();     // EEPROMから上書き

    keyball_set_cpi(kbpf.cpi[osi()]);
    keyball_set_scroll_div(kbpf.sdiv[osi()]);

    //////////////////////////////////////////////////////////////////////////////////////////
    // old config
    // keyball_set_cpi(CPI_DEFAULT);
    // keyball_set_scroll_div(KEYBALL_SCROLL_DIV_DEFAULT);  // 最低でも 1 以上になる

    // // read keyball configuration from EEPROM
    // if (eeconfig_is_enabled()) {
    //     keyball_config_t c = {.raw = eeconfig_read_kb()};
    //     printf("read cpi: %u, scroll_div: %u\n", c.cpi, c.sdiv);
    //     if (c.cpi < 1) {
    //         c.cpi = CPI_DEFAULT;
    //     }
    //     keyball_set_cpi(c.cpi);
    //     keyball_set_scroll_div(c.sdiv);
// #ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
    //     set_auto_mouse_enable(c.amle);
    //     set_auto_mouse_timeout(c.amlto == 0 ? AUTO_MOUSE_TIME : (c.amlto + 1) * AML_TIMEOUT_QU);
// #endif
// #if KEYBALL_SCROLLSNAP_ENABLE == 2
    //     keyball_set_scrollsnap_mode(c.ssnap);
// #endif
    // }

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

        case SCRL_MO:
            keyball_set_scroll_mode(record->event.pressed);
            // process_auto_mouse may use this in future, if changed order of
            // processes.
            return true;
    }

    // process events which works on pressed only.
    if (record->event.pressed) {
        switch (keycode) {
//////////////////////////////////////////////////////////////////////////////////////////
          // new
            case KBC_RST:
                kb_profiles_defaults();
                keyball_set_cpi(kbpf.cpi[osi()]);
                keyball_set_scroll_div(kbpf.sdiv[osi()]);
                kb_profiles_write();
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
                set_auto_mouse_enable(false);
                set_auto_mouse_timeout(AUTO_MOUSE_TIME);
#endif
                break;

            case KBC_SAVE:
                kb_profiles_write();  // OSごとの全データを一括保存
                dprintf("KB profiles saved (magic=0x%08lX ver=%u)\n",
                        (unsigned long)kbpf.magic, kbpf.version);
                break;

            case SCRL_DVI:
                keyball_set_scroll_div(keyball_get_scroll_div() + 1);
                break;
            case SCRL_DVD:
                keyball_set_scroll_div(keyball_get_scroll_div() - 1);
                break;

            case SCRL_INV: { // OS別反転トグル
                uint8_t i = osi();
                kbpf.inv[i] = !kbpf.inv[i];
                dprintf("invert toggle OS=%u -> %u\n", i, kbpf.inv[i]);
            } break;

//////////////////////////////////////////////////////////////////////////////////////////
            //old
            // case KBC_RST:
            //     keyball_set_cpi(0);
            //     keyball_set_scroll_div(KEYBALL_SCROLL_DIV_DEFAULT);
// #ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
            //     set_auto_mouse_enable(false);
            //     set_auto_mouse_timeout(AUTO_MOUSE_TIME);
// #endif
            //     break;
            // case KBC_SAVE: {
            //     keyball_config_t c = {
            //         .cpi   = keyball.cpi_value,
            //         .sdiv  = keyball.scroll_div,
// #ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
            //         .amle  = get_auto_mouse_enable(),
            //         .amlto = (get_auto_mouse_timeout() / AML_TIMEOUT_QU) - 1,
// #endif
// #if KEYBALL_SCROLLSNAP_ENABLE == 2
            //         .ssnap = keyball_get_scrollsnap_mode(),
// #endif
            //     };
            //     eeconfig_update_kb(c.raw);
            // } break;

            case CPI_I100:
                add_cpi(100);
                break;
            case CPI_D100:
                add_cpi(-100);
                break;
            case CPI_I1K:
                add_cpi(1000);
                break;
            case CPI_D1K:
                add_cpi(-1000);
                break;

            case SCRL_TO:
                keyball_set_scroll_mode(!keyball.scroll_mode);
                break;
            // case SCRL_DVI:
            //     add_scroll_div(1);
            //     break;
            // case SCRL_DVD:
            //     add_scroll_div(-1);
            //     break;

#if KEYBALL_SCROLLSNAP_ENABLE == 2
            case SSNP_HOR:
                keyball_set_scrollsnap_mode(KEYBALL_SCROLLSNAP_MODE_HORIZONTAL);
                break;
            case SSNP_VRT:
                keyball_set_scrollsnap_mode(KEYBALL_SCROLLSNAP_MODE_VERTICAL);
                break;
            case SSNP_FRE:
                keyball_set_scrollsnap_mode(KEYBALL_SCROLLSNAP_MODE_FREE);
                break;
#endif

#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
            case AML_TO:
                set_auto_mouse_enable(!get_auto_mouse_enable());
                break;
            case AML_I50:
                {
                    uint16_t v = get_auto_mouse_timeout() + 50;
                    set_auto_mouse_timeout(MIN(v, AML_TIMEOUT_MAX));
                }
                break;
            case AML_D50:
                {
                    uint16_t v = get_auto_mouse_timeout() - 50;
                    set_auto_mouse_timeout(MAX(v, AML_TIMEOUT_MIN));
                }
                break;
#endif

            default:
                return true;
        }
        return false;
    }

    return true;
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

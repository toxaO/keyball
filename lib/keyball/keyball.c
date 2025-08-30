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
#include "eeprom.h"
#include "eeconfig.h"

#include <string.h>

#define _CONSTRAIN(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
#define CONSTRAIN_HV(val)      (mouse_hv_report_t) _CONSTRAIN(val, MOUSE_REPORT_HV_MIN, MOUSE_REPORT_HV_MAX)

// Anything above this value makes the cursor fly across the screen.
const uint16_t CPI_MAX        = 4000;
const uint8_t SCROLL_DIV_MAX = 7;

#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
const uint16_t AML_TIMEOUT_MIN = 100;
const uint16_t AML_TIMEOUT_MAX = 1000;
const uint16_t AML_TIMEOUT_QU  = 50;   // Quantization Unit
#endif

static const char BL = '\xB0'; // Blank indicator character

static int32_t g_move_gain_lo_fp = KEYBALL_MOVE_GAIN_LO_FP;
static int16_t g_move_th1 = KEYBALL_MOVE_TH1;  // th2 は固定でもOK。要れば同様に可変化。

static uint8_t g_os_idx = 0;      // 決定した OS スロット
static bool    g_os_idx_init = false;

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
    uint8_t  mv_gain_lo_fp[8]; // 固定小数点(1/256)。16..255 推奨
    uint8_t  mv_th1[8];        // 0..(mv_th2-1)
    uint8_t  mv_th2[8];        // 1..63 など適当な上限（今回は固定でも可）
} keyball_profiles_t;

#define KBPF_MAGIC   0x4B425031u /* 'KBP1' */
#define KBPF_VERSION 2
_Static_assert(sizeof(keyball_profiles_t) == 40 + 24, "update size expectation if you change fields");

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

static void add_cpi(int16_t delta) {
    int16_t v = keyball_get_cpi() + delta;
    keyball_set_cpi(v < 1 ? 1 : v);
}

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

        kbpf.mv_gain_lo_fp[i] = (uint8_t)_CONSTRAIN(KEYBALL_MOVE_GAIN_LO_FP, 1, 255);
        kbpf.mv_th1[i]        = (uint8_t)_CONSTRAIN(KEYBALL_MOVE_TH1, 0, 63);
        kbpf.mv_th2[i]        = (uint8_t)_CONSTRAIN(KEYBALL_MOVE_TH2, 1, 63);
        if (kbpf.mv_th1[i] >= kbpf.mv_th2[i]) kbpf.mv_th1[i] = kbpf.mv_th2[i] - 1;
    }
    kbpf.magic   = KBPF_MAGIC;
    kbpf.version = KBPF_VERSION;
    kbpf.reserved= 0;
}

static void kb_profiles_validate(void) {
    if (kbpf.magic != KBPF_MAGIC || (kbpf.version != 1 && kbpf.version != 2)) {
        kb_profiles_defaults();
        return;
    }
    for (int i = 0; i < 8; ++i) {
        kbpf.cpi[i]  = clamp_cpi(kbpf.cpi[i] ? kbpf.cpi[i] : KEYBALL_CPI_DEFAULT);
        kbpf.sdiv[i] = clamp_sdiv(kbpf.sdiv[i] ? kbpf.sdiv[i] : KEYBALL_SCROLL_DIV_DEFAULT);
        kbpf.inv[i]  = kbpf.inv[i] ? 1 : 0;
    }
    if (kbpf.version == 1) {
        // v1→v2 移行：追加分はデフォルト埋め
        for (int i = 0; i < 8; ++i) {
            kbpf.mv_gain_lo_fp[i] = (uint8_t)_CONSTRAIN(KEYBALL_MOVE_GAIN_LO_FP, 1, 255);
            kbpf.mv_th1[i]        = (uint8_t)_CONSTRAIN(KEYBALL_MOVE_TH1, 0, 63);
            kbpf.mv_th2[i]        = (uint8_t)_CONSTRAIN(KEYBALL_MOVE_TH2, 1, 63);
            if (kbpf.mv_th1[i] >= kbpf.mv_th2[i]) kbpf.mv_th1[i] = kbpf.mv_th2[i] - 1;
        }
        kbpf.version = 2; // 次回保存でv2化
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


static inline int16_t clamp_xy(int16_t v) {
    return (int16_t)_CONSTRAIN(v, MOUSE_REPORT_XY_MIN, MOUSE_REPORT_XY_MAX);
}

//////////////////////////////////////////////////////////////////////////////
// Pointing device driver
__attribute__((weak))
void keyball_on_apply_motion_to_mouse_move(report_mouse_t *report,
                                           report_mouse_t *output,
                                           bool is_left) {
#if KEYBALL_MOVE_SHAPING_ENABLE
    // 32bit蓄積（商/余り用）
    static int32_t acc_x = 0, acc_y = 0;
    static uint8_t last_sx = 0, last_sy = 0;
    static uint32_t last_ts = 0;

    int16_t sx = (int16_t)report->x;
    int16_t sy = (int16_t)report->y;

    // アイドル・方向反転で蓄積を捨てる（跳ね防止）
    uint32_t now = timer_read32();
    if (TIMER_DIFF_32(now, last_ts) > KEYBALL_MOVE_IDLE_RESET_MS) {
        acc_x = acc_y = 0;
    }
    if ((int8_t)sx && (int8_t)last_sx && ((sx ^ last_sx) < 0)) acc_x = 0;
    if ((int8_t)sy && (int8_t)last_sy && ((sy ^ last_sy) < 0)) acc_y = 0;
    last_sx = (uint8_t)sx; last_sy = (uint8_t)sy;
    last_ts = now;

    // 速度近似（高コストなsqrt回避）
    int16_t ax = (sx < 0 ? -sx : sx);
    int16_t ay = (sy < 0 ? -sy : sy);
    int16_t mag = (ax > ay) ? ax : ay;

    // ゲイン算出（固定小数点）
    // int32_t g_lo = KEYBALL_MOVE_GAIN_LO_FP;  // 例: 64
    int32_t g_lo = g_move_gain_lo_fp;  // 例: 64
    int32_t g_hi = KEYBALL_MOVE_GAIN_HI_FP;  // 例: 256
    int32_t gain_fp;

    if (mag <= g_move_th1) {
        gain_fp = g_lo;
    } else if (mag >= KEYBALL_MOVE_TH2) {
        gain_fp = g_hi;
    } else {
        // 線形補間
        int32_t num = (int32_t)(mag - g_move_th1);
        int32_t den = (int32_t)(KEYBALL_MOVE_TH2 - g_move_th1);
        if (den < 1) den = 1; // 保険
        gain_fp = g_lo + ( (g_hi - g_lo) * num ) / den;
    }

    // 固定小数点で適用（商/余り）
    acc_x += (int32_t)sx * gain_fp;
    acc_y += (int32_t)sy * gain_fp;

    int16_t out_x = (int16_t)(acc_x / KMF_DEN);
    int16_t out_y = (int16_t)(acc_y / KMF_DEN);

    acc_x -= (int32_t)out_x * KMF_DEN;
    acc_y -= (int32_t)out_y * KMF_DEN;

    // クランプして反映
    output->x = (int8_t)clamp_xy(out_x);
    output->y = (int8_t)clamp_xy(out_y);

    // 左右で「移動」は反転しない（従来のscrollとは別）
    (void)is_left;
#else
    // 旧仕様：そのまま
    output->x = report->x;
    output->y = report->y;
#endif
}

// __attribute__((weak)) void keyball_on_apply_motion_to_mouse_move(report_mouse_t *report, report_mouse_t *output, bool is_left) {
//     output->x = report->x;
//     output->y = report->y;

// }

__attribute__((weak))
void keyball_on_apply_motion_to_mouse_scroll(report_mouse_t *report,
                                             report_mouse_t *output,
                                             bool is_left) {
    int16_t out_x = 0;
    int16_t out_y = 0;

    // 32bitにして余裕を持たせる（高速回転や高CPIでのあふれ対策）
    static int32_t acc_x_mac = 0, acc_y_mac = 0;
    static int32_t acc_x_gen = 0, acc_y_gen = 0;
    static uint8_t last_sdiv = 0;
    static int8_t  last_sx = 0, last_sy = 0;
    static uint32_t last_ts = 0;

    uint32_t now = timer_read32();
    int16_t sx = (int16_t)report->x;
    int16_t sy = (int16_t)report->y;
    uint8_t sdiv = keyball_get_scroll_div();

    // 感度変更やアイドルで余りリセット
    if (sdiv != last_sdiv || TIMER_DIFF_32(now, last_ts) > KEYBALL_SCROLL_IDLE_RESET_MS) {
        acc_x_mac = acc_y_mac = 0;
        acc_x_gen = acc_y_gen = 0;
        last_sdiv = sdiv;
    }
#if KEYBALL_SCROLL_RESET_ON_DIRCHANGE
    if ((int8_t)sx && (int8_t)last_sx && ((sx ^ last_sx) < 0)) { acc_x_mac = acc_x_gen = 0; }
    if ((int8_t)sy && (int8_t)last_sy && ((sy ^ last_sy) < 0)) { acc_y_mac = acc_y_gen = 0; }
#endif
    last_sx = (int8_t)sx; last_sy = (int8_t)sy;
    last_ts = now;

    switch (detected_host_os()) {
      case OS_MACOS: {
        // WHEEL_DELTA=120 を「分母」に集約。分解能を上げるときはさらに掛け算。
        const int32_t DEN = 120 * (int32_t)KEYBALL_SCROLL_FINE_DEN;
        acc_x_mac += (int32_t)sx * (int32_t)sdiv;
        acc_y_mac += (int32_t)sy * (int32_t)sdiv;

        out_x = (int16_t)(acc_x_mac / DEN);
        out_y = (int16_t)(acc_y_mac / DEN);

        acc_x_mac -= (int32_t)out_x * DEN;
        acc_y_mac -= (int32_t)out_y * DEN;
        break;
      }
      default: {
        // Windows/Linux 等はそのまま（必要なら sdiv を掛ける）
        // uint8_t sdiv = keyball_get_scroll_div();
        out_x = (int16_t)report->x * sdiv;
        out_y = (int16_t)report->y * sdiv;
        break;
      }
    }

    // ---- モデル反映（従来ロジック）----
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
    if (output->h != 0 || output->v != 0) {
        keyball.scroll_snap_last = now;
    } else if (TIMER_DIFF_32(now, keyball.scroll_snap_last) >= KEYBALL_SCROLLSNAP_RESET_TIMER) {
        keyball.scroll_snap_tension_h = 0;
    }
    if (abs(keyball.scroll_snap_tension_h) < KEYBALL_SCROLLSNAP_TENSION_THRESHOLD) {
        keyball.scroll_snap_tension_h += out_y;
        output->h = 0;
    }
#elif KEYBALL_SCROLLSNAP_ENABLE == 2
    switch (keyball_get_scrollsnap_mode()) {
        case KEYBALL_SCROLLSNAP_MODE_VERTICAL:   output->h = 0; break;
        case KEYBALL_SCROLLSNAP_MODE_HORIZONTAL: output->v = 0; break;
        default: break;
    }
#endif

    // 反転（OS別）
    if (kbpf.inv[osi()]) {
        output->h = -output->h;
        output->v = -output->v;
    }
}

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
    uint8_t i = osi();
    kbpf.cpi[i] = cpi;
    dprintf("keyball: cpi set OS=%u -> %u\n", i, cpi);
    pointing_device_set_cpi_on_side(true,  cpi);
    pointing_device_set_cpi_on_side(false, cpi);
}

uint8_t keyball_get_scroll_div(void) {
    return clamp_sdiv(kbpf.sdiv[osi()]);
}

void keyball_set_scroll_div(uint8_t div) {
    div = clamp_sdiv(div);
    uint8_t i = osi();
    kbpf.sdiv[i] = div;
    dprintf("keyball: sdiv set OS=%u -> %u\n", i, div);
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
    kb_profiles_defaults(); // まず既定値
    kb_profiles_read();     // EEPROMから上書き

    keyball_set_cpi(keyball_get_cpi());
    keyball_set_scroll_div(keyball_get_scroll_div());
    g_move_gain_lo_fp = kbpf.mv_gain_lo_fp[osi()];
    g_move_th1        = kbpf.mv_th1[osi()];
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

        case SCRL_MO:
            keyball_set_scroll_mode(record->event.pressed);
            // process_auto_mouse may use this in future, if changed order of
            // processes.
            return true;
    }

    // process events which works on pressed only.
    if (record->event.pressed) {
        switch (keycode) {
            case KBC_RST:
                kb_profiles_defaults();
                keyball_set_cpi(kbpf.cpi[osi()]);
                keyball_set_scroll_div(kbpf.sdiv[osi()]);
                g_move_gain_lo_fp = kbpf.mv_gain_lo_fp[osi()];
                g_move_th1        = kbpf.mv_th1[osi()];
                kb_profiles_write();
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
                set_auto_mouse_enable(false);
                set_auto_mouse_timeout(AUTO_MOUSE_TIME);
#endif
                break;

            case KBC_SAVE:
                kbpf.mv_gain_lo_fp[osi()] = (uint8_t)_CONSTRAIN(g_move_gain_lo_fp, 1, 255);
                kbpf.mv_th1[osi()]        = (uint8_t)_CONSTRAIN(g_move_th1, 0, kbpf.mv_th2[osi()] - 1);
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

            case SCRL_TO:
                keyball_set_scroll_mode(!keyball.scroll_mode);
                break;

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

            case MVGL_UP:
                 g_move_gain_lo_fp = _CONSTRAIN(g_move_gain_lo_fp + 8, 16, 255); // 255に上限
                 kbpf.mv_gain_lo_fp[osi()] = (uint8_t)_CONSTRAIN(g_move_gain_lo_fp, 1, 255);
                 dprintf("move: gain_lo=%ld/256\n", (long)g_move_gain_lo_fp);
                 break;
            case MVGL_DN:
                 g_move_gain_lo_fp = _CONSTRAIN(g_move_gain_lo_fp - 8, 16, 255);
                 kbpf.mv_gain_lo_fp[osi()] = (uint8_t)_CONSTRAIN(g_move_gain_lo_fp, 1, 255);
                 dprintf("move: gain_lo=%ld/256\n", (long)g_move_gain_lo_fp);
                 break;
            case MVTH1_UP:
                 g_move_th1 = _CONSTRAIN(g_move_th1 + 1, 0, kbpf.mv_th2[osi()] - 1);
                 kbpf.mv_th1[osi()] = (uint8_t)_CONSTRAIN(g_move_th1, 0, kbpf.mv_th2[osi()] - 1);
                 dprintf("move: th1=%d\n", g_move_th1);
                 break;
            case MVTH1_DN:
                 g_move_th1 = _CONSTRAIN(g_move_th1 - 1, 0, kbpf.mv_th2[osi()] - 1);
                 kbpf.mv_th1[osi()] = (uint8_t)_CONSTRAIN(g_move_th1, 0, kbpf.mv_th2[osi()] - 1);
                 dprintf("move: th1=%d\n", g_move_th1);
                 break;

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

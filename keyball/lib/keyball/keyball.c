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
#include "timer.h"
#include "oled_driver.h"

// ==== OLED UI state ====
static kb_oled_mode_t g_oled_mode = KB_OLED_MODE_NORMAL;
static uint8_t        g_sw_dbg_page = 0;      // 既存
static bool           g_sw_dbg_en   = true;   // 既存（デバッグ描画ON/OFFフラグ）

#define KB_SW_DBG_PAGE_COUNT   3
#define KB_OLED_UI_DEBOUNCE_MS 100

static uint32_t g_oled_ui_ts = 0;  // 操作デバウンス

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

// マウス移動量調整用
static int32_t g_move_gain_lo_fp = KEYBALL_MOVE_GAIN_LO_FP;
static int16_t g_move_th1 = KEYBALL_MOVE_TH1;  // th2 は固定でもOK。要れば同様に可変化。

// OS 検出用
static uint8_t g_os_idx = 0;      // 決定した OS スロット
static bool    g_os_idx_init = false;

////////////////////////////////////////////////////////////////////////////////////////////////////
// スワイプ用
// // ---- Swipe thresholds (compile-time for now) ----
// #ifndef KB_SW_DEADZONE
// #  define KB_SW_DEADZONE 1
// #endif
// #ifndef KB_SW_STEP
// #  define KB_SW_STEP     200   // しきい値：超えるたびに発火、余剰は持ち越し
// #endif
// #ifndef KB_SWIPE_FREEZE_POINTER
// #  define KB_SWIPE_FREEZE_POINTER 1   // 1: スワイプ中 出力を0に
// #endif

static inline int16_t kb_abs16(int16_t v) { return (v < 0) ? -v : v; }
static inline void kb_sat_add_pos32(int32_t *acc, int32_t delta) {
  // acc は正値のみ運用。オーバーフロー防止。
  int64_t t = (int64_t)(*acc) + (int64_t)delta;
  if (t > (1L<<27)) t = (1L<<27);
  if (t < 0) t = 0;
  *acc = (int32_t)t;
}

typedef struct {
  bool            active;
  kb_swipe_tag_t  tag;
  bool            fired_any;
  kb_swipe_dir_t  last_dir;   // 直近の発火方向（検出未実装の間は NONE のまま）
                              // 方向別累積（正距離）。RIGHT/LEFT/DOWN/UP
  int32_t acc_r, acc_l, acc_d, acc_u;
} kb_swipe_session_t;

static kb_swipe_session_t g_sw = {0};

// ==== API実装（検出はまだ入れない）====
void keyball_swipe_begin(kb_swipe_tag_t mode_tag) {
  g_sw.active   = true;
  g_sw.tag      = mode_tag;
  g_sw.fired_any= false;
  g_sw.last_dir = KB_SWIPE_NONE;
  g_sw.acc_r = g_sw.acc_l = g_sw.acc_d = g_sw.acc_u = 0;
}
void keyball_swipe_end(void) {
  g_sw.active   = false;
  g_sw.tag      = 0;
  g_sw.fired_any= false;
  g_sw.last_dir = KB_SWIPE_NONE;
  g_sw.acc_r = g_sw.acc_l = g_sw.acc_d = g_sw.acc_u = 0;
}
bool           keyball_swipe_is_active(void)         { return g_sw.active; }
kb_swipe_tag_t keyball_swipe_mode_tag(void)          { return g_sw.tag; }
kb_swipe_dir_t keyball_swipe_direction(void)         { return g_sw.last_dir; }
bool           keyball_swipe_fired_since_begin(void) { return g_sw.fired_any; }
bool           keyball_swipe_consume_fired(void)     { bool f = g_sw.fired_any; g_sw.fired_any = false; return f; }

kb_swipe_params_t keyball_swipe_get_params(void){
    kb_swipe_params_t p = {
        .step     = kbpf.step,
        .deadzone = kbpf.deadzone,
        .freeze   = (kbpf.freeze & 1) != 0
    };
    return p;
}

void keyball_swipe_set_step(uint16_t v){
  if (v < 10) v = 10;
  if (v > 2000) v = 2000;
  kbpf.step = v;
  uprintf("SW step=%u\r\n", kbpf.step);
}

void keyball_swipe_set_deadzone(uint8_t v){
  if (v > 32) v = 32;
  kbpf.deadzone = v;
  uprintf("SW deadzone=%u\r\n", kbpf.deadzone);
}

void keyball_swipe_set_freeze(bool on){
  kbpf.freeze = on ? 1 : 0;
  uprintf("SW freeze=%u\r\n", kbpf.freeze ? 1 : 0);
}

void keyball_swipe_toggle_freeze(void){
  kbpf.freeze ^= 1;  // ← g_sw_params ではなく kbpf をトグル
  uprintf("SW freeze=%u\r\n", kbpf.freeze ? 1 : 0);
}

static inline bool ui_op_ready(void){
  if (TIMER_DIFF_32(timer_read32(), g_oled_ui_ts) < KB_OLED_UI_DEBOUNCE_MS) return false;
  g_oled_ui_ts = timer_read32();
  return true;
}

void keyball_oled_set_mode(kb_oled_mode_t m){
  g_oled_mode = m;
  g_sw_dbg_en = (m == KB_OLED_MODE_DEBUG);  // 通常=非表示, デバッグ=表示
  oled_clear();
}

void keyball_oled_mode_toggle(void){
  if (!ui_op_ready()) return;
  keyball_oled_set_mode((g_oled_mode == KB_OLED_MODE_DEBUG) ? KB_OLED_MODE_NORMAL : KB_OLED_MODE_DEBUG);
}

kb_oled_mode_t keyball_oled_get_mode(void){ return g_oled_mode; }

void keyball_swipe_dbg_next_page(void){
  if (!ui_op_ready()) return;
  g_sw_dbg_page = (g_sw_dbg_page + 1) % KB_SW_DBG_PAGE_COUNT;
  oled_clear();
}

void keyball_swipe_dbg_prev_page(void){
  if (!ui_op_ready()) return;
  g_sw_dbg_page = (g_sw_dbg_page + KB_SW_DBG_PAGE_COUNT - 1) % KB_SW_DBG_PAGE_COUNT;
  oled_clear();
}

uint8_t keyball_swipe_dbg_get_page(void){ return g_sw_dbg_page; }

// プロトタイプ宣言
static void kb_apply_swipe(report_mouse_t *report, report_mouse_t *output, bool is_left);

static void kb_sw_try_fire(kb_swipe_dir_t dir,
    int32_t *acc_target,
    int32_t *a1, int32_t *a2, int32_t *a3) {

  while (*acc_target >= kbpf.step) {
    if (keyball_on_swipe_fire) {
      keyball_on_swipe_fire(g_sw.tag, dir);
    }
    g_sw.fired_any = true;
    g_sw.last_dir  = dir;

    *acc_target -= kbpf.step;      // ← ここを修正！
    if (*acc_target < 0) *acc_target = 0;
    *a1 = *a2 = *a3 = 0;           // 他3方向はクリア
  }
}

void keyball_swipe_dbg_toggle(void)         { g_sw_dbg_en = !g_sw_dbg_en; }

//////////////////////////////////////////////////////////////////////////////////////////
// new keyball profiles
// ---- Keyball専用 EEPROM ブロック（VIA不使用前提）----
// typedef struct __attribute__((packed)) {
//   uint32_t magic;     // 'KBP1'
//   uint16_t version;   // 1
//   uint16_t reserved;  // 0
//   uint16_t cpi[8];    // 100..CPI_MAX
//   uint8_t  sdiv[8];   // 1..SCROLL_DIV_MAX
//   uint8_t  inv[8];    // 0/1
//   uint8_t  mv_gain_lo_fp[8]; // 固定小数点(1/256)。16..255 推奨
//   uint8_t  mv_th1[8];        // 0..(mv_th2-1)
//   uint8_t  mv_th2[8];        // 1..63 など適当な上限（今回は固定でも可）
// } keyball_profiles_t;

// #define KBPF_MAGIC   0x4B425031u /* 'KBP1' */
// #define KBPF_VERSION 2
// _Static_assert(sizeof(keyball_profiles_t) == 40 + 24, "update size expectation if you change fields");

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

keyball_profiles_t kbpf;

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

static void add_cpi(int16_t delta) {
  int16_t v = keyball_get_cpi() + delta;
  keyball_set_cpi(v < 1 ? 1 : v);
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

static inline int16_t clamp_xy(int16_t v) {
  return (int16_t)_CONSTRAIN(v, MOUSE_REPORT_XY_MIN, MOUSE_REPORT_XY_MAX);
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
  // kbpf.version = KBPF_VERSION;
  kbpf.version = KBPF_VER_CUR;
  kbpf.reserved= 0;
}

static inline void kbpf_set_swipe_defaults(keyball_profiles_t *p){
  p->step     = KB_SW_STEP;
  p->deadzone = KB_SW_DEADZONE;
  p->freeze    = (KB_SWIPE_FREEZE_POINTER ? 0x01 : 0x00);
}

static void kb_profiles_validate(void) {
  if (kbpf.magic != KBPF_MAGIC || (kbpf.version != 1 && kbpf.version != KBPF_VER_CUR)) {
    kb_profiles_defaults();
    return;
  }
  for (int i = 0; i < 8; ++i) {
    kbpf.cpi[i]  = clamp_cpi(kbpf.cpi[i] ? kbpf.cpi[i] : KEYBALL_CPI_DEFAULT);
    kbpf.sdiv[i] = clamp_sdiv(kbpf.sdiv[i] ? kbpf.sdiv[i] : KEYBALL_SCROLL_DIV_DEFAULT);
    kbpf.inv[i]  = kbpf.inv[i] ? 1 : 0;
  }
  if (kbpf.version == 1) {
    // v1 -> v2 への移行（新フィールドに既定値を充填）
    kbpf.step     = KB_SW_STEP;
    kbpf.deadzone = KB_SW_DEADZONE;
    kbpf.freeze   = (KB_SWIPE_FREEZE_POINTER ? 1 : 0);
    kbpf.version  = KBPF_VER_CUR; // SAVE 時に確定
  }
  // v2 の範囲ガード
  if (kbpf.step < 1 || kbpf.step > 2000) kbpf.step = KB_SW_STEP;
  if (kbpf.deadzone > 32)                kbpf.deadzone = KB_SW_DEADZONE;
  kbpf.freeze &= 0x01;
}


void kbpf_after_load_fixup(void){  // 例名。あなたの初期化ハンドラに入れてOK
  if (kbpf.version < KBPF_VER_CUR) {
    // 旧版 → 新フィールドに既定値を充填
    kbpf_set_swipe_defaults(&kbpf);
    kbpf.version = KBPF_VER_CUR;
    // ★ ここで即セーブはしない（ユーザーが SAVE した時に上書きでOK）
  } else {
    // 現行/将来版 → 範囲ガード（壊れ値対策）
    if (kbpf.step == 0 || kbpf.step > 2000) kbpf.step = KB_SW_STEP;
    if (kbpf.deadzone > 32) kbpf.deadzone = KB_SW_DEADZONE;
    kbpf.freeze &= 0x01; // 予約ビットを落とす
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
// スワイプ処理
static void kb_apply_swipe(report_mouse_t *report, report_mouse_t *output, bool is_left) {
  // ---- RAW motion（整形前）----
  int16_t sx = (int16_t)report->x;
  int16_t sy = (int16_t)report->y;

  // デッドゾーン
  if (kb_abs16(sx) < kbpf.deadzone) sx = 0;
  if (kb_abs16(sy) < kbpf.deadzone) sy = 0;

  // 符号別に累積、反対側は 0（距離は正で加算）
  if (sx > 0)      { kb_sat_add_pos32(&g_sw.acc_r, sx);  g_sw.acc_l = 0; }
  else if (sx < 0) { kb_sat_add_pos32(&g_sw.acc_l, -sx); g_sw.acc_r = 0; }

  if (sy > 0)      { kb_sat_add_pos32(&g_sw.acc_d, sy);  g_sw.acc_u = 0; }
  else if (sy < 0) { kb_sat_add_pos32(&g_sw.acc_u, -sy); g_sw.acc_d = 0; }

  // 主成分優先の評価順（斜め同時越え時の順序安定化）
  bool prefer_x = (kb_abs16((int16_t)report->x) >= kb_abs16((int16_t)report->y));
  if (prefer_x) {
    kb_sw_try_fire(KB_SWIPE_RIGHT, &g_sw.acc_r, &g_sw.acc_l, &g_sw.acc_u, &g_sw.acc_d);
    kb_sw_try_fire(KB_SWIPE_LEFT,  &g_sw.acc_l, &g_sw.acc_r, &g_sw.acc_u, &g_sw.acc_d);
    kb_sw_try_fire(KB_SWIPE_DOWN,  &g_sw.acc_d, &g_sw.acc_u, &g_sw.acc_r, &g_sw.acc_l);
    kb_sw_try_fire(KB_SWIPE_UP,    &g_sw.acc_u, &g_sw.acc_d, &g_sw.acc_r, &g_sw.acc_l);
  } else {
    kb_sw_try_fire(KB_SWIPE_DOWN,  &g_sw.acc_d, &g_sw.acc_u, &g_sw.acc_r, &g_sw.acc_l);
    kb_sw_try_fire(KB_SWIPE_UP,    &g_sw.acc_u, &g_sw.acc_d, &g_sw.acc_r, &g_sw.acc_l);
    kb_sw_try_fire(KB_SWIPE_RIGHT, &g_sw.acc_r, &g_sw.acc_l, &g_sw.acc_u, &g_sw.acc_d);
    kb_sw_try_fire(KB_SWIPE_LEFT,  &g_sw.acc_l, &g_sw.acc_r, &g_sw.acc_u, &g_sw.acc_d);
  }
}

// ポインターの動き変換フック
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

// スクロール変換フック
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

// report の motion を output に変換して加算し、report の motion はクリアする。
static void motion_to_mouse(report_mouse_t *report, report_mouse_t *output, bool is_left, bool as_scroll) {
  if (keyball_swipe_is_active()) {
    kb_apply_swipe(report, output, is_left);
    // ★ freezeがOFFなら、通常のMove経路も通してポインタを動かす
    if (!kbpf.freeze) {
      keyball_on_apply_motion_to_mouse_move(report, output, is_left);
    }
  } else if (as_scroll) {
    keyball_on_apply_motion_to_mouse_scroll(report, output, is_left);
  } else {
    keyball_on_apply_motion_to_mouse_move(report, output, is_left);
  }
  // clear motion
  report->x = 0;
  report->y = 0;
}

// 両手キーボードでのポインティングデバイス処理
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
                          // kbpf_after_load_fixup(); // 旧版からの移行処理

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

#if defined(OLED_ENABLE) || defined(OLED_DRIVER_ENABLE)
// 再入防止（レンダ中に再呼び出されてもスキップ）
static bool g_sw_dbg_in_render = false;

// 0..9999 に丸めた無符号
static inline unsigned clip0_9999(int32_t v){
  if (v <= 0) return 0u;
  if (v >= 9999) return 9999u;
  return (unsigned)v;
}
static inline const char* kb_dir_str(kb_swipe_dir_t d){
  return (d==KB_SWIPE_UP)?"UP ":(d==KB_SWIPE_DOWN)?"DN ":
    (d==KB_SWIPE_LEFT)?"LT ":(d==KB_SWIPE_RIGHT)?"RT ":"NON";
}

void keyball_oled_render_swipe_debug(void){
  if (g_oled_mode != KB_OLED_MODE_DEBUG) return; // 通常モードなら描かない
  if (!g_sw_dbg_en) return;
  if (g_sw_dbg_in_render) return;
  g_sw_dbg_in_render = true;

  char line[32];
  oled_set_cursor(0, 0); // 毎回先頭から

  switch (g_sw_dbg_page) {
    case 0: { // 環境（Move shaping 等）
              uint16_t cpi = keyball_get_cpi();
              snprintf(line, sizeof(line), "CPI:%u", (unsigned)cpi);
              oled_write_ln(line, false);

              // Move shaping が無効なら別表示
#ifdef KEYBALL_MOVE_SHAPING_ENABLE
              extern int32_t g_move_gain_lo_fp;
              extern int16_t g_move_th1;
              snprintf(line, sizeof(line), "Glo:%ld Th1:%d", (long)g_move_gain_lo_fp, (int)g_move_th1);
#else
              snprintf(line, sizeof(line), "MoveShape:OFF");
#endif
              oled_write_ln(line, false);

              snprintf(line, sizeof(line), "Div:%u Inv:%u",
                       (unsigned)keyball_get_scroll_div(),
                       (unsigned)(kbpf.inv[osi()] ? 1 : 0));
              oled_write_ln(line, false);

              snprintf(line, sizeof(line), "Pg:%u/%u", (unsigned)(g_sw_dbg_page+1), (unsigned)KB_SW_DBG_PAGE_COUNT);
              oled_write_ln(line, false);
            } break;

    case 1: { // 状態・基本パラメータ
              kb_swipe_params_t p = keyball_swipe_get_params();
              snprintf(line, sizeof(line), "A:%u Tg:%u Fz:%u",
                  keyball_swipe_is_active()?1u:0u,
                  (unsigned)keyball_swipe_mode_tag(),
                  p.freeze?1u:0u);
              oled_write_ln(line, false);

              snprintf(line, sizeof(line), "St:%u Dz:%u",
                  (unsigned)p.step, (unsigned)p.deadzone);
              oled_write_ln(line, false);

              snprintf(line, sizeof(line), "Dir:%s Fd:%u",
                  kb_dir_str(keyball_swipe_direction()),
                  keyball_swipe_fired_since_begin()?1u:0u);
              oled_write_ln(line, false);

              snprintf(line, sizeof(line), "Pg:%u/%u", (unsigned)(g_sw_dbg_page+1), (unsigned)KB_SW_DBG_PAGE_COUNT);
              oled_write_ln(line, false);
            } break;

    case 2: { // Accumulators
              unsigned ar = clip0_9999(g_sw.acc_r);
              unsigned al = clip0_9999(g_sw.acc_l);
              unsigned ad = clip0_9999(g_sw.acc_d);
              unsigned au = clip0_9999(g_sw.acc_u);

              snprintf(line, sizeof(line), "R%4u L%4u", ar, al);
              oled_write_ln(line, false);
              snprintf(line, sizeof(line), "D%4u U%4u", ad, au);
              oled_write_ln(line, false);

              oled_write_ln("Acc", false);
              snprintf(line, sizeof(line), "Pg:%u/%u", (unsigned)(g_sw_dbg_page+1), (unsigned)KB_SW_DBG_PAGE_COUNT);
              oled_write_ln(line, false);
            } break;
  }

  g_sw_dbg_in_render = false;
}
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

      case SW_ST_U:
                     if (record->event.pressed) {
                       kb_swipe_params_t p = keyball_swipe_get_params();
                       keyball_swipe_set_step(p.step + 10);
                     }
                     return false;

      case SW_ST_D:
                     if (record->event.pressed) {
                       kb_swipe_params_t p = keyball_swipe_get_params();
                       keyball_swipe_set_step((p.step > 10) ? p.step - 10 : 10);
                     }
                     return false;

      case SW_DZ_U:
                     if (record->event.pressed) {
                       kb_swipe_params_t p = keyball_swipe_get_params();
                       keyball_swipe_set_deadzone(p.deadzone + 1);
                     }
                     return false;

      case SW_DZ_D:
                     if (record->event.pressed) {
                       kb_swipe_params_t p = keyball_swipe_get_params();
                       keyball_swipe_set_deadzone((p.deadzone > 0) ? p.deadzone - 1 : 0);
                     }
                     return false;

      case SW_FRZ:
                     if (record->event.pressed) {
                       keyball_swipe_toggle_freeze();
                     }
                     return false;

      case DBG_TOG:
                     keyball_oled_mode_toggle();
                     return false;

      case DBG_NP:
                     keyball_swipe_dbg_next_page();
                     return false;

      case DBG_PP:
                     keyball_swipe_dbg_prev_page();
                     return false;

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

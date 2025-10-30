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

#pragma once

#include "keyball_kbpf.h"
#include "keyball_keycodes.h"
#include "keyball_move.h"
#include "keyball_oled.h"
#include "keyball_scroll.h"
#include "keyball_swipe.h"
#ifdef HAPTIC_ENABLE
#    include "haptic.h"
#endif

// カスタムキーコードの運用（KBレベル/ユーザーレベルと SAFE_RANGE）
// ----------------------------------------------------------------------
// 1) KBレベル（QK_KB_*）
//    - Vial の customKeycodes は QK_KB_0, QK_KB_1, ... の順に並ぶ前提。
//    - 列挙順は QK_KB_* の連番順になるように並べる（途中で前後させない）。
//
// 2) SAFE_RANGE（動的決定）
//    - KBレベルの列挙の末尾に KEYBALL_SAFE_RANGE を置くことで、
//      「最後に割り当てられた QK_KB_* の次の値」を SAFE_RANGE として動的に決定します。
//    - ユーザーレベルのキー（配布でVial表示したいキー）は、この KEYBALL_SAFE_RANGE から
//      連番で定義します（ユーザ側ヘッダで `= KEYBALL_SAFE_RANGE` を用いる）。
//
// 3) Vial 側の扱い
//    - `keymaps/<name>/vial.json` の customKeycodes 配列は、上記の列挙順と一致させます。
//    - KBレベルの追加で SAFE_RANGE が増減する場合は、vial.json の配列順も追随してください。
//     （必要があればダミー項目で穴埋めしてインデックスを合わせても構いません。）

//////////////////////////////////////////////////////////////////////////////
// Configurations

// naming update: CPI -> Mouse Speed (MoSp), Scroll Step -> Scroll Speed (ScSp)
#ifndef KEYBALL_MOUSE_SPEED_DEFAULT
#define KEYBALL_MOUSE_SPEED_DEFAULT 2800
#endif

#ifndef KEYBALL_SCROLL_SPEED_DEFAULT
#define KEYBALL_SCROLL_SPEED_DEFAULT 4
#endif

// 入力がしばらく無い時に余りを捨てる（ms）
#ifndef KEYBALL_SCROLL_IDLE_RESET_MS
#define KEYBALL_SCROLL_IDLE_RESET_MS 80
#endif

/// To disable scroll snap feature, define 0 in your config.h
#ifndef KEYBALL_SCROLLSNAP_ENABLE
#define KEYBALL_SCROLLSNAP_ENABLE 2
#endif

#ifndef KEYBALL_SCROLLSNAP_RESET_TIMER
#define KEYBALL_SCROLLSNAP_RESET_TIMER 500
#endif

#ifndef KEYBALL_SCROLLSNAP_TENSION_THRESHOLD
#define KEYBALL_SCROLLSNAP_TENSION_THRESHOLD 200
#endif

// スクロール反転の既定（未定義の場合は0=off）
#ifndef KEYBALL_SCROLL_INVERT
#define KEYBALL_SCROLL_INVERT 0
#endif

// 最終出力に適用する水平/垂直の個別ゲイン（固定小数点: 256=1.00）
#ifndef KEYBALL_SCROLL_HOR_GAIN_FP
#define KEYBALL_SCROLL_HOR_GAIN_FP 256
#endif
#ifndef KEYBALL_SCROLL_VER_GAIN_FP
#define KEYBALL_SCROLL_VER_GAIN_FP 256
#endif

// Auto mouse layer: accumulation reset window (ms)
#ifndef KEYBALL_AML_ACC_RESET_MS
#define KEYBALL_AML_ACC_RESET_MS 200
#endif
// AML 積算スケーリング（大きいほど積算が遅くなる）
#ifndef KEYBALL_AML_ACC_DIV
#define KEYBALL_AML_ACC_DIV 8
#endif
// AML 積算に加える最小単位（ノイズ抑制）
#ifndef KEYBALL_AML_ACC_MIN_UNIT
#define KEYBALL_AML_ACC_MIN_UNIT 2
#endif

//////////////////////////////////////////////////////////////////////////////
// kbpf persistent defaults (edit here to tweak build-time behaviour)
//////////////////////////////////////////////////////////////////////////////

// --- Per-OS slot defaults --------------------------------------------------
// Applied to every kbpf OS profile (0..7) when kbpf_defaults() runs.
#ifndef KBPF_DEFAULT_CPI
#    define KBPF_DEFAULT_CPI KEYBALL_MOUSE_SPEED_DEFAULT
#endif
#ifndef KBPF_DEFAULT_SCROLL_STEP
#    define KBPF_DEFAULT_SCROLL_STEP KEYBALL_SCROLL_SPEED_DEFAULT
#endif
#ifndef KBPF_DEFAULT_SCROLL_INVERT
#    define KBPF_DEFAULT_SCROLL_INVERT ((KEYBALL_SCROLL_INVERT) ? 1 : 0)
#endif
#ifndef KBPF_DEFAULT_MOVE_GAIN_LO_FP
#    define KBPF_DEFAULT_MOVE_GAIN_LO_FP KEYBALL_MOVE_GAIN_LO_FP
#endif
#ifndef KBPF_DEFAULT_MOVE_TH1
#    define KBPF_DEFAULT_MOVE_TH1 KEYBALL_MOVE_TH1
#endif
#ifndef KBPF_DEFAULT_MOVE_TH2
#    define KBPF_DEFAULT_MOVE_TH2 KEYBALL_MOVE_TH2
#endif
#ifndef KBPF_DEFAULT_SCROLL_INTERVAL
#    define KBPF_DEFAULT_SCROLL_INTERVAL 1
#endif
#ifndef KBPF_DEFAULT_SCROLL_VALUE
#    define KBPF_DEFAULT_SCROLL_VALUE 1
#endif
#ifndef KBPF_DEFAULT_SCROLL_PRESET
#    define KBPF_DEFAULT_SCROLL_PRESET 1
#endif
// macOS specific overrides (slot = OS_MACOS)
#ifndef KBPF_DEFAULT_SCROLL_INTERVAL_MAC
#    define KBPF_DEFAULT_SCROLL_INTERVAL_MAC 120
#endif
#ifndef KBPF_DEFAULT_SCROLL_VALUE_MAC
#    define KBPF_DEFAULT_SCROLL_VALUE_MAC 120
#endif
#ifndef KBPF_DEFAULT_SCROLL_PRESET_MAC
#    define KBPF_DEFAULT_SCROLL_PRESET_MAC 2
#endif

// --- Global defaults -------------------------------------------------------
// Single shared settings persisted once per keyboard.
// Swipe behaviour -----------------------------------------------------------
#ifndef KBPF_DEFAULT_SWIPE_STEP
#    define KBPF_DEFAULT_SWIPE_STEP 180
#endif
#ifndef KBPF_DEFAULT_SWIPE_DEADZONE
#    define KBPF_DEFAULT_SWIPE_DEADZONE 1
#endif
#ifndef KBPF_DEFAULT_SWIPE_FREEZE
#    define KBPF_DEFAULT_SWIPE_FREEZE 1
#endif
#ifndef KBPF_DEFAULT_SWIPE_RESET_MS
#    define KBPF_DEFAULT_SWIPE_RESET_MS 30
#endif
#ifndef KBPF_DEFAULT_SWIPE_HAPTIC_MODE
#    ifdef HAPTIC_ENABLE
#        define KBPF_DEFAULT_SWIPE_HAPTIC_MODE HAPTIC_DEFAULT_MODE
#    else
#        define KBPF_DEFAULT_SWIPE_HAPTIC_MODE 0
#    endif
#endif
#ifndef KBPF_DEFAULT_SWIPE_HAPTIC_MODE_REPEAT
#    define KBPF_DEFAULT_SWIPE_HAPTIC_MODE_REPEAT KBPF_DEFAULT_SWIPE_HAPTIC_MODE
#endif
#ifndef KBPF_DEFAULT_SWIPE_HAPTIC_IDLE_MS
#    define KBPF_DEFAULT_SWIPE_HAPTIC_IDLE_MS 1000
#endif
#ifndef KBPF_DEFAULT_SCROLL_DEADZONE
#    define KBPF_DEFAULT_SCROLL_DEADZONE 0
#endif
// hysteresisは未実装
#ifndef KBPF_DEFAULT_SCROLL_HYSTERESIS
#    define KBPF_DEFAULT_SCROLL_HYSTERESIS 0
#endif
#ifndef KBPF_DEFAULT_MOVE_DEADZONE
#    define KBPF_DEFAULT_MOVE_DEADZONE 0
#endif
#ifndef KBPF_DEFAULT_AML_ENABLE
#    define KBPF_DEFAULT_AML_ENABLE 0
#endif
#ifndef KBPF_DEFAULT_AML_LAYER
#    define KBPF_DEFAULT_AML_LAYER 0xFFu
#endif
#ifndef KBPF_DEFAULT_AML_TIMEOUT
#    define KBPF_DEFAULT_AML_TIMEOUT 3000
#endif
#ifndef KBPF_DEFAULT_AML_THRESHOLD
#    define KBPF_DEFAULT_AML_THRESHOLD 100
#endif
#ifndef KBPF_DEFAULT_SCROLLSNAP_MODE
#    define KBPF_DEFAULT_SCROLLSNAP_MODE KEYBALL_SCROLLSNAP_MODE_VERTICAL
#endif
#ifndef KBPF_DEFAULT_SCROLLSNAP_THR
#    define KBPF_DEFAULT_SCROLLSNAP_THR KEYBALL_SCROLLSNAP_TENSION_THRESHOLD
#endif
#ifndef KBPF_DEFAULT_SCROLLSNAP_RESET_MS
#    define KBPF_DEFAULT_SCROLLSNAP_RESET_MS KEYBALL_SCROLLSNAP_RESET_TIMER
#endif
#ifndef KBPF_DEFAULT_SCROLL_HOR_GAIN_PCT
#    define KBPF_DEFAULT_SCROLL_HOR_GAIN_PCT 100
#endif
#ifndef KBPF_DEFAULT_SCROLL_LAYER_ENABLE
#    define KBPF_DEFAULT_SCROLL_LAYER_ENABLE 1
#endif
#ifndef KBPF_DEFAULT_SCROLL_LAYER_INDEX
#    define KBPF_DEFAULT_SCROLL_LAYER_INDEX 6
#endif
#ifndef KBPF_DEFAULT_LAYER
#    define KBPF_DEFAULT_LAYER 0
#endif

// --- Compatibility aliases -------------------------------------------------
// Preserve historical macro names so older user configuration overrides keep working.
#ifndef KB_SW_STEP
#    define KB_SW_STEP KBPF_DEFAULT_SWIPE_STEP
#endif
#ifndef KB_SW_DEADZONE
#    define KB_SW_DEADZONE KBPF_DEFAULT_SWIPE_DEADZONE
#endif
#ifndef KB_SWIPE_FREEZE_POINTER
#    define KB_SWIPE_FREEZE_POINTER KBPF_DEFAULT_SWIPE_FREEZE
#endif
#ifndef KB_SW_RST_MS
#    define KB_SW_RST_MS KBPF_DEFAULT_SWIPE_RESET_MS
#endif
#ifndef KB_SCROLL_DEADZONE
#    define KB_SCROLL_DEADZONE KBPF_DEFAULT_SCROLL_DEADZONE
#endif
#ifndef KB_SCROLL_HYST
#    define KB_SCROLL_HYST KBPF_DEFAULT_SCROLL_HYSTERESIS
#endif
#ifndef KB_MOVE_DEADZONE
#    define KB_MOVE_DEADZONE KBPF_DEFAULT_MOVE_DEADZONE
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// pointer motion configuration
// ==== Move shaping (pointer) ====
#ifndef KEYBALL_MOVE_SHAPING_ENABLE
#define KEYBALL_MOVE_SHAPING_ENABLE 1
#endif

// 固定小数点の分母（256が安全）
#define KMF_DEN 256

// 低速域ゲイン（0.25 ≒ 64/256）、高速域ゲイン（1.00 ≒ 256/256）
#ifndef KEYBALL_MOVE_GAIN_LO_FP
// 低速域ゲイン（固定小数点1/256）。少し抑えて蓄積寄りに。
#define KEYBALL_MOVE_GAIN_LO_FP 154 // 154/256 ≒ 0.60（少し抑える）
#endif
#ifndef KEYBALL_MOVE_GAIN_HI_FP
#define KEYBALL_MOVE_GAIN_HI_FP 256 // 1.00（変更なし）
#endif

// 速度の近似は max(|x|,|y|) + min(|x|,|y|)/2 を使用（軽い・十分）
#ifndef KEYBALL_MOVE_TH1
// 速度しきい値（mag = max(|x|,|y|) + min(|x|,|y|)/2）
#define KEYBALL_MOVE_TH1 2 // ここまでは低速域ゲイン
#endif
#ifndef KEYBALL_MOVE_TH2
#define KEYBALL_MOVE_TH2 8 // 緩やかにGAIN_LO→GAIN_HIへ線形補間
#endif

// 軸ごとのゲイン補正（固定小数点1/256）
#ifndef KEYBALL_MOVE_AXIS_GAIN_X_FP
#define KEYBALL_MOVE_AXIS_GAIN_X_FP KMF_DEN
#endif
#ifndef KEYBALL_MOVE_AXIS_GAIN_Y_FP
#define KEYBALL_MOVE_AXIS_GAIN_Y_FP ((KMF_DEN * 5) / 4) // ≒1.25倍で縦方向を底上げ
#endif

#ifndef KEYBALL_MOVE_IDLE_RESET_MS
#define KEYBALL_MOVE_IDLE_RESET_MS 80 // アイドルでaccを捨てて跳ね防止
#endif

// XYのクランプ（QMK標準は±127）
#ifndef MOUSE_REPORT_XY_MIN
#define MOUSE_REPORT_XY_MIN (-127)
#endif
#ifndef MOUSE_REPORT_XY_MAX
#define MOUSE_REPORT_XY_MAX (127)
#endif

// 基本はOLED有効
#ifndef OLED_DRIVER_ENABLE
#define KEYBALL_OLED_ENABLE 1
#endif

/// Specify SROM ID to be uploaded PMW3360DW (optical sensor).  It will be
/// enabled high CPI setting or so.  Valid valus are 0x04 or 0x81.  Define this
/// in your config.h to be enable.  Please note that using this option will
/// increase the firmware size by more than 4KB.
// #define KEYBALL_PMW3360_UPLOAD_SROM_ID 0x04
// #define KEYBALL_PMW3360_UPLOAD_SROM_ID 0x81

/// Defining this macro keeps two functions intact: keycode_config() and
/// mod_config() in keycode_config.c.
///
/// These functions customize the magic key code and are useless if the magic
/// key code is disabled.  Therefore, Keyball automatically disables it.
/// However, there may be cases where you still need these functions even after
/// disabling the magic key code. In that case, define this macro.
// #define KEYBALL_KEEP_MAGIC_FUNCTIONS

//////////////////////////////////////////////////////////////////////////////
// Constants

#define KEYBALL_TX_GETINFO_INTERVAL 500
#define KEYBALL_TX_GETINFO_MAXTRY 10
#define KEYBALL_TX_GETMOTION_INTERVAL 4

#if (PRODUCT_ID & 0xff00) == 0x0000
#define KEYBALL_MODEL 46
#elif (PRODUCT_ID & 0xff00) == 0x0100
#define KEYBALL_MODEL 61
#elif (PRODUCT_ID & 0xff00) == 0x0200
#define KEYBALL_MODEL 39
#elif (PRODUCT_ID & 0xff00) == 0x0300
#define KEYBALL_MODEL 147
#elif (PRODUCT_ID & 0xff00) == 0x0400
#define KEYBALL_MODEL 44
#endif

#define KEYBALL_OLED_MAX_PRESSING_KEYCODES 6

//////////////////////////////////////////////////////////////////////////////
// Types

enum keyball_keycodes {
  // Configuration (Kb 0..)
  KBC_RST = QK_KB_0,  // Keyball configuration: reset to default
  KBC_SAVE = QK_KB_1, // Keyball configuration: save to EEPROM

  // (removed) Pointer parameter adjustment keycodes were deprecated in favor of OLED setting UI

  // Setting view (Kb 2)
  STG_TOG = QK_KB_2, // Toggle setting view

  // Scroll control (Kb 3..4)  [番号を詰めた]
  // Only mode toggles are provided. Others are handled via OLED setting UI.
  SCRL_TO  = QK_KB_3,  // Toggle scroll mode
  SCRL_MO  = QK_KB_4,  // Momentary scroll mode

  // Scroll snap (Kb 5..7)  [番号を詰めた]
#if KEYBALL_SCROLLSNAP_ENABLE == 2
  SSNP_VRT = QK_KB_5,  // Set scroll snap mode to vertical
  SSNP_HOR = QK_KB_6,  // Set scroll snap mode to horizontal
  SSNP_FRE = QK_KB_7,  // Disable scroll snap mode (free)
#endif

  // Swipe action keys (Kb 8..12)  [番号を詰めた]
  APP_SW  = QK_KB_8,
  VOL_SW  = QK_KB_9,
  BRO_SW  = QK_KB_10,
  TAB_SW  = QK_KB_11,
  WIN_SW  = QK_KB_12,

  // Multi action keys (Kb 13..16): A,B,C,D correspond to LEFT,RIGHT,UP,DOWN  [番号を詰めた]
  MULTI_A = QK_KB_13,
  MULTI_B = QK_KB_14,
  MULTI_C = QK_KB_15,
  MULTI_D = QK_KB_16,

  // Arrow proxy swipe key (Kb 17)  [番号を詰めた]
  SW_ARR  = QK_KB_17,

  // Extension swipe keys (Kb 18..19): for user-expansion examples  [番号を詰めた]
  SW_EX1  = QK_KB_18,
  SW_EX2  = QK_KB_19,

  // Speed adjustments (Kb 20..23)
  // Scroll Speed (ScSp): 1..7, center=4
  SCSP_DEC = QK_KB_20, // Scroll speed -
  SCSP_INC = QK_KB_21, // Scroll speed +
  // Mouse Speed (MoSp): CPI ± step (100)
  MOSP_DEC = QK_KB_22, // Mouse speed - (CPI down)
  MOSP_INC = QK_KB_23, // Mouse speed + (CPI up)

  KEYBALL_SAFE_RANGE,

  // (removed) AML target layer adjustment

  // User customizable keycodes start here.
};

// ユーザーレベルのカスタムキーは、ユーザー側ヘッダで
//   enum custom_keycodes { USER_X = KEYBALL_SAFE_RANGE, USER_Y, ... };
// のように KEYBALL_SAFE_RANGE から連番で定義します。

// Debug aliases removed

// Vial ではユーザーレベル（QK_USER_*）のキーは customKeycodes へ掲載しません。
// 掲載するのはキーボードレベル（QK_KB_*）のみとします。

// 互換用のエイリアスは削除しました

typedef union {
  uint32_t raw;
  struct {
    uint16_t cpi;
    uint8_t sdiv : 3; // scroll divider (deprecated, kept for bitfield layout)
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
    uint8_t amle : 1;   // automatic mouse layer enabled
    uint16_t amlto : 5; // automatic mouse layer timeout
#endif
#if KEYBALL_SCROLLSNAP_ENABLE == 2
    uint8_t ssnap : 2; // scroll snap mode
#endif
  };
} keyball_config_t;

typedef struct {
  uint8_t ballcnt; // count of balls: support only 0 or 1, for now
} keyball_info_t;

typedef struct {
  int16_t x;
  int16_t y;
} keyball_motion_t;

typedef uint8_t keyball_cpi_t;

typedef struct {
  bool this_have_ball;
  bool that_enable;
  bool that_have_ball;

  keyball_motion_t this_motion;
  keyball_motion_t that_motion;

  uint16_t cpi_value;

  bool scroll_mode;
  uint32_t scroll_mode_changed;
  uint8_t scroll_div;

#if KEYBALL_SCROLLSNAP_ENABLE == 1
  uint32_t scroll_snap_last;
  int8_t scroll_snap_tension_h;
#elif KEYBALL_SCROLLSNAP_ENABLE == 2
  keyball_scrollsnap_mode_t scrollsnap_mode;
#endif

  uint16_t last_kc;
  keypos_t last_pos;
  report_mouse_t last_mouse;
  uint8_t last_layer; // 送信時の最上位レイヤ
  uint8_t last_mods;  // 送信時の修飾キー状態

  // Buffer to indicate pressing keys.
  char pressing_keys[KEYBALL_OLED_MAX_PRESSING_KEYCODES + 1];
} keyball_t;

typedef enum {
  KEYBALL_ADJUST_PENDING = 0,
  KEYBALL_ADJUST_PRIMARY = 1,
  KEYBALL_ADJUST_SECONDARY = 2,
} keyball_adjust_t;

//////////////////////////////////////////////////////////////////////////////
// Exported values (touch carefully)

extern keyball_t keyball;

uint8_t keyball_os_idx(void);

//////////////////////////////////////////////////////////////////////////////
// Hook points

/// keyball_on_adjust_layout is called when the keyboard layout adjustted
void keyball_on_adjust_layout(keyball_adjust_t v);

/// keyball_on_apply_motion_to_mouse_move applies trackball's motion m to r as
/// mouse movement.
/// You can change the default algorithm by override this function.
void keyball_on_apply_motion_to_mouse_move(report_mouse_t *r, report_mouse_t *o,
                                           bool is_left);

/// keyball_on_apply_motion_to_mouse_scroll applies trackball's motion m to r
/// as mouse scroll.
/// You can change the default algorithm by override this function.
void keyball_on_apply_motion_to_mouse_scroll(report_mouse_t *r,
                                             report_mouse_t *o, bool is_left);

//////////////////////////////////////////////////////////////////////////////
// Public API functions

/// keyball_get_scroll_mode gets current scroll mode.
bool keyball_get_scroll_mode(void);

/// keyball_set_scroll_mode modify scroll mode.
void keyball_set_scroll_mode(bool mode);

/// keyball_get_scroll_div gets current scroll step (ST) level.
/// See also keyball_set_scroll_div for details.
uint8_t keyball_get_scroll_div(void);

/// keyball_set_scroll_div changes scroll step (ST) level.
///
/// ST はスクロール速度の段階を表し、数値が大きいほど速くなります。
/// 有効範囲は 1..SCROLL_DIV_MAX（既定=4）で、範囲外はクランプされます。
void keyball_set_scroll_div(uint8_t div);

/// keyball_get_cpi gets current CPI of trackball.
/// The actual CPI value is the returned value +1 and multiplied by 100:
///
///     CPI = (v + 1) * 100
uint16_t keyball_get_cpi(void);

/// keyball_set_cpi changes CPI of trackball.
/// Valid values are between 0 to 119, and the actual CPI value is the set
/// value +1 and multiplied by 100:
///
///     CPI = (v + 1) * 100
///
/// In addition, if you do not upload SROM, the maximum value will be limited
/// to 34 (3500CPI).
void keyball_set_cpi(uint16_t cpi);


// ---- Keyball専用 EEPROM ブロック（VIA不使用前提）----
typedef struct __attribute__((packed)) {
  uint32_t magic;           // 'KBP1'
  uint16_t version;         // structure version
  uint16_t reserved;        // 0
  uint16_t cpi[8];          // 100..CPI_MAX
  // スクロール設定
  uint8_t scroll_step[8];   // ST: 1..7（内部的には1..7相当。0は最小相当）
  uint8_t scroll_invert[8]; // 0/1
  // ポインタ移動（Move shaping）設定
  uint8_t move_gain_lo_fp[8]; // 固定小数点(1/256)。16..255 推奨
  uint8_t move_th1[8];        // 0..(move_th2-1)
  uint8_t move_th2[8];        // 1..63 など適当な上限
  // スクロールの新パラメータ（OS別保存）。scroll_step は ST(感度段)として利用する。
  // interval: 蓄積のしきい値（大きいほどゆっくり）
  // value   : 出力量の分母（大きいほど小さく出る）
  uint8_t scroll_interval[8];
  uint8_t scroll_value[8];
  // スクロールプリセット選択（OS別保存）
  // 0: {120,1}, 1: {1,1}, 2: {120,120}
  uint8_t scroll_preset[8];
  // スワイプ設定
  uint16_t swipe_step;        // 発火しきい値
  uint8_t  swipe_deadzone;    // デッドゾーン
  uint8_t  swipe_freeze;      // bit0: freeze (1=FREEZE ON)
  uint8_t  swipe_reset_ms;    // スワイプ蓄積リセット遅延(ms)
  uint8_t  swipe_haptic_mode; // スワイプ時のハプティックモード
  uint8_t  swipe_haptic_mode_repeat; // 連続スワイプ時のハプティックモード
  uint16_t swipe_haptic_idle_ms;     // 初回へ戻すまでの待ち時間(ms)
  // 互換保持のため残置（現行ロジックでは未使用）
  uint8_t scroll_deadzone;   // スクロール用デッドゾーン
  uint8_t scroll_hysteresis; // スクロール反転ヒステリシス
  // Move(deadzone)
  uint8_t  move_deadzone;    // 0..32 程度の小さなデッドゾーン
  // Auto mouse layer settings
  uint8_t  aml_enable;      // 0/1
  uint8_t  aml_layer;       // target layer index (0..31, 0xFF=unset)
  uint16_t aml_timeout;     // ms
  uint16_t aml_threshold;   // activation threshold (counts, 50..1000)
  // Scroll snap (global)
  uint8_t  scrollsnap_mode; // keyball_scrollsnap_mode_t
  // Scroll snap parameters (global)
  uint16_t  scrollsnap_thr;   // tension threshold to temporarily free
  uint16_t scrollsnap_rst_ms; // time to keep FREE before restoring
  // Horizontal scroll gain as a percentage of vertical (global)
  // 1..100 (%). 100% = KEYBALL_SCROLL_VER_GAIN_FP と同等
  uint8_t  scroll_hor_gain_pct;
  // Auto scroll layer configuration (global)
  uint8_t  scroll_layer_enable; // 0/1
  uint8_t  scroll_layer_index;  // 0..31
  // Default base layer configuration (global)
  uint8_t  default_layer;   // 0..31 (QMK default layer index)
} keyball_profiles_t;

#define KBPF_MAGIC 0x4B425031u /* 'KBP1' */

extern keyball_profiles_t kbpf;

#define KBPF_VER_OLD 7
#define KBPF_VER_CUR 16 // v16: swipe haptic 2段階設定を追加（互換なし）

//////////////////////////////////////////////////////////////////////////////
// OS-dependent key tap helper (KB-level)
//
// ユーザー/キーボード両方から共通利用するため、KBレベルで提供します。
// host OS を検出して、対応するキーコードを tap_code16() で送出します。
// それぞれ未使用の場合は KC_NO を指定してください。
void tap_code16_os(uint16_t win, uint16_t mac, uint16_t ios, uint16_t linux, uint16_t unsure);

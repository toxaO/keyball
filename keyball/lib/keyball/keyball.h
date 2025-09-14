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

//////////////////////////////////////////////////////////////////////////////
// Configurations

#ifndef KEYBALL_CPI_DEFAULT
#define KEYBALL_CPI_DEFAULT 2800
#endif

#ifndef KEYBALL_SCROLL_STEP_DEFAULT
#define KEYBALL_SCROLL_STEP_DEFAULT 4
#endif

// 1 なら従来、2 で半段階、3 なら 1/3 ...（分解能を上げたいほど大きく）
#ifndef KEYBALL_SCROLL_FINE_DEN
#define KEYBALL_SCROLL_FINE_DEN 4
#endif
// 入力がしばらく無い時に余りを捨てる（ms）
#ifndef KEYBALL_SCROLL_IDLE_RESET_MS
#define KEYBALL_SCROLL_IDLE_RESET_MS 80
#endif
// 方向が変わったら余りをゼロに（バネ戻り防止）
#ifndef KEYBALL_SCROLL_RESET_ON_DIRCHANGE
#define KEYBALL_SCROLL_RESET_ON_DIRCHANGE 1
#endif
// 1フレームで出す最大ホイール量（スパイク抑制）
#ifndef KEYBALL_SCROLL_FRAME_CLAMP
#define KEYBALL_SCROLL_FRAME_CLAMP 8
#endif

/// To disable scroll snap feature, define 0 in your config.h
#ifndef KEYBALL_SCROLLSNAP_ENABLE
#define KEYBALL_SCROLLSNAP_ENABLE 2
#endif

#ifndef KEYBALL_SCROLLSNAP_RESET_TIMER
#define KEYBALL_SCROLLSNAP_RESET_TIMER 100
#endif

#ifndef KEYBALL_SCROLLSNAP_TENSION_THRESHOLD
#define KEYBALL_SCROLLSNAP_TENSION_THRESHOLD 12
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
#define KEYBALL_MOVE_GAIN_LO_FP 144 // 144/256 ≒ 0.5625
#endif
#ifndef KEYBALL_MOVE_GAIN_HI_FP
#define KEYBALL_MOVE_GAIN_HI_FP 256 // 1.00
#endif

// 速度の近似は max(|x|,|y|) を使用（軽い・十分）
#ifndef KEYBALL_MOVE_TH1
#define KEYBALL_MOVE_TH1 3 // ここまでは低速域ゲイン
#endif
#ifndef KEYBALL_MOVE_TH2
#define KEYBALL_MOVE_TH2 12 // 緩やかにGAIN_LO→GAIN_HIへ線形補間
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

  // Pointer settings (Kb 2..)
  CPI_I100 = QK_KB_2, // CPI +100 CPI
  CPI_D100 = QK_KB_3, // CPI -100 CPI
  MVGL     = QK_KB_4, // 低速ゲイン調整(Shiftで減少)
  MVTH1    = QK_KB_5, // しきい値1調整(Shiftで減少)

  // Scroll control (Kb 6..)
  SCRL_PST = QK_KB_6,  // プリセット（OS別）切替
  SCRL_TO  = QK_KB_7,  // Toggle scroll mode
  SCRL_MO  = QK_KB_8,  // Momentary scroll mode
  SCRL_STI = QK_KB_9,  // Increase scroll step (ST)
  SCRL_STD = QK_KB_10, // Decrease scroll step (ST)
  SCRL_INV = QK_KB_11, // scroll direction inverse

  // Scroll snap (Kb 12..)
  SSNP_VRT = QK_KB_12, // Set scroll snap mode as vertical
  SSNP_HOR = QK_KB_13, // Set scroll snap mode as horizontal
  SSNP_FRE = QK_KB_14, // Set scroll snap mode as disable (free scroll)

  // Automatic mouse layer (Kb 15..)
  AML_TO  = QK_KB_15, // Toggle automatic mouse layer
  AML_I50 = QK_KB_16, // Increment automatic mouse layer timeout
  AML_D50 = QK_KB_17, // Decrement automatic mouse layer timeout

  // Swipe control (Kb 18..)
  SW_RT  = QK_KB_18, // スワイプリセット遅延調整(Shiftで減少)
  SW_ST  = QK_KB_19, // スワイプ閾値調整(Shiftで減少)
  SW_DZ  = QK_KB_20, // スワイプゆらぎ抑制調整(Shiftで減少)
  SW_FRZ = QK_KB_21, // スワイプのポインタフリーズ

  // Debug (Kb 22..)
  DBG_TOG = QK_KB_22, // Toggle debug output
  DBG_NP  = QK_KB_23, // Debug page next
  DBG_PP  = QK_KB_24, // Debug page previous

  // User customizable keycodes start here.
  KEYBALL_SAFE_RANGE = QK_USER_0,
};

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
  uint16_t swipe_step;       // 発火しきい値
  uint8_t  swipe_deadzone;   // デッドゾーン
  uint8_t  swipe_freeze;     // bit0: freeze (1=FREEZE ON)
  uint8_t  swipe_reset_ms;   // スワイプ蓄積リセット遅延(ms)
  // 互換保持のため残置（現行ロジックでは未使用）
  uint8_t scroll_deadzone;   // スクロール用デッドゾーン
  uint8_t scroll_hysteresis; // スクロール反転ヒステリシス
  // Auto mouse layer settings
  uint8_t  aml_enable;      // 0/1
  uint8_t  aml_layer;       // target layer index (0..31, 0xFF=unset)
  uint16_t aml_timeout;     // ms
  // Scroll snap (global)
  uint8_t  scrollsnap_mode; // keyball_scrollsnap_mode_t
} keyball_profiles_t;

#define KBPF_MAGIC 0x4B425031u /* 'KBP1' */

extern keyball_profiles_t kbpf;

// 既定値（ビルド時デフォルトを反映）
#ifndef KB_SW_STEP
#define KB_SW_STEP 180
#endif
#ifndef KB_SW_DEADZONE
#define KB_SW_DEADZONE 1
#endif
#ifndef KB_SWIPE_FREEZE_POINTER
#define KB_SWIPE_FREEZE_POINTER 1
#endif
#ifndef KB_SW_RST_MS
#define KB_SW_RST_MS 30
#endif

#ifndef KB_SCROLL_DEADZONE
#define KB_SCROLL_DEADZONE 0
#endif
#ifndef KB_SCROLL_HYST
#define KB_SCROLL_HYST 0
#endif

#define KBPF_VER_OLD 7
#define KBPF_VER_CUR 8 // v8: AMLとスクロールスナップの永続化を追加（互換なし）

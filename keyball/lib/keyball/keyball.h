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

//////////////////////////////////////////////////////////////////////////////
// Configurations

#ifndef KEYBALL_CPI_DEFAULT
#    define KEYBALL_CPI_DEFAULT 500
#endif

#ifndef KEYBALL_SCROLL_DIV_DEFAULT
#    define KEYBALL_SCROLL_DIV_DEFAULT 3
#endif

// 1 なら従来、2 で半段階、3 なら 1/3 ...（分解能を上げたいほど大きく）
#ifndef KEYBALL_SCROLL_FINE_DEN
#  define KEYBALL_SCROLL_FINE_DEN 4
#endif
// 入力がしばらく無い時に余りを捨てる（ms）
#ifndef KEYBALL_SCROLL_IDLE_RESET_MS
#  define KEYBALL_SCROLL_IDLE_RESET_MS 80
#endif
// 方向が変わったら余りをゼロに（バネ戻り防止）
#ifndef KEYBALL_SCROLL_RESET_ON_DIRCHANGE
#  define KEYBALL_SCROLL_RESET_ON_DIRCHANGE 1
#endif
// 1フレームで出す最大ホイール量（スパイク抑制）
#ifndef KEYBALL_SCROLL_FRAME_CLAMP
#  define KEYBALL_SCROLL_FRAME_CLAMP 8
#endif

/// To disable scroll snap feature, define 0 in your config.h
#ifndef KEYBALL_SCROLLSNAP_ENABLE
#    define KEYBALL_SCROLLSNAP_ENABLE 2
#endif

#ifndef KEYBALL_SCROLLSNAP_RESET_TIMER
#    define KEYBALL_SCROLLSNAP_RESET_TIMER 100
#endif

#ifndef KEYBALL_SCROLLSNAP_TENSION_THRESHOLD
#    define KEYBALL_SCROLLSNAP_TENSION_THRESHOLD 12
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// pointer motion configuration
// ==== Move shaping (pointer) ====
#ifndef KEYBALL_MOVE_SHAPING_ENABLE
#  define KEYBALL_MOVE_SHAPING_ENABLE 1
#endif

// 固定小数点の分母（256が安全）
#define KMF_DEN       256

// 低速域ゲイン（0.25 ≒ 64/256）、高速域ゲイン（1.00 ≒ 256/256）
#ifndef KEYBALL_MOVE_GAIN_LO_FP
#  define KEYBALL_MOVE_GAIN_LO_FP  64     // 0.25
#endif
#ifndef KEYBALL_MOVE_GAIN_HI_FP
#  define KEYBALL_MOVE_GAIN_HI_FP  256    // 1.00
#endif

// 速度の近似は max(|x|,|y|) を使用（軽い・十分）
#ifndef KEYBALL_MOVE_TH1
#  define KEYBALL_MOVE_TH1  2   // ここまでは低速域ゲイン
#endif
#ifndef KEYBALL_MOVE_TH2
#  define KEYBALL_MOVE_TH2  12  // 緩やかにGAIN_LO→GAIN_HIへ線形補間
#endif

#ifndef KEYBALL_MOVE_IDLE_RESET_MS
#  define KEYBALL_MOVE_IDLE_RESET_MS  80  // アイドルでaccを捨てて跳ね防止
#endif

// XYのクランプ（QMK標準は±127）
#ifndef MOUSE_REPORT_XY_MIN
#  define MOUSE_REPORT_XY_MIN  (-127)
#endif
#ifndef MOUSE_REPORT_XY_MAX
#  define MOUSE_REPORT_XY_MAX  (127)
#endif


// 基本はOLED有効
#ifndef OLED_DRIVER_ENABLE
#  define KEYBALL_OLED_ENABLE 1
#endif

/// Specify SROM ID to be uploaded PMW3360DW (optical sensor).  It will be
/// enabled high CPI setting or so.  Valid valus are 0x04 or 0x81.  Define this
/// in your config.h to be enable.  Please note that using this option will
/// increase the firmware size by more than 4KB.
//#define KEYBALL_PMW3360_UPLOAD_SROM_ID 0x04
//#define KEYBALL_PMW3360_UPLOAD_SROM_ID 0x81

/// Defining this macro keeps two functions intact: keycode_config() and
/// mod_config() in keycode_config.c.
///
/// These functions customize the magic key code and are useless if the magic
/// key code is disabled.  Therefore, Keyball automatically disables it.
/// However, there may be cases where you still need these functions even after
/// disabling the magic key code. In that case, define this macro.
//#define KEYBALL_KEEP_MAGIC_FUNCTIONS

//////////////////////////////////////////////////////////////////////////////
// Constants

#define KEYBALL_TX_GETINFO_INTERVAL 500
#define KEYBALL_TX_GETINFO_MAXTRY 10
#define KEYBALL_TX_GETMOTION_INTERVAL 4

#if (PRODUCT_ID & 0xff00) == 0x0000
#    define KEYBALL_MODEL 46
#elif (PRODUCT_ID & 0xff00) == 0x0100
#    define KEYBALL_MODEL 61
#elif (PRODUCT_ID & 0xff00) == 0x0200
#    define KEYBALL_MODEL 39
#elif (PRODUCT_ID & 0xff00) == 0x0300
#    define KEYBALL_MODEL 147
#elif (PRODUCT_ID & 0xff00) == 0x0400
#    define KEYBALL_MODEL 44
#endif

#define KEYBALL_OLED_MAX_PRESSING_KEYCODES 6

//////////////////////////////////////////////////////////////////////////////
// Types

enum keyball_keycodes {
    KBC_RST  = QK_KB_0, // Keyball configuration: reset to default
    KBC_SAVE = QK_KB_1, // Keyball configuration: save to EEPROM

    CPI_I100 = QK_KB_2, // CPI +100 CPI
    CPI_D100 = QK_KB_3, // CPI -100 CPI
    CPI_I1K  = QK_KB_4, // CPI +1000 CPI
    CPI_D1K  = QK_KB_5, // CPI -1000 CPI

    // In scroll mode, motion from primary trackball is treated as scroll
    // wheel.
    SCRL_TO  = QK_KB_6, // Toggle scroll mode
    SCRL_MO  = QK_KB_7, // Momentary scroll mode
    SCRL_DVI = QK_KB_8, // Increment scroll divider
    SCRL_DVD = QK_KB_9, // Decrement scroll divider

    SSNP_VRT = QK_KB_13, // Set scroll snap mode as vertical
    SSNP_HOR = QK_KB_14, // Set scroll snap mode as horizontal
    SSNP_FRE = QK_KB_15, // Set scroll snap mode as disable (free scroll)

    // Auto mouse layer control keycodes.
    // Only works when POINTING_DEVICE_AUTO_MOUSE_ENABLE is defined.
    AML_TO   = QK_KB_10, // Toggle automatic mouse layer
    AML_I50  = QK_KB_11, // Increment automatic mouse layer timeout
    AML_D50  = QK_KB_12, // Decrement automatic mouse layer timeout

    // not offical
    SCRL_INV = QK_KB_16, // scroll direction inverse.


    MVGL_UP = QK_KB_17,   // 低速ゲイン↑
    MVGL_DN = QK_KB_18,   // 低速ゲイン↓
    MVTH1_UP = QK_KB_19,  // しきい値1↑
    MVTH1_DN = QK_KB_20,  // しきい値1↓

    SW_ST_U = QK_KB_21, // スワイプ閾値上昇
    SW_ST_D = QK_KB_22, // スワイプ閾値下降
    SW_DZ_U = QK_KB_23, // スワイプゆらぎ抑制（強）
    SW_DZ_D = QK_KB_24, // スワイプゆらぎ抑制（弱）
    SW_FRZ  = QK_KB_25, // スワイプのポインタフリーズ

    DBG_TOG = QK_KB_26, // Toggle debug output
    DBG_NP = QK_KB_27, // Debug page next
    DBG_PP = QK_KB_28, // Debug page previous

    // User customizable 32 keycodes.
    KEYBALL_SAFE_RANGE = QK_USER_0,
};

typedef union {
    uint32_t raw;
    struct {
        uint16_t cpi;
        uint8_t sdiv : 3;  // scroll divider
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
        uint8_t amle : 1;  // automatic mouse layer enabled
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

typedef enum {
    KEYBALL_SCROLLSNAP_MODE_VERTICAL   = 0,
    KEYBALL_SCROLLSNAP_MODE_HORIZONTAL = 1,
    KEYBALL_SCROLLSNAP_MODE_FREE       = 2,
} keyball_scrollsnap_mode_t;

typedef struct {
    bool this_have_ball;
    bool that_enable;
    bool that_have_ball;

    keyball_motion_t this_motion;
    keyball_motion_t that_motion;

    uint16_t cpi_value;

    bool     scroll_mode;
    uint32_t scroll_mode_changed;
    uint8_t  scroll_div;

#if KEYBALL_SCROLLSNAP_ENABLE == 1
    uint32_t scroll_snap_last;
    int8_t   scroll_snap_tension_h;
#elif KEYBALL_SCROLLSNAP_ENABLE == 2
    keyball_scrollsnap_mode_t scrollsnap_mode;
#endif

    uint16_t       last_kc;
    keypos_t       last_pos;
    report_mouse_t last_mouse;

    // Buffer to indicate pressing keys.
    char pressing_keys[KEYBALL_OLED_MAX_PRESSING_KEYCODES + 1];
} keyball_t;

typedef enum {
    KEYBALL_ADJUST_PENDING   = 0,
    KEYBALL_ADJUST_PRIMARY   = 1,
    KEYBALL_ADJUST_SECONDARY = 2,
} keyball_adjust_t;

//////////////////////////////////////////////////////////////////////////////
// Exported values (touch carefully)

extern keyball_t keyball;

//////////////////////////////////////////////////////////////////////////////
// Hook points

/// keyball_on_adjust_layout is called when the keyboard layout adjustted
void keyball_on_adjust_layout(keyball_adjust_t v);

/// keyball_on_apply_motion_to_mouse_move applies trackball's motion m to r as
/// mouse movement.
/// You can change the default algorithm by override this function.
void keyball_on_apply_motion_to_mouse_move(report_mouse_t *r, report_mouse_t *o, bool is_left);

/// keyball_on_apply_motion_to_mouse_scroll applies trackball's motion m to r
/// as mouse scroll.
/// You can change the default algorithm by override this function.
void keyball_on_apply_motion_to_mouse_scroll(report_mouse_t *r, report_mouse_t *o, bool is_left);

//////////////////////////////////////////////////////////////////////////////
// Public API functions

/// keyball_oled_render_ballinfo renders ball information to OLED.
/// It uses just 21 columns to show the info.
void keyball_oled_render_ballinfo(void);

/// keyball_oled_render_keyinfo renders last processed key information to OLED.
/// It shows column, row, key code, and key name (if available).
void keyball_oled_render_keyinfo(void);

/// keyball_oled_render_layerinfo renders current layer status information to
/// OLED.  It shows layer mask with number (1~f) for active layers and '_' for
/// inactive layers.
void keyball_oled_render_layerinfo(void);

// show current swipe status
void keyball_oled_render_swipe_debug(void);

/// show mouse motion config
void keyball_oled_render_ballsubinfo(void);

/// keyball_get_scroll_mode gets current scroll mode.
bool keyball_get_scroll_mode(void);

/// keyball_set_scroll_mode modify scroll mode.
void keyball_set_scroll_mode(bool mode);

/// keyball_get_scrollsnap_mode gets current scroll snap mode.
keyball_scrollsnap_mode_t keyball_get_scrollsnap_mode(void);

/// keyball_set_scrollsnap_mode change scroll snap mode.
void keyball_set_scrollsnap_mode(keyball_scrollsnap_mode_t mode);

/// keyball_get_scroll_div gets current scroll divider.
/// See also keyball_set_scroll_div for the scroll divider's detail.
uint8_t keyball_get_scroll_div(void);

/// keyball_set_scroll_div changes scroll divider.
///
/// The scroll divider is the number that divides the raw value when applying
/// trackball motion to scrolling.  The CPI value of the trackball is very
/// high, so if you apply it to scrolling as is, it will scroll too much.
/// In order to adjust the scroll amount to be appropriate, it is applied after
/// dividing it by a scroll divider.  The actual denominator is determined by
/// the following formula:
///
///   denominator = 2 ^ (div - 1) ^2
///
/// Valid values are between 1 and 7, KEYBALL_SCROLL_DIV_DEFAULT is used when 0
/// is specified.
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


// === Swipe hook (KB-level detect -> user-level action) ===
typedef enum {
    KB_SWIPE_NONE = 0,
    KB_SWIPE_UP,
    KB_SWIPE_DOWN,
    KB_SWIPE_RIGHT,
    KB_SWIPE_LEFT,
} kb_swipe_dir_t;

// ユーザー定義のモードタグ（KBは中身を解釈しない）
typedef uint8_t kb_swipe_tag_t;

// user → KB：スワイプセッション開始/終了
void            keyball_swipe_begin(kb_swipe_tag_t mode_tag);
void            keyball_swipe_end(void);

// user が参照したい状態
bool            keyball_swipe_is_active(void);          // 押下中？
kb_swipe_tag_t  keyball_swipe_mode_tag(void);           // begin() で渡されたタグ
kb_swipe_dir_t  keyball_swipe_direction(void);          // 現在の方向（未実装の間は常に NONE）
bool            keyball_swipe_fired_since_begin(void);  // セッション開始以降に1回でも発火したか
bool            keyball_swipe_consume_fired(void);      // ↑を取得して false に戻す

// KB → user：発火イベント（弱シンボル；実装は user 側。未定義なら呼ばない）
__attribute__((weak)) void keyball_on_swipe_fire(kb_swipe_tag_t mode_tag, kb_swipe_dir_t dir);

// ==== Swipe runtime params ====
typedef struct {
    uint16_t step;     // 発火しきい値（counts）
    uint8_t  deadzone; // デッドゾーン（counts）
    bool     freeze;   // スワイプ中ポインタ凍結
} kb_swipe_params_t;

// 取得・設定
kb_swipe_params_t keyball_swipe_get_params(void);
void keyball_swipe_set_step(uint16_t v);
void keyball_swipe_set_deadzone(uint8_t v);
void keyball_swipe_set_freeze(bool on);
void keyball_swipe_toggle_freeze(void);

// --- OLED UI mode ---
typedef enum { KB_OLED_MODE_NORMAL = 0, KB_OLED_MODE_DEBUG = 1 } kb_oled_mode_t;

void            keyball_oled_mode_toggle(void);
void            keyball_oled_set_mode(kb_oled_mode_t m);
kb_oled_mode_t  keyball_oled_get_mode(void);

// --- Swipe Debug pages (for OLED) ---
void    keyball_swipe_dbg_toggle(void);
void    keyball_swipe_dbg_show(bool on);
void    keyball_swipe_dbg_next_page(void);
void    keyball_swipe_dbg_prev_page(void);
uint8_t keyball_swipe_dbg_get_page(void);

// ==== Swipe params persistence ====
bool keyball_swipe_cfg_load(void);   // 起動時に呼ぶ: true=読めた, false=初期化
void keyball_swipe_cfg_save(void);   // 現在の params を保存
void keyball_swipe_cfg_reset(void);  // 既定値に戻して保存（=工場出荷）


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
  uint16_t step;     // 発火しきい値
  uint8_t  deadzone; // デッドゾーン
  uint8_t  freeze;    // bit0: freeze (1=FREEZE ON)
} keyball_profiles_t;

#define KBPF_MAGIC   0x4B425031u /* 'KBP1' */

extern keyball_profiles_t kbpf;

// 既定値（ビルド時デフォルトを反映）
#ifndef KB_SW_STEP
#  define KB_SW_STEP 200
#endif
#ifndef KB_SW_DEADZONE
#  define KB_SW_DEADZONE 1
#endif
#ifndef KB_SWIPE_FREEZE_POINTER
#  define KB_SWIPE_FREEZE_POINTER 1
#endif

#define KBPF_VER_OLD  1   // 例：既存
#define KBPF_VER_CUR  2   // ★ 今回の拡張版

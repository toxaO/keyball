/*
This is the c configuration file for the keymap

Copyright 2022 @Yowkees
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

#ifdef RGBLIGHT_ENABLE
#    define RGBLIGHT_EFFECT_BREATHING
#    define RGBLIGHT_EFFECT_RAINBOW_MOOD
#    define RGBLIGHT_EFFECT_RAINBOW_SWIRL
#    define RGBLIGHT_EFFECT_SNAKE
#    define RGBLIGHT_EFFECT_KNIGHT
#    define RGBLIGHT_EFFECT_CHRISTMAS
#    define RGBLIGHT_EFFECT_STATIC_GRADIENT
#    define RGBLIGHT_EFFECT_RGB_TEST
#    define RGBLIGHT_EFFECT_ALTERNATING
#    define RGBLIGHT_EFFECT_TWINKLE
#endif

#define TAP_CODE_DELAY 5

// AML設定
#define POINTING_DEVICE_AUTO_MOUSE_ENABLE
#define AUTO_MOUSE_DEFAULT_LAYER 5
#define AUTO_MOUSE_LAYER_KEEP_TIME 30000 // マウスレイヤーを維持する時間を30秒に設定

#define TAPPING_TERM 220
#define PERMISSIVE_HOLD

// タップ→ホールドしたときにタップしたキーコードを連続送信するための待受時間
#define QUICK_TAP_TERM 150
#define QUICK_TAP_TERM_PER_KEY
#define AUTO_MOUSE_TIME 450  /* マウスが止まってから元のレイヤーに戻るまでの時間(ms) */
// for CAPS WORD
#define BOTH_SHIFTS_TURNS_ON_CAPS_WORD
#define DOUBLE_TAP_SHIFT_TURNS_ON_CAPS_WORD
// for sm_td
// #define MAX_DEFERRED_EXECUTORS 10

#define AUTO_DISABLE_IME_TIME 300000  /* 自動的に IMEが無効化されるまでの時間(ms) */

// #define SMTD_GLOBAL_RELEASE_TERM 75

#define COMBO_TERM 50  /* default: 50 */

// for DEBOUNCE_TYPE = sym_eager_pk
#undef DEBOUNCE
#define DEBOUNCE 50

// OLED自動消灯時間を所定60秒から10秒に変更
#define OLED_TIMEOUT 10000 // ミリ秒

// レイヤ―数追加
#define DYNAMIC_KEYMAP_LAYER_COUNT 10

#undef KEYBALL_SCROLL_DIV_DEFAULT
// マウス、スクロール速度デフォルト値の設定
#define KEYBALL_SCROLL_DIV_DEFAULT 5 // スクロール速度 (default: 4)
#define KEYBALL_CPI_DEFAULT 900 // マウス速度 (default: 500)

// バックライトのBreathingエフェクトの最大輝度を50%(最大255)に設定
// #define RGBLIGHT_EFFECT_BREATHE_MAX 50

// LEDの最大輝度を制限
#undef RGBLIGHT_LIMIT_VAL  // すでに定義されている値を解除
#define RGBLIGHT_LIMIT_VAL 64  // 0～255 の範囲で設定（例: 100 に制限）


// バックライトの初期値
#define RGBLIGHT_DEFAULT_HUE 106         // バックライトの初期値
#define RGBLIGHT_DEFAULT_SAT 255          // 〃
#define RGBLIGHT_DEFAULT_VAL 50         // 〃

// USB接続時のタイムアウト時間を2.5秒に設定
// # define SPLIT_USB_TIMEOUT 5000 // ミリ秒
// # define SPLIT_USB_TIMEOUT_POLL 25
// # define USB_VBUS_PIN GP24 // USB VBUSのピンを指定

// デバッグ情報を有効化
// #define CONSOLE_ENABLE
// #define DEBUG_MATRIX_SCAN_RATE

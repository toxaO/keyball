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

#    undef RGBLIGHT_DEFAULT_MODE
#    define RGBLIGHT_DEFAULT_MODE RGBLIGHT_MODE_BREATHING

#    define RGBLIGHT_LAYERS
#    define RGBLIGHT_LAYERS_RETAIN_VAL
#    define RGBLIGHT_MAX_LAYERS 32
#endif

// scroll snap
#undef KEYBALL_SCROLLSNAP_ENABLE
#define KEYBALL_SCROLLSNAP_ENABLE 2

// scroll_inv
#define KEYBALL_SCROLL_INVERT 1

// Scroll snap の初期テンション閾値（FREE解放までの直交移動量合計の目安）
#undef  KEYBALL_SCROLLSNAP_TENSION_THRESHOLD
#define KEYBALL_SCROLLSNAP_TENSION_THRESHOLD 200

//default
// 命名更新: CPI -> Mouse Speed (MoSp)
#undef  KEYBALL_MOUSE_SPEED_DEFAULT
#define KEYBALL_MOUSE_SPEED_DEFAULT 2800

// スクロールステップの初期値（ST）: 4 を既定にする
// 命名更新: Scroll Step -> Scroll Speed (ScSp)
#undef  KEYBALL_SCROLL_SPEED_DEFAULT
#define KEYBALL_SCROLL_SPEED_DEFAULT 4

// tap dance
#define TAPPING_TERM 175
#define TAPPING_TERM_PER_KEY

// レイヤー数増加させたい時はundefして16bitを追加
#undef LAYER_STATE_8BIT
#define LAYER_STATE_16BIT

// --- Vial/VIA 共有設定 ----------------------------------------------------

#ifndef DYNAMIC_KEYMAP_LAYER_COUNT
#    define DYNAMIC_KEYMAP_LAYER_COUNT 16
#endif

#ifndef EECONFIG_KB_DATA_SIZE
#    define EECONFIG_KB_DATA_SIZE 128
#endif

#ifdef VIAL_ENABLE
#    ifndef VIA_EEPROM_CUSTOM_CONFIG_SIZE
#        define VIA_EEPROM_CUSTOM_CONFIG_SIZE 128
#    endif

#    ifndef VIA_FIRMWARE_VERSION
#        define VIA_FIRMWARE_VERSION 0x00010002
#    endif

#    define VIAL_COMBO_ENABLE
#    undef TAPPING_TERM_PER_KEY
#endif

// Vial 用: キーボード UID（固有値）
#define VIAL_KEYBOARD_UID {0x4B, 0x66, 0x5A, 0xCA, 0x0B, 0xAD, 0x5B, 0x75}

// Vial 用: アンロックコンボ（VIA互換のための設定）
#define VIAL_UNLOCK_COMBO_ROWS {0, 0}
#define VIAL_UNLOCK_COMBO_COLS {0, 1}

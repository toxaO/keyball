// keyball39
// mymap2.0
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

#    define RGBLIGHT_DEFAULT_MODE RGBLIGHT_MODE_BREATHING + 3
#    define RGBLIGHT_DEFAULT_HUE 127
#    define RGBLIGHT_DEFAULT_SAT 255
#    define RGBLIGHT_DEFAULT_VAL 100
#    undef RGBLIGHT_LIMIT_VAL
#    define RGBLIGHT_LIMIT_VAL 125
#    define RGBLIGHT_LAYERS
#    define RGBLIGHT_LAYERS_RETAIN_VAL
#endif

// repeat key macro用
/* #define M_REPEAT_TH 300 */

// scroll snap
#undef KEYBALL_SCROLLSNAP_ENABLE
#define KEYBALL_SCROLLSNAP_ENABLE 1

// 水平方向の閾値
#undef KEYBALL_SCROLLSNAP_TENSION_THRESHOLD
#define KEYBALL_SCROLLSNAP_TENSION_THRESHOLD 1

//default
#undef  KEYBALL_CPI_DEFAULT
#define KEYBALL_CPI_DEFAULT 1100

#undef KEYBALL_SCROLL_DIV_DEFAULT
#define KEYBALL_SCROLL_DIV_DEFAULT 4

// scroll_inv
//#define KEYBALL_SCROLL_INV 1

/* #define TAP_CODE_DELAY 5 */

// tap dance
#define TAPPING_TERM 175
#define TAPPING_TERM_PER_KEY

// レイヤー数増加させたい時はundefして16bitを追加
#undef LAYER_STATE_8BIT
#define LAYER_STATE_16BIT

// 容量削減
#undef LOCKING_SUPPORT_ENABLE
#undef LOCKING_RESYNC_ENABLE
#define NO_MUSIC_MODE

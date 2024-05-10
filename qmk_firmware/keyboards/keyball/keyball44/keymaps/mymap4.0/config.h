// keyball44
// mymap3.0
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
#    undef  RGBLIGHT_EFFECT_RAINBOW_MOOD
#    undef  RGBLIGHT_EFFECT_RAINBOW_SWIRL
#    undef  RGBLIGHT_EFFECT_SNAKE
#    undef  RGBLIGHT_EFFECT_KNIGHT
#    undef  RGBLIGHT_EFFECT_CHRISTMAS
#    undef  RGBLIGHT_EFFECT_STATIC_GRADIENT
#    undef  RGBLIGHT_EFFECT_RGB_TEST
#    undef  RGBLIGHT_EFFECT_ALTERNATING
#    undef  RGBLIGHT_EFFECT_TWINKLE

#    define RGBLIGHT_DEFAULT_MODE RGBLIGHT_MODE_BREATHING + 3
#    define RGBLIGHT_DEFAULT_HUE 127
#    define RGBLIGHT_DEFAULT_SAT 255
#    define RGBLIGHT_DEFAULT_VAL 100
#    define RGBLIGHT_LAYERS
#    define RGBLIGHT_LAYERS_RETAIN_VAL
#endif

//scroll threshold
#undef KEYBALL_SCROLLSNAP_TENSION_THRESHOL
#define KEYBALL_SCROLLSNAP_TENSION_THRESHOLD 1

#define LED_LAYOUT( \
        L00, L01, L02, L03, L04, L05,        R00, R01, R02, R03, R04, R05, \
        L10, L11, L12, L13, L14, L15,        R10, R11, R12, R13, R14, R15, \
        L20, L21, L22, L23, L24, L25,        R20, R21, R22, R23, R24, R25, \
                  L32, L33,                                 R33, R34, \
        L40, L41, L42, L43, L44,                  R40, R41, R42, R43, R44, \
        L50, L51, L52, L53, L54,                  R50, R51, R52, R53, R54) \
{ \
        L05, L15, L25, \
        L04, L14, L24, \
        L03, L13, L23, L33, \
        L02, L12, L22, L32, \
        L01, L11, L21, \
        L00, L10, L20, \
        L44, L43, L42, L41, L40, \
        L50, L51, L52, L53, L54, \
        R50, R51, R52, R53, R54, \
        R44, R43, R42, R41, R40, \
        R05, R15, R25, \
        R04, R14, R24, R34, \
        R03, R13, R23, R33, \
        R02, R12, R22, \
        R01, R11, R21, \
        R00, R10, R20, \
}

#define RGBLIGHT_LED_MAP LED_LAYOUT( \
     0,  1,  2,  3,  4,  5,            30, 31, 32, 33, 34, 35, \
     6,  7,  8,  9, 10, 11,            36, 37, 49, 39, 40, 41, \
    12, 13, 14, 15, 16, 17,            42, 43, 44, 45, 46, 47, \
            18, 19,                                48, 49,     \
    20, 21, 22, 23, 24,                    50, 51, 52, 53, 54, \
    25, 26, 27, 28, 29,                    55, 56, 57, 58, 59)

//default
#undef  KEYBALL_CPI_DEFAULT
#define KEYBALL_CPI_DEFAULT 1100

#undef KEYBALL_SCROLL_DIV_DEFAULT
#define KEYBALL_SCROLL_DIV_DEFAULT 7

// scroll_inv
//#define KEYBALL_SCROLL_INV 1

// 容量削減
#undef LOCKING_SUPPORT_ENABLE
#undef LOCKING_RESYNC_ENABLE
#define NO_MUSIC_MODE

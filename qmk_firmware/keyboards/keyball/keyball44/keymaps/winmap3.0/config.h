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
#endif

#undef SPLIT_USB_DETECT
#undef  SPLIT_USB_TIMEOUT
// #define SPLIT_USB_TIMEOUT       2500

#define RGBLIGHT_SPLIT
#define RGBLIGHT_SLEEP
#define RGBLIGHT_LAYERS


#define LED_LAYOUT( \
        L00, L01, L02, L03, L04, L05,        R00, R01, R02, R03, R04, R05, \
        L10, L11, L12, L13, L14, L15,        R10, R11, R12, R13, R14, R15, \
        L20, L21, L22, L23, L24, L25,        R20, R21, R22, R23, R24, R25, \
                  L32, L33,                                 R33, R34, \
        L40, L41, L42, L43, L44,                  R40, R41, R42, R43, R44, \
        L50, L51, L52, L53, L54,                  R50, R51, R52, R53, R54) \
{ \
        L25, L22, L14, L10, L03, L00, \
        L32, L23, L15, L11, L04, L01, \
        L33, L24, L20, L12, L05, L02, \
                  L21, L13,           \
        L44, L43, L40, L41, L40,      \
        L50, L51, L52, L53, L54,      \
        R52, R44, R41, R25, R21, R14, \
        R53, R50, R42, R33, R22, R15, \
        R54, R51, R43, R34, R23, R20, \
                       R40, R24,      \
             R13, R12, R11, R10, R05, \
             R00, R01, R02, R03, R04  \
}
/*
{\
        L00, L01, L02, L03, L04, L05, \
        L10, L11, L12, L13, L14, L15, \
        L20, L21, L22, L23, L24, L25, \
                  L32, L33,           \
        L40, L41, L42, L43, L44,      \
        L50, L51, L52, L53, L54,      \
        \
        R00, R01, R02, R03, R04, R05, \
        R10, R11, R12, R13, R14, R15, \
        R20, R21, R22, R23, R24, R25, \
                       R33, R34, \
             R40, R41, R42, R43, R44, \
             R50, R51, R52, R53, R54  \
}
*/


#define RGBLIGHT_LED_MAP LED_LAYOUT( \
     0,  1,  2,  3,  4,  5,            30, 31, 32, 33, 34, 35, \
     6,  7,  8,  9, 10, 11,            36, 37, 49, 39, 40, 41, \
    12, 13, 14, 15, 16, 17,            42, 43, 44, 45, 46, 47, \
            18, 19,                                48, 49,     \
    20, 21, 22, 23, 24,                    50, 51, 52, 53, 54, \
    25, 26, 27, 28, 29,                    55, 56, 57, 58, 59)


// コンボ定義数
//#define COMBO_COUNT 2

#define TAP_CODE_DELAY 5
#define TAPPING_TERM 175
#define TAPPING_TERM_PER_KEY

//default
#undef  KEYBALL_CPI_DEFAULT
#define KEYBALL_CPI_DEFAULT 1400

#undef KEYBALL_SCROLL_DIV_DEFAULT
#define KEYBALL_SCROLL_DIV_DEFAULT 8

//scroll threshold
#undef KEYBALL_SCROLLSNAP_TENSION_THRESHOL
#define KEYBALL_SCROLLSNAP_TENSION_THRESHOLD 4
// scroll_inv
#define KEYBALL_SCROLL_INV 1

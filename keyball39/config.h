/*
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

// Key matrix parameters
#define MATRIX_ROWS         (4 * 2)  // split keyboard
#define MATRIX_COLS         6
#define MATRIX_ROW_PINS     { GP29, GP28, GP27, GP26 }
#define MATRIX_COL_PINS     { GP4, GP5, GP6, GP7, GP8, GP9}
#define MATRIX_MASKED
#define DEBOUNCE            5
#define DIODE_DIRECTION     COL2ROW

// Split parameters
#define SOFT_SERIAL_PIN         GP1
#define SPLIT_HAND_MATRIX_GRID  GP27, GP9
#define SPLIT_HAND_MATRIX_GRID_LOW_IS_LEFT
//#define SPLIT_USB_DETECT
//#define SPLIT_USB_TIMEOUT       500

#define SPLIT_TRANSACTION_IDS_KB KEYBALL_GET_INFO

// RGB LED settings
#define WS2812_DI_PIN       GP0
#ifdef RGBLIGHT_ENABLE
#    define RGBLIGHT_LED_COUNT      48
#    define RGBLED_SPLIT    { 24, 24 }  // (24 + 22)
#    ifndef RGBLIGHT_LIMIT_VAL
#        define RGBLIGHT_LIMIT_VAL  150 // limitated for power consumption
#    endif
#    ifndef RGBLIGHT_VAL_STEP
#        define RGBLIGHT_VAL_STEP   15
#    endif
#    ifndef RGBLIGHT_HUE_STEP
#        define RGBLIGHT_HUE_STEP   17
#    endif
#    ifndef RGBLIGHT_SAT_STEP
#        define RGBLIGHT_SAT_STEP   17
#    endif
#endif
#ifdef RGB_MATRIX_ENABLE
#    define RGB_MATRIX_SPLIT    { 24, 24 }
#endif

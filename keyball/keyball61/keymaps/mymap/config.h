/* mymap config for keyball61 (inherits defaults) */

#pragma once

#define TAP_CODE_DELAY 5

#define POINTING_DEVICE_AUTO_MOUSE_ENABLE
#define AUTO_MOUSE_DEFAULT_LAYER 2

// EEPROM キーボード領域サイズ（kbpf用に十分なサイズを確保）
#undef  EECONFIG_KB_DATA_SIZE
#define EECONFIG_KB_DATA_SIZE 128

// スクロール反転の既定（0=off, 1=on）
#ifndef KEYBALL_SCROLL_INVERT
#define KEYBALL_SCROLL_INVERT 0
#endif

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

// Vial settings (UID must be unique per keyboard)
#define VIAL_KEYBOARD_UID {0x61, 0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07}
// Unlock combo: (row,col) = (0,0) + (0,1)
#define VIAL_UNLOCK_COMBO_ROWS { 0, 0 }
#define VIAL_UNLOCK_COMBO_COLS { 0, 1 }

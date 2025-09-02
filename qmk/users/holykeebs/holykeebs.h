#pragma once

#include QMK_KEYBOARD_H

enum hk_keycodes {
    // Saves the HK configuration to EEPROM. By default any changes to the HK configuration are not saved to EEPROM, so
    // use this if you want your changes to persist.
    HK_SAVE_SETTINGS = QK_KB_0, // 0x7E00

    // Resets the HK configuration to default values. This is unlike QK_CLEAR_EEPROM, which clears all EEPROM data.
    HK_RESET_SETTINGS, // 0x7E01

    // Dumps the HK configuration to the console. Only useful when CONSOLE_ENABLE is set.
    HK_DUMP_SETTINGS, // 0x7E02

    // The behavior the keycodes below enable can be used with either a single or dual pointing device setup. Normally,
    // it's applied to the main side. To apply it to the peripheral side, hold shift.

    // Sets a scaling factor on x/y movement in the corresponding mode. The default scaling factor is determined by the
    // type of pointing device.
    //
    // Hold these keys and then press + or - to increase or decrease the corresponding scaling factor.
    HK_POINTER_SET_DEFAULT_SCALER, // 0x7E03
    HK_POINTER_SET_SNIPING_SCALER, // 0x7E04

    // Sets the scroll buffer size. The buffer acccumulates x/y movements and outputs a h/v value when the thershold is reached.
    //
    // Hold this key and then press + or - to increase or decrease the buffer size.
    HK_POINTER_SET_SCROLL_BUFFER, // 0x7E05

    // Enters sniping mode.
    HK_SNIPING_MODE, // 0x7E06

    // Toggles sniping mode.
    HK_SNIPING_MODE_TOGGLE, // 0x7E07

    // Enters drag-scroll mode. Drag scroll translates x/y movements to h/v movements.
    HK_DRAGSCROLL_MODE, // 0x7E08

    // Toggles drag-scroll mode.
    HK_DRAGSCROLL_MODE_TOGGLE, // 0x7E09

    // Scroll lock filters out any scroll movement that is not in the specified direction. This cycles through the
    // possible modes: off, horizontal, vertical.
    HK_CYCLE_SCROLL_LOCK, // 0x7E0A
};

#define HK_SAVE      HK_SAVE_SETTINGS
#define HK_RESET     HK_RESET_SETTINGS
#define HK_DUMP      HK_DUMP_SETTINGS
#define HK_P_SET_D   HK_POINTER_SET_DEFAULT_SCALER
#define HK_P_SET_S   HK_POINTER_SET_SNIPING_SCALER
#define HK_P_SET_BUF HK_POINTER_SET_SCROLL_BUFFER
#define HK_S_MODE    HK_SNIPING_MODE
#define HK_S_MODE_T  HK_SNIPING_MODE_TOGGLE
#define HK_D_MODE    HK_DRAGSCROLL_MODE
#define HK_D_MODE_T  HK_DRAGSCROLL_MODE_TOGGLE
#define HK_C_SCROLL  HK_CYCLE_SCROLL_LOCK

// #define ENABLE_DRIFT_DETECTION
// #define ENABLE_PIMORONI_ADAPTIVE_MOTION

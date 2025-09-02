#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "eeconfig.h"

typedef union PACKED {
    uint8_t raw[EECONFIG_USER_DATA_SIZE];
    struct {
        struct {
            hk_cursor_mode main_cursor_mode : 2;
            bool main_drag_scroll : 1;
            hk_scroll_lock main_scroll_lock : 2;
            int16_t main_default_multiplier;
            int16_t main_sniping_multiplier;
            uint8_t main_scroll_buffer_size;

            hk_cursor_mode peripheral_cursor_mode : 2;
            bool peripheral_drag_scroll : 1;
            hk_scroll_lock peripheral_scroll_lock : 2;
            int16_t peripheral_default_multiplier;
            int16_t peripheral_sniping_multiplier;
            uint8_t peripheral_scroll_buffer_size;
        } pointing;

        bool check       : 1;
    };
} hk_eeprom_config_t;

_Static_assert(sizeof(hk_eeprom_config_t) <= EECONFIG_USER_DATA_SIZE, "User EECONFIG block is not large enough.");

extern hk_eeprom_config_t hk_eeprom_config;

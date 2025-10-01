/* Copyright 2017 Yang Liu
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define _RGBM_SINGLE_DYNAMIC(sym) RGBLIGHT_MODE_##sym,

enum RGBLIGHT_EFFECT_MODE {
    RGBLIGHT_MODE_zero = 0,
#include "rgblight_modes.h"
    RGBLIGHT_MODE_last
};


#define RGBLIGHT_MODES (RGBLIGHT_MODE_last - 1)

#ifndef RGBLIGHT_VAL_STEP
#    define RGBLIGHT_VAL_STEP 50
#endif
#ifndef RGBLIGHT_LIMIT_VAL
#    define RGBLIGHT_LIMIT_VAL 150
#endif

#include <stdint.h>
#include <stdbool.h>
#include "progmem.h"
#include "ws2812.h"
#include "color.h"

extern LED_TYPE led[RGBLED_NUM];

extern bool  is_rgblight_initialized;

typedef union {
    struct {
        bool    enable  : 1;
        bool    rainbow_mode : 1;
        uint8_t mode    : 6;
        uint8_t hue     : 8;
        uint8_t sat     : 8;
        uint8_t val     : 8;
    };
} rgblight_config_t;


typedef struct _rgblight_status_t {
    uint8_t base_mode;
    bool    timer_enabled;
#ifdef RGBLIGHT_SPLIT
    uint8_t change_flags;
#endif

} rgblight_status_t;

/*
 * Structure for RGB Light clipping ranges
 */
typedef struct _rgblight_ranges_t {
    uint8_t clipping_start_pos;
    uint8_t clipping_num_leds;
    uint8_t effect_start_pos;
    uint8_t effect_end_pos;
    uint8_t effect_num_leds;
} rgblight_ranges_t;

extern rgblight_ranges_t rgblight_ranges;

/* === Utility Functions ===*/
void sethsv(uint8_t hue, uint8_t sat, uint8_t val, LED_TYPE *led1);
void sethsv_raw(uint8_t hue, uint8_t sat, uint8_t val, LED_TYPE *led1); // without RGBLIGHT_LIMIT_VAL check
void setrgb(uint8_t r, uint8_t g, uint8_t b, LED_TYPE *led1);

/* === Low level Functions === */
void rgblight_set(void);
void rgblight_set_clipping_range(uint8_t start_pos, uint8_t num_leds);

/* === Effects and Animations Functions === */
/*   effect range setting */
void rgblight_set_effect_range(uint8_t start_pos, uint8_t num_leds);

/*   effect mode change */
void rgblight_mode(uint8_t mode);

/*   effects mode disab-le/enable */
void rgblight_toggle(void);
void rgblight_enable(void);
void rgblight_disable(void);

/*   hue, sat, val change */
void rgblight_increase_val(void);
void rgblight_decrease_val(void);
void rgblight_sethsv(uint8_t hue, uint8_t sat, uint8_t val);

/*       query */
uint8_t rgblight_get_mode(void);
uint8_t rgblight_get_val(void);
bool    rgblight_is_enabled(void);


/* === qmk_firmware (core)internal Functions === */
void     rgblight_init(void);
void     eeconfig_update_rgblight_default(void);

#define EZ_RGB(val) rgblight_show_solid_color((val >> 16) & 0xFF, (val >> 8) & 0xFF, val & 0xFF)
void rgblight_show_solid_color(uint8_t r, uint8_t g, uint8_t b);

void rgblight_task(void);

#ifdef RGBLIGHT_SPLIT
#    define RGBLIGHT_STATUS_CHANGE_MODE (1 << 0)
#    define RGBLIGHT_STATUS_CHANGE_HSVS (1 << 1)
#    define RGBLIGHT_STATUS_CHANGE_TIMER (1 << 2)
#    define RGBLIGHT_STATUS_ANIMATION_TICK (1 << 3)
#    define RGBLIGHT_STATUS_CHANGE_LAYERS (1 << 4)

typedef struct _rgblight_syncinfo_t {
    rgblight_config_t config;
    rgblight_status_t status;
} rgblight_syncinfo_t;

/* for split keyboard master side */
uint8_t rgblight_get_change_flags(void);
void    rgblight_clear_change_flags(void);
void    rgblight_get_syncinfo(rgblight_syncinfo_t *syncinfo);
/* for split keyboard slave side */
void rgblight_update_sync(rgblight_syncinfo_t *syncinfo, bool write_to_eeprom);
#endif

void rgblight_value(uint8_t row, uint8_t col, bool update, bool scr);
void rgblight_value_range(uint8_t row, uint8_t col, uint8_t add);

void rgblight_sethue(uint8_t hue);
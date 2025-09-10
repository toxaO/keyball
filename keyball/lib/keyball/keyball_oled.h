#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum { KB_OLED_MODE_NORMAL = 0, KB_OLED_MODE_DEBUG = 1 } kb_oled_mode_t;

void        keyball_oled_mode_toggle(void);
void        keyball_oled_set_mode(kb_oled_mode_t m);
kb_oled_mode_t keyball_oled_get_mode(void);

void        keyball_oled_dbg_toggle(void);
void        keyball_oled_dbg_show(bool on);
bool        keyball_oled_dbg_enabled(void);

void        keyball_oled_next_page(void);
void        keyball_oled_prev_page(void);
uint8_t     keyball_oled_get_page(void);
uint8_t     keyball_oled_get_page_count(void);

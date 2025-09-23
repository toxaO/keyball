#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "report.h"

typedef enum {
    KEYBALL_SCROLLSNAP_MODE_VERTICAL   = 0,
    KEYBALL_SCROLLSNAP_MODE_HORIZONTAL = 1,
    KEYBALL_SCROLLSNAP_MODE_FREE       = 2,
} keyball_scrollsnap_mode_t;


bool keyball_get_scroll_mode(void);
void keyball_set_scroll_mode(bool mode);
keyball_scrollsnap_mode_t keyball_get_scrollsnap_mode(void);
void keyball_set_scrollsnap_mode(keyball_scrollsnap_mode_t mode);
uint8_t keyball_get_scroll_div(void);
void keyball_set_scroll_div(uint8_t div);

void keyball_on_apply_motion_to_mouse_scroll(report_mouse_t *report,
                                             report_mouse_t *output,
                                             bool is_left);

void keyball_scroll_get_dbg(int16_t *sx, int16_t *sy, int16_t *h, int16_t *v);
void keyball_scroll_get_dbg_inner(int32_t *ah, int32_t *av, int8_t *t);

void keyball_apply_scroll_layer_state(uint32_t state);
void keyball_refresh_scroll_layer(void);

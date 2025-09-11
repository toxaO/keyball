#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "report.h"

typedef enum {
    KEYBALL_SCROLLSNAP_MODE_VERTICAL   = 0,
    KEYBALL_SCROLLSNAP_MODE_HORIZONTAL = 1,
    KEYBALL_SCROLLSNAP_MODE_FREE       = 2,
} keyball_scrollsnap_mode_t;

extern uint8_t g_scroll_deadzone;
extern uint8_t g_scroll_hysteresis;

bool keyball_get_scroll_mode(void);
void keyball_set_scroll_mode(bool mode);
keyball_scrollsnap_mode_t keyball_get_scrollsnap_mode(void);
void keyball_set_scrollsnap_mode(keyball_scrollsnap_mode_t mode);
uint8_t keyball_get_scroll_div(void);
void keyball_set_scroll_div(uint8_t div);

void keyball_on_apply_motion_to_mouse_scroll(report_mouse_t *report,
                                             report_mouse_t *output,
                                             bool is_left);

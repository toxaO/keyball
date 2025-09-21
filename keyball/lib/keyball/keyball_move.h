#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "report.h"

extern int32_t g_move_gain_lo_fp;
extern int16_t g_move_th1;
extern int16_t g_move_th2;

void keyball_on_apply_motion_to_mouse_move(report_mouse_t *report,
                                           report_mouse_t *output,
                                           bool is_left);

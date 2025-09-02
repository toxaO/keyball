#include <math.h>

#include QMK_KEYBOARD_H
#include "pimoroni.h"

// Modify these to adjust non-linear mouse scaling
#define MAX_SCALE 32
#define MIN_SCALE 1
#define GROWTH_FACTOR 64
#define MOMENTUM 0.01

// Variable to store an exponential moving average scaling factor to denoise the non-linear scaling
float accumulated_factor = MIN_SCALE;

// add non-linear scaling to all mouse movements
void pimoroni_adaptive_motion(report_mouse_t* mouse_report) {
    // compute the size of the last mouse movement
    float mouse_length = sqrt(mouse_report->x*mouse_report->x + mouse_report->y*mouse_report->y);

    // compute an instantaneous scaling factor and update exponential moving average
    float factor =  GROWTH_FACTOR*mouse_length+ MIN_SCALE;
    accumulated_factor = accumulated_factor*(1-MOMENTUM) + factor*MOMENTUM;

    if (accumulated_factor > MAX_SCALE) {
        // clamp the scaling factor to avoid overflowing mouse_report
        mouse_report->x *= MAX_SCALE;
        mouse_report->y *= MAX_SCALE;
    }
    else {
        // scale up the mouse movement by the average factor
        mouse_report->x = (int16_t)(mouse_report->x * accumulated_factor);
        mouse_report->y = (int16_t)(mouse_report->y * accumulated_factor);
    }
}

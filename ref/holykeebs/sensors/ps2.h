#pragma once

#include "pointing_device.h"

uint16_t ps2_driver_get_cpi(void);
void ps2_driver_set_cpi(uint16_t cpi);
report_mouse_t ps2_driver_read(report_mouse_t mouse_report);

const pointing_device_driver_t ps2_pointing_device_driver;

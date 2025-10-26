#pragma once

#include <stdint.h>

void keyball_led_monitor_init(void);
void keyball_led_monitor_on(void);
void keyball_led_monitor_off(void);
void keyball_led_monitor_step(int8_t delta);
uint8_t keyball_led_monitor_get_index(void);

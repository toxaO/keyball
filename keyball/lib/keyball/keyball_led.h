#pragma once

#include <stdint.h>

void keyball_led_monitor_init(void);
void keyball_led_monitor_on(void);
void keyball_led_monitor_off(void);
void keyball_led_monitor_step(int8_t delta);
uint8_t keyball_led_monitor_get_index(void);

void keyball_led_set_hsv_at(uint8_t hue, uint8_t sat, uint8_t val, uint8_t index);
void keyball_led_set_hsv_range(uint8_t hue, uint8_t sat, uint8_t val, uint8_t index, uint8_t count);

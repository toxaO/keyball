#ifndef MYLIB_OLED_H
#define MYLIB_OLED_H

#include "quantum.h"

void oled_render_layer_state(void) ;
const char *format_4d(int d);
void oled_set_info(void);
void oledkit_render_info_user(void);

#endif

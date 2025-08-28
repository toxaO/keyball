#include "quantum.h"
#include "oled.h"
#include "my_keycode.h"
#include "lib/keyball/keyball.h"

void oled_render_layer_state(void) {
    oled_write_P(PSTR("Layer: "), false);
    switch (get_highest_layer(layer_state)) {
        case _Def:
            oled_write_ln_P(PSTR("Default   "), false);
            break;
        case _NumP:
            oled_write_ln_P(PSTR("NumPad  "), false);
            break;
        case _Sym:
            oled_write_ln_P(PSTR("Sym / Num "), false);
            break;
        case _mCur:
            oled_write_ln_P(PSTR("Cur / Func"), false);
            break;
        case _wCur:
            oled_write_ln_P(PSTR("Cur / Func"), false);
            break;
        case _mMou:
            oled_write_ln_P(PSTR("Mouse     "), false);
            break;
        case _wMou:
            oled_write_ln_P(PSTR("Mouse     "), false);
            break;
        case _Scr:
            oled_write_ln_P(PSTR("Scroll    "), false);
            break;
    }
}

const char *format_4d(int d) {
    static char buf[5] = {0}; // max width (4) + NUL (1)
    char        lead   = ' ';
    if (d < 0) {
        d    = -d;
        lead = '-';
    }
    buf[3] = (d % 10) + '0';
    d /= 10;
    if (d == 0) {
        buf[2] = lead;
        lead   = ' ';
    } else {
        buf[2] = (d % 10) + '0';
        d /= 10;
    }
    if (d == 0) {
        buf[1] = lead;
        lead   = ' ';
    } else {
        buf[1] = (d % 10) + '0';
        d /= 10;
    }
    buf[0] = lead;
    return buf;
}

// test
/* void oled_test(void) { */
/*   if (first_pressed) { */
/*     oled_write_P(PSTR("first: on"), false); */
/*   } else { */
/*     oled_write_P(PSTR("first: off"), false); */
/*   } */
/* } */

void oled_set_info(void) {
    oled_write_P(PSTR("CPI:"), false);
    oled_write(format_4d(keyball_get_cpi()) + 1, false);
    oled_write_P(PSTR(" / DVI: "), false);
    oled_write_char('0' + keyball_get_scroll_div(), false);
    oled_write_ln_P(PSTR(""), false);
}

// OLEDの実装
void oledkit_render_info_user(void) {
    // keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
    // keyball_oled_render_layerinfo();
    /* oled_test(); */
    // oled_set_info();
    oled_render_layer_state();
}

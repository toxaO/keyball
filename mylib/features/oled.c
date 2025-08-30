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


// OLEDの実装
void oledkit_render_info_user(void) {
    // keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
    // keyball_oled_render_layerinfo();
    oled_render_layer_state();
    keyball_oled_render_ballsubinfo();
}

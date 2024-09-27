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
    /* keyball_oled_render_keyinfo(); */
    /* keyball_oled_render_ballinfo(); */
    /* keyball_oled_render_layerinfo(); */
    oled_render_layer_state();
}

// swipeの反応速度のテストに使用したコード
/* void oled_render_repeat_speed(void) { */
/*     oled_write_P(PSTR("re_sp: "), false); */
/*     switch (repeat_speed) { */
/*         case NORMAL: */
/*             oled_write_ln_P(PSTR("NORMAL"), false); */
/*             break; */
/*         case HIGH: */
/*             oled_write_ln_P(PSTR("HIGH"), false); */
/*             break; */
/*         case VERY_HIGH: */
/*             oled_write_ln_P(PSTR("VERY_HIGH"), false); */
/*             break; */
/*     } */
/* } */

/* static const char *format_4d(int d) { */
/*     static char buf[5] = {0}; // max width (4) + NUL (1) */
/*     char        lead   = ' '; */
/*     if (d < 0) { */
/*         d    = -d; */
/*         lead = '-'; */
/*     } */
/*     buf[3] = (d % 10) + '0'; */
/*     d /= 10; */
/*     if (d == 0) { */
/*         buf[2] = lead; */
/*         lead   = ' '; */
/*     } else { */
/*         buf[2] = (d % 10) + '0'; */
/*         d /= 10; */
/*     } */
/*     if (d == 0) { */
/*         buf[1] = lead; */
/*         lead   = ' '; */
/*     } else { */
/*         buf[1] = (d % 10) + '0'; */
/*         d /= 10; */
/*     } */
/*     buf[0] = lead; */
/*     return buf; */
/* } */


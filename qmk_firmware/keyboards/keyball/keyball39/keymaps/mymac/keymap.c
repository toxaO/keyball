// keyball39
// mymap2.0
/*
Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H

#include "quantum.h"
#include "my_keycode.h"
#include "features/swipe.h"
#include "features/macro_key.h"


// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  // keymap for default
  [_Def] = LAYOUT_universal(
 //,--------------------------------------------------------------------------.                                  ,--------------------------------------------------------------------------.
     KC_Q         , KC_W         , KC_E         , KC_R         , KC_T         ,                                    KC_Y         , KC_U         , KC_I         , KC_O         , KC_P         ,
 //,--------------+--------------+--------------+--------------+--------------.                                  ,--------------+--------------+--------------+--------------+--------------.
     KC_A         , KC_S         , KC_D         , KC_F         , KC_G         ,                                    KC_H         , KC_J         , KC_K         , KC_L         , _Mou_SCLN    ,
 //,--------------+--------------+--------------+--------------+--------------.                                  ,--------------+--------------+--------------+--------------+--------------.
     ALT_T(KC_Z)  , GUI_T(KC_X)  , KC_C         , KC_V         , KC_B         ,                                    KC_N         , KC_M         , KC_COMM      , GUI_T(KC_DOT), ALT_T(KC_SLSH),
 //,--------------+--------------+--------------+--------------+--------------+--------------.    ,--------------+--------------+--------------+--------------+--------------+--------------.
     KC_MINUS     , XXX          , XXX          , CTL_T(KC_MINUS) , EISU_S       , _Sym_SPC     ,      _Cur_ENT     , KANA_C       , XXXXXXX      , XXXXXXX      , XXXXXXX      , _Esc_NumP
 //,-----------------------------------------------------------------------------------------.    ,-----------------------------------------------------------------------------------------.
  ),

  [_Sym] = LAYOUT_universal(
 //,--------------------------------------------------------------------------.                                  ,--------------------------------------------------------------------------.
     S(KC_1)      , S(KC_2)      , S(KC_3)      , S(KC_4)      , S(KC_5)      ,                                    S(KC_6)      , S(KC_7)      , S(KC_8)      , S(KC_9)      , TILDE        ,
 //,--------------+--------------+--------------+--------------+--------------.                                  ,--------------+--------------+--------------+--------------+--------------.
     ALT_T(KC_1)  , GUI_T(KC_2)  , CTL_T(KC_3)  , SFT_T(KC_4)  , RCTL_T(KC_5) ,                                    RCTL_T(KC_6) , SFT_T(KC_7)  , CTL_T(KC_8)  , GUI_T(KC_9)  , ALT_T(KC_0)  ,
 //,--------------+--------------+--------------+--------------+--------------.                                  ,--------------+--------------+--------------+--------------+--------------.
     AT           , AST          , CARET        , V_BAR        , B_QUO        ,                                    L_BRC        , R_BRC        , L_SQBR       , R_SQBR       , CLN          ,
 //,--------------+--------------+--------------+--------------+--------------+--------------.    ,--------------+--------------+--------------+--------------+--------------+--------------.
     XXX          , R_D_ARR      , R_ARROW      , EQL          , XXX          , _______      ,      U_BAR        , YEN          , XXXXXXX      , XXXXXXX      , XXXXXXX      , BSLSH
 //,-----------------------------------------------------------------------------------------.    ,-----------------------------------------------------------------------------------------.
  ),

  [_mCur] = LAYOUT_universal(
 //,--------------------------------------------------------------------------.                                  ,--------------------------------------------------------------------------.
     M_F1         , M_F2         , M_F3         , M_F4         , M_F5         ,                                    M_F6         , M_F7         , M_F8         , M_F9         , M_F10        ,
 //,--------------+--------------+--------------+--------------+--------------.                                  ,--------------+--------------+--------------+--------------+--------------.
     KC_LALT      , KC_LGUI      , KC_LCTL      , KC_LSFT      , KC_RCTL      ,                                    KC_LEFT      , KC_DOWN      , KC_UP        , KC_RIGHT     , XXX          ,
 //,--------------+--------------+--------------+--------------+--------------.                                  ,--------------+--------------+--------------+--------------+--------------.
     M_F11        , M_F12        , M_F13        , M_F14        , M_F15        ,                                    m_HOME       , m_PGDN       , m_PGUP       , m_END        , XXX          ,
 //,--------------+--------------+--------------+--------------+--------------+--------------.    ,--------------+--------------+--------------+--------------+--------------+--------------.
     XXX          , XXX          , XXX          , XXX          , XXX          , XXX         ,       _______      , XXX          , XXXXXXX      , XXXXXXX      , XXXXXXX      , XXX
 //,-----------------------------------------------------------------------------------------.    ,-----------------------------------------------------------------------------------------.
  ),

  [_wCur] = LAYOUT_universal(
 //,--------------------------------------------------------------------------.                                  ,--------------------------------------------------------------------------.
     KC_F1        , KC_F2        , KC_F3        , KC_F4        , KC_F5        ,                                    KC_F6        , KC_F7        , KC_F8        , KC_F9        , KC_F10       ,
 //,--------------+--------------+--------------+--------------+--------------.                                  ,--------------+--------------+--------------+--------------+--------------.
     KC_LALT      , KC_LGUI      , KC_LCTL      , KC_LSFT      , KC_RCTL      ,                                    KC_LEFT      , KC_DOWN      , KC_UP        , KC_RIGHT     , XXX          ,
 //,--------------+--------------+--------------+--------------+--------------.                                  ,--------------+--------------+--------------+--------------+--------------.
     KC_F11       , KC_F12       , KC_F13       , KC_F14       , KC_F15       ,                                    KC_HOME      , KC_PGDN      , KC_PGUP      , KC_END       , XXX          ,
 //,--------------+--------------+--------------+--------------+--------------+--------------.    ,--------------+--------------+--------------+--------------+--------------+--------------.
     XXX          , XXX          , XXX          , XXX          , XXX          , XXX         ,       _______      , XXX          , XXXXXXX      , XXXXXXX      , XXXXXXX      , XXX
 //,-----------------------------------------------------------------------------------------.    ,-----------------------------------------------------------------------------------------.
  ),

  [_NumP] = LAYOUT_universal(
 //,--------------------------------------------------------------------------.                                  ,--------------------------------------------------------------------------.
     KC_Q         ,  KC_W        , KC_UP        , XXX          , XXX          ,                                    KC_MINUS     , KC_P7        , KC_P8        , KC_P9        , KC_PPLS      ,
 //,--------------+--------------+--------------+--------------+--------------.                                  ,--------------+--------------+--------------+--------------+--------------.
     KC_A         ,  KC_LEFT     , KC_DOWN      , KC_RIGHT     , M_F2         ,                                    KC_P0        , KC_P4        , KC_P5        , KC_P6        , MO(_mMou)    ,
 //,--------------+--------------+--------------+--------------+--------------.                                  ,--------------+--------------+--------------+--------------+--------------.
     KC_Z         ,  KC_X        , KC_C         , XXX          , XXX          ,                                    KC_PDOT      , KC_P1        , KC_P2        , KC_P3        , KC_PSLS      ,
 //,--------------+--------------+--------------+--------------+--------------+--------------.    ,--------------+--------------+--------------+--------------+--------------+--------------.
     XXX          ,  XXX         , XXX          , KC_ESC       , SFT_T(KC_ENT), KC_SPACE     ,      KC_ENT       , KC_TAB       , XXXXX        , XXXXX        , XXXXX        , TG(_NumP)
 //,-----------------------------------------------------------------------------------------.    ,-----------------------------------------------------------------------------------------.
  ),

  [_mMou] = LAYOUT_universal(
 //,--------------------------------------------------------------------------.                                  ,--------------------------------------------------------------------------.
     XXX          , m_CLOSE      , XXX          , m_RELOAD     , m_NEW_TAB    ,                                    KC_BTN4      , KC_BTN5      , M_CLICK      , S_M_CLICK    , m_CLOSE      ,
 //,--------------+--------------+--------------+--------------+--------------.                                  ,--------------+--------------+--------------+--------------+--------------.
     m_ALL        , m_SAVE       , XXX          , m_FIND       , XXX          ,                                    APP_SWIPE    , L_CLICK      , R_CLICK      , MO(_Scr)     , _______      ,
 //,--------------+--------------+--------------+--------------+--------------.                                  ,--------------+--------------+--------------+--------------+--------------.
     m_UNDO       , m_CUT        , m_COPY       , m_PASTE      , XXX          ,                                    VOL_SWIPE    , BROWSE_SWIPE , TAB_SWIPE    , WIN_SWIPE    , XXX          ,
 //,--------------+--------------+--------------+--------------+--------------+--------------.    ,--------------+--------------+--------------+--------------+--------------+--------------.
     XXX          , m_QUIT       , XXX          , XXX          , KC_LSFT      , XXX          ,      KC_ESC       , DEEPL        , XXXXX        , XXXXXXX      , XXXXXXX      , XXX
 //,-----------------------------------------------------------------------------------------.    ,-----------------------------------------------------------------------------------------.
  ),

  [_wMou] = LAYOUT_universal(
 //,--------------------------------------------------------------------------.                                  ,--------------------------------------------------------------------------.
     XXX          , w_CLOSE      , XXX          , w_RELOAD     , w_NEW_TAB    ,                                    KC_BTN4      , KC_BTN5      , M_CLICK      , S_M_CLICK    , w_CLOSE      ,
 //,--------------+--------------+--------------+--------------+--------------.                                  ,--------------+--------------+--------------+--------------+--------------.
     w_ALL        , w_SAVE       , XXX          , w_FIND       , XXX          ,                                    APP_SWIPE    , L_CLICK      , R_CLICK      , MO(_Scr)     , _______      ,
 //,--------------+--------------+--------------+--------------+--------------.                                  ,--------------+--------------+--------------+--------------+--------------.
     w_UNDO       , w_CUT        , w_COPY       , w_PASTE      , XXX          ,                                    VOL_SWIPE    , BROWSE_SWIPE , TAB_SWIPE    , WIN_SWIPE    , XXX          ,
 //,--------------+--------------+--------------+--------------+--------------+--------------.    ,--------------+--------------+--------------+--------------+--------------+--------------.
     XXX          , w_QUIT       , XXX          , XXX          , KC_LSFT      , XXX          ,      KC_ESC       , DEEPL        , XXXXX        , XXXXXXX      , XXXXXXX      , XXX
 //,-----------------------------------------------------------------------------------------.    ,-----------------------------------------------------------------------------------------.
  ),

  [_Scr] = LAYOUT_universal(
 //,--------------------------------------------------------------------------.                                  ,--------------------------------------------------------------------------.
     RGB_TOG      , XXX          , RGB_HUI      , RGB_SAI      , RGB_VAI      ,                                    XXX          , KC_BTN4      , KC_BTN5      , XXX          , XXX          ,
 //,--------------+--------------+--------------+--------------+--------------.                                  ,--------------+--------------+--------------+--------------+--------------.
     XXX          , XXX          , RGB_HUD      , RGB_SAD      , RGB_VAD      ,                                    XXX          , KC_LSFT      , KC_LGUI      , _______      , XXX          ,
 //,--------------+--------------+--------------+--------------+--------------.                                  ,--------------+--------------+--------------+--------------+--------------.
     XXX          , XXX          , XXX          , XXX          , XXX          ,                                    XXX          , KC_LCTL      , KC_LALT      , XXX          , XXX          ,
 //,--------------+--------------+--------------+--------------+--------------+--------------.    ,--------------+--------------+--------------+--------------+--------------+--------------.
     XXX          , XXX          , XXX          , KBC_SAVE     , SCRL_DVI     , SCRL_DVD     ,      CPI_D100     , CPI_I100     , XXXXXXX      , XXXXXXX      , XXXXXXX      , _______
 //,-----------------------------------------------------------------------------------------.    ,-----------------------------------------------------------------------------------------.
  ),

};


// clang-format on


// layer state setting ------------------------------
layer_state_t layer_state_set_user(layer_state_t state) {

    // Auto enable scroll mode when the highest layer is 3
    keyball_set_scroll_mode(get_highest_layer(state) == 7);

    //LED------------------------------
    uint8_t layer = biton32(state);
    switch (layer) {
        case _Def:
            rgblight_sethsv(HSV_CYAN);
            break;
        case _NumP:
            rgblight_sethsv(HSV_WHITE);
            break;
        case _Sym:
            rgblight_sethsv(HSV_PURPLE);
            break;
        case _mCur:
            rgblight_sethsv(HSV_BLUE);
            break;
        case _wCur:
            rgblight_sethsv(HSV_BLUE);
            break;
        case _mMou:
            rgblight_sethsv(HSV_YELLOW);
            break;
        case _wMou:
            rgblight_sethsv(HSV_YELLOW);
            break;
        case _Scr:
            rgblight_sethsv(HSV_ORANGE);
            break;
    }

    return state;
}


// oled setting ------------------------------
#ifdef OLED_ENABLE
#    include "lib/oledkit/oledkit.h"

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

void oledkit_render_info_user(void) {
    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
    keyball_oled_render_layerinfo();
    oled_render_layer_state();
}

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

/* void oledkit_render_info_user(void) { */
/*     oled_render_layer_state(); */
/*     /1* keyball_oled_render_keyinfo(); *1/ */
/*     keyball_oled_render_ballinfo(); */
/*     /1* oled_write_P(PSTR("arrow_count: "),false); *1/ */
/*     /1* oled_write(format_4d(arrow_count), false); *1/ */
/*     /1* oled_render_repeat_speed(); *1/ */
/* } */
#endif

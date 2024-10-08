// keyball44
// mymap3.0
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
#include "features/layer.h"
#ifdef OLED_ENABLE
#include "lib/oledkit/oledkit.h"
#include "features/oled.h"
#endif

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  // keymap for default
  [_Def] = LAYOUT_universal(
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    KC_ESC    , KC_Q      , KC_W      , KC_E      , KC_R      , KC_T      ,                               KC_Y      , KC_U      , KC_I      , KC_O      , KC_P      , XXX       ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    KC_LCTL   , KC_A      , KC_S      , KC_D      , KC_F      , KC_G      ,                               KC_H      , KC_J      , KC_K      , KC_L      ,_Mou_SCLN  , XXX       ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    KC_LSFT   , KC_Z      , KC_X      , KC_C      , KC_V      , KC_B      ,                               KC_N      , KC_M      , GUI_T(KC_COMM),ALT_T(KC_DOT),SFT_T(KC_SLSH),XXX     ,
//,-----------+-----------+-----------+-----------+-----------+-----------+-----------.     ,-----------+-----------+-----------+-----------+-----------+-----------+-----------.
                            KC_LALT   , KC_LGUI   , MINUS_G   , EISU_S_N  , _Sym_SPC  ,       _Cur_ENT  , KANA_C    , XXXXXXX   , XXXXXXX   ,             _Esc_NumP
//,-----------------------------------------------------------------------------------.     ,-----------------------------------------------------------------------------------.
  ),

  [_Sym] = LAYOUT_universal(
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , S(KC_1)   , S(KC_2)   , S(KC_3)   , S(KC_4)   , S(KC_5)   ,                               S(KC_6)   , S(KC_7)   , S(KC_8)   , S(KC_9)   , TILDE     , XXX       ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , KC_1      , KC_2      , KC_3      , KC_4      , KC_5      ,                               KC_6      ,  KC_7     , KC_8      , KC_9      , KC_0      , XXX       ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , AT        , AST       , CARET     , V_BAR     , B_QUO     ,                               L_BRC     , R_BRC     , L_SQBR    , R_SQBR    , CLN       , XXX       ,
//,-----------+-----------+-----------+-----------+-------_----+-----------+----------.     ,-----------+-----------+-----------+-----------+-----------+-----------+-----------.
                            _______   , _______   , EQL       , XXX       , _______   ,       U_BAR     , YEN       , XXXXXXX   , XXXXXXX   ,             BSLSH
//,-----------------------------------------------------------------------------------.     ,-----------------------------------------------------------------------------------.
  ),

  [_mCur] = LAYOUT_universal(
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , M_F1      , M_F2      , M_F3      , M_F4      , M_F5      ,                               M_F6      , M_F7      , M_F8      , M_F9      , M_F10     , XXX       ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , XXX       , XXX       , XXX       , XXX       , XXX       ,                               KC_LEFT   , KC_DOWN   , KC_UP     , KC_RIGHT  , XXX       , XXX       ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , M_F11     , M_F12     , M_F13     , M_F14     , M_F15     ,                               m_HOME    , m_PGDN    , m_PGUP    , m_END     , XXX       , XXX       ,
//,-----------+-----------+-----------+-----------+-----------+-----------+-----------.     ,-----------+-----------+-----------+-----------+-----------+-----------+-----------.
                            _______   , _______   , XXX       , KC_LSFT   , XXX       ,       _______   , XXX       , XXXXXXX   , XXXXXXX   ,             XXX
//,-----------------------------------------------------------------------------------.     ,-----------------------------------------------------------------------------------.
  ),

  [_wCur] = LAYOUT_universal(
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , KC_F1     , KC_F2     , KC_F3     , KC_F4     , KC_F5     ,                               KC_F6     , KC_F7     , KC_F8     , KC_F9     , KC_F10    , XXX       ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , XXX       , XXX       , XXX       , XXX       , XXX       ,                               KC_LEFT   , KC_DOWN   , KC_UP     , KC_RIGHT  , XXX       , XXX       ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , KC_F11    , KC_F12    , KC_F13    , KC_F14    , KC_F15    ,                               KC_HOME   , KC_PGDN   , KC_PGUP   , KC_END    , XXX       , XXX       ,
//,-----------+-----------+-----------+-----------+-----------+-----------+-----------.     ,-----------+-----------+-----------+-----------+-----------+-----------+-----------.
                            _______   , _______   , XXX       , KC_LSFT   , XXX       ,       _______   , XXX       , XXXXXXX   , XXXXXXX   ,             BSLSH
//,-----------------------------------------------------------------------------------.     ,-----------------------------------------------------------------------------------.
  ),

  [_NumP] = LAYOUT_universal(
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , KC_BSPC   , KC_P7     , KC_P8     , KC_P9     , KC_PMNS   ,                               KC_PMNS   , KC_P7     , KC_P8     , KC_P9     , KC_PPLS   , XXX       ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , KC_ENT    , KC_P4     , KC_P5     , KC_P6     , KC_P0     ,                               KC_P0     , KC_P4     , KC_P5     , KC_P6     , MO(_mMou)  , XXX       ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , KC_TAB    , KC_P1     , KC_P2     , KC_P3     , KC_PDOT   ,                               KC_PDOT   , KC_P1     , KC_P2     , KC_P3     , KC_PSLS   , XXX       ,
//,-----------+-----------+-----------+-----------+-----------+-----------+-----------.     ,-----------+-----------+-----------+-----------+-----------+-----------+-----------.
                            _______   , _______   , _______   , KC_ESC    ,  KC_SPACE ,       KC_ENT    , KC_TAB    , XXXXXXX   , XXXXXXX   ,             TG(_NumP)
//,-----------------------------------------------------------------------------------.     ,-----------------------------------------------------------------------------------.
  ),

  [_mMou] = LAYOUT_universal(
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , XXX       , m_CLOSE   , KC_PGUP   , m_RELOAD  , m_NEW_TAB ,                               KC_BTN4   , KC_BTN5   , M_CLICK   , KC_LSFT   , XXX       , XXX       ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , m_ALL     , m_SAVE    , KC_PGDN   , m_FIND    , XXX       ,                               APP_SWIPE , L_CLICK   , R_CLICK   , MO(_Scr)  , _______   , XXX       ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , m_UNDO    , m_CUT     , m_COPY    , m_PASTE   , XXX       ,                               VOL_SWIPE ,BROWSE_SWIPE, TAB_SWIPE , WIN_SWIPE , XXX      , XXX       ,
//,-----------+-----------+-----------+-----------+-----------+-----------+-----------.     ,-----------+-----------+-----------+-----------+-----------+-----------+-----------.
                            _______   , _______   , XXX       , KC_LSFT   , XXX       ,       MULTI_A   , MULTI_B   , XXXXXXX   , XXXXXXX   ,             KC_PSCR
//,-----------------------------------------------------------------------------------.     ,-----------------------------------------------------------------------------------.
  ),

  [_wMou] = LAYOUT_universal(
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , XXX       , w_CLOSE   , KC_PGUP   , w_RELOAD  , w_NEW_TAB ,                               XXX       , XXX       , M_CLICK   , KC_LSFT   , XXX       , XXX       ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , w_ALL     , w_SAVE    , KC_PGDN   , w_FIND    , XXX       ,                               APP_SWIPE , L_CLICK   , R_CLICK   , MO(_Scr)  , _______   , XXX       ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , w_UNDO    , w_CUT     , w_COPY    , w_PASTE   , XXX       ,                               VOL_SWIPE ,BROWSE_SWIPE, TAB_SWIPE , WIN_SWIPE , XXX      , XXX       ,
//,-----------+-----------+-----------+-----------+-----------+-----------+-----------.     ,-----------+-----------+-----------+-----------+-----------+-----------+-----------.
                            _______   , _______   , XXX       , KC_LSFT   , XXX       ,       MULTI_A   , MULTI_B   , XXXXXXX   , XXXXXXX   ,             KC_PSCR
//,-----------------------------------------------------------------------------------.     ,-----------------------------------------------------------------------------------.
  ),

  [_Scr] = LAYOUT_universal(
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , RGB_HUI   , RGB_SAI   , RGB_VAI   , XXX       , XXX       ,                               XXX       , XXX       , XXX       , XXX       , XXX       , _______   ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , XXX       , RGB_TOG   , XXX       , XXX       , XXX       ,                               XXX       , XXX       , KC_BTN4   , _______   , _______   , _______   ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    _______   , RGB_HUD   , RGB_SAD   , RGB_VAD   , XXX       , XXX       ,                               XXX       , XXX       , XXX       , XXX       , XXX       , _______   ,
//,-----------+-----------+-----------+-----------+-----------+-----------+-----------.     ,-----------+-----------+-----------+-----------+-----------+-----------+-----------.
                            _______   , _______   , KBC_SAVE  , SCRL_DVI  , SCRL_DVD  ,       CPI_D100  , CPI_I100  , XXXXXXX   , XXXXXXX   ,             XXX
//,-----------------------------------------------------------------------------------.     ,-----------------------------------------------------------------------------------.
  ),

};

// clang-format on

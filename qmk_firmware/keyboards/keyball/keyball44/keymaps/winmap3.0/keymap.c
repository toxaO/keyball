// keyball44
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
#include "toxa_keycode.h"


// layer_names------------------------------
enum layer_names {
    _DEF = 0,
    _FUNC,
    _SYM,
    _CUR,
    _MOUSE,
    _ADJ,
    _DEL
};


// LT------------------------------
#define SYM_SPC LT(_SYM, KC_SPC)
#define MOU_SCLN LT(_MOUSE, KC_SCLN)
#define CUR_ENT LT(_CUR, KC_ENT)
#define CUR2_DOT LT(_DEL, KC_KP_DOT)
#define MOU_AST LT(_MOUSE, KC_PAST)
#define FUNC_BS LT(_FUNC, KC_BSPC)
#define DL_RSQB LT(_DEL, R_SQBR)
#define DL_0 LT(_DEL, KC_KP_0)


// カスタムキーコード------------------------------
enum custom_keycodes {
    SPD_UP = KEYBALL_SAFE_RANGE
};


// マクロ用変数------------------------------
// SPD_UP用
static bool spd_up = false;
static int arrow_count = 0;
enum arrow_held {
    NONE = 0,
    UP,
    RIGHT,
    LEFT,
    DOWN
};
enum arrow_held arrow_held = NONE;

// Ctrl+h用
static bool ctrl_held = false;
static bool ctrl_h_held = false;
static int ctrl_held_delay = 0;

#define M_REPEAT_TH 300

/*
//キーコンボ関連1------------------------------
//変更する際はconfig.hの記載も忘れないこと
enum combos {
    RL_CLICK, // middle click
    S_RL_CLICK
};

//キーコンボ関連2
const uint16_t PROGMEM rl_click_combo[] = {L_CLICK, R_CLICK, COMBO_END};
const uint16_t PROGMEM s_rl_click_combo[] = {KC_MPLY, KC_MFFD, COMBO_END};

//キーコンボ関連3
combo_t key_combos[COMBO_COUNT] = {
  [RL_CLICK] = COMBO(rl_click_combo, M_CLICK),
  [S_RL_CLICK] = COMBO(s_rl_click_combo, LSFT(M_CLICK))
};
*/

// タップダンス------------------------------
// 登録用関数
void sp_mis_app(tap_dance_state_t *state, void *user_data) {
    if(state->count == 1){
        SEND_STRING(SS_DOWN(X_LGUI)SS_TAP(X_SPACE)SS_UP(X_LGUI));
    }else if(state->count == 2){
        SEND_STRING(SS_DOWN(X_LCTL)SS_TAP(X_UP)SS_UP(X_LCTL));
    }else if(state->count == 3){
        SEND_STRING(SS_DOWN(X_LCTL)SS_TAP(X_DOWN)SS_UP(X_LCTL));
    }
}

enum {
  T_DESK = 0  // spotlight と mission control とapplication control
};

tap_dance_action_t tap_dance_actions[] = {
    [T_DESK] = ACTION_TAP_DANCE_FN(sp_mis_app)
};

//------------------------------

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  // keymap for default
  // デフォルトレイヤー
  [_DEF] = LAYOUT_universal(
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    ESC_A     , KC_Q      , KC_W      , KC_E      , KC_R      , KC_T      ,                               KC_Y      , KC_U      , KC_I      , KC_O      , KC_P      , AT_A      ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    TAB_C     , KC_A      , KC_S      , KC_D      , KC_F      , KC_G      ,                               KC_H      , KC_J      , KC_K      , KC_L      , MOU_SCLN  , CLN    ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    W_UNDER   , KC_Z      , KC_X      , KC_C      , KC_V      , KC_B      ,                               KC_N      , KC_M      , KC_COMM   , KC_DOT    , KC_SLSH   , MINUS_S   ,
//,-----------+-----------+-----------+-----------+-----------+-----------+-----------.     ,-----------+-----------+-----------+-----------+-----------+-----------+-----------.
                            KC_LGUI   , KC_DEL    , FUNC_BS   , EISU_S    , SYM_SPC   ,       CUR_ENT   , KANA_C    , XXXXXXX   , XXXXXXX   ,             TG(_FUNC)
//,-----------------------------------------------------------------------------------.     ,-----------------------------------------------------------------------------------.
  ),

  //ファンクションレイヤー
  [_FUNC] = LAYOUT_universal(
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    KC_LALT   , KC_F1     , KC_F2     , KC_F3     , KC_F4     , KC_F5     ,                               KC_MINUS  , KC_KP_7   , KC_KP_8   , KC_KP_9   , KC_BSPC   , KC_LALT   ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    KC_LCTL   , KC_F6     , KC_F7     , KC_F8     , KC_F9     , KC_F10    ,                               KC_KP_0   , KC_KP_4   , KC_KP_5   , KC_KP_6   , MO(_MOUSE), KC_LCTL   ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    KC_LSFT   , KC_F11    , KC_F12    , KC_F11    , KC_F12    , KC_F13    ,                               KC_DOT    , KC_KP_1   , KC_KP_2   , KC_KP_3   , KC_ENT    , KC_LSFT   ,
//,-----------+-----------+-----------+-----------+-------_----+-----------+----------.     ,-----------+-----------+-----------+-----------+-----------+-----------+-----------.
                            KC_LGUI   , _______   , _______   , KC_LSFT   , XXX       ,       KC_ENT    , KC_LCTL   , XXXXXXX   , XXXXXXX   ,             TG(_FUNC)
//,-----------------------------------------------------------------------------------.     ,-----------------------------------------------------------------------------------.
  ),

  //記号レイヤー
  [_SYM] = LAYOUT_universal(
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    KC_LALT   , LSFT(KC_1), LSFT(KC_2), LSFT(KC_3), LSFT(KC_4), LSFT(KC_5),                               LSFT(KC_6), LSFT(KC_7), L_PAREN   , R_PAREN   , CARET     , KC_LALT   ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    KC_LCTL   , KC_1      , KC_2      , KC_3      , KC_4      , KC_5      ,                               KC_6      , KC_7      , KC_8      , KC_9      , KC_0      , KC_LCTL   ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    KC_LSFT   , KC_LGUI   , L_BRC     , R_BRC     , L_SQBR    , R_SQBR    ,                               XXX       , XXX       , LESS      , MORE      , QUESTION  , EQL       ,
//,-----------+-----------+-----------+-----------+-----------+-----------+-----------.     ,-----------+-----------+-----------+-----------+-----------+-----------+-----------.
                            _______   , _______   , KC_DEL    , XXX       , _______   ,       TILDE     , PIPE      , XXXXXXX   , XXXXXXX   ,             KC_INT1
//,-----------------------------------------------------------------------------------.     ,-----------------------------------------------------------------------------------.
  ),

  //カーソルレイヤー
  [_CUR] = LAYOUT_universal(
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    KC_LALT   , XXX       , XXX       , XXX       , XXX       , XXX       ,                               HOME      , KC_PGDN   , KC_PGUP   , END       , XXX       , KC_LALT   ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    KC_TAB    , XXX       , XXX       , XXX       , XXX       , XXX       ,                               KC_LEFT   , KC_DOWN   , KC_UP     , KC_RIGHT  , XXX       , KC_LCTL   ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    KC_LSFT   , KC_LGUI   , XXX       , XXX       , XXX       , XXX       ,                               C(KC_LEFT), LESS      , MORE      ,C(KC_RIGHT), KC_LGUI   , KC_LSFT   ,
//,-----------+-----------+-----------+-----------+-----------+-----------+-----------.     ,-----------+-----------+-----------+-----------+-----------+-----------+-----------.
                            _______   , XXX       , XXX       , XXX       , SPD_UP    ,       _______   , XXX       , XXXXXXX   , XXXXXXX   ,             XXX
//,-----------------------------------------------------------------------------------.     ,-----------------------------------------------------------------------------------.
  ),

  //マウスレイヤー
  [_MOUSE] = LAYOUT_universal(
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    KC_LALT   , XXX       , C(KC_W)   , KC_PGUP   , C(KC_R)   , C(KC_T)   ,                               TAB_L     , TAB_R     , M_CLICK   , KC_LSFT   , KBC_SAVE  , C(S(KC_T)),
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    KC_LCTL   , C(KC_A)   , C(KC_S)   , KC_PGDN   , C(KC_F)   , C(S(KC_T)),                               G(KC_TAB) , L_CLICK   , R_CLICK   , MO(_ADJ)  , _______   , C(KC_T)   ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    KC_LSFT   , C(KC_Z)   , C(KC_X)   , C(KC_C)   , C(KC_V)   , C(KC_Y)   ,                               KC_ENT    , KC_WBAK   , KC_WFWD   , KC_ESC    , KBC_RST   , C(KC_W)   ,
//,-----------+-----------+-----------+-----------+-----------+-----------+-----------.     ,-----------+-----------+-----------+-----------+-----------+-----------+-----------.
                            KC_LGUI   , KC_PSCR   , KC_DEL    , KC_LSFT   , XXX       ,       W_DESK_L  , W_DESK_R  , XXXXXXX   , XXXXXXX   ,             KC_PSCR
//,-----------------------------------------------------------------------------------.     ,-----------------------------------------------------------------------------------.
  ),

  //アジャストレイヤー
  [_ADJ] = LAYOUT_universal(
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    RGB_MOD   , XXX       , XXX       , SCRL_DVD  , CPI_I100  , CPI_I1K   ,                               C(KC_C)   , KC_VOLU   , KC_MUTE   , XXX       , XXX       , _______   ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    RGB_RMOD  , XXX       , XXX       , SCRL_DVI  , CPI_D100  , CPI_D1K   ,                               KC_MRWD   , KC_MPLY   , KC_MFFD   , _______   , _______   , _______   ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    RGB_M_B   , XXX       , XXX       , XXX       , XXX       , XXX       ,                               C(KC_V)   , KC_VOLD   , XXX       , XXX       , XXX       , _______   ,
//,-----------+-----------+-----------+-----------+-----------+-----------+-----------.     ,-----------+-----------+-----------+-----------+-----------+-----------+-----------.
                            XXX       , XXX       , XXX       , XXX       , XXX       ,       C(KC_C)   , C(KC_V)   , XXXXXXX   , XXXXXXX   ,             XXX
//,-----------------------------------------------------------------------------------.     ,-----------------------------------------------------------------------------------.
  ),

  //カーソル2
  [_DEL] = LAYOUT_universal(
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    XXX       , XXX       , XXX       , XXX       , XXX       , XXX       ,                               XXX       , XXX       , XXX       , XXX       , XXX       , _______   ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    XXX       , XXX       , XXX       , XXX       , XXX       , XXX       ,                               KC_BSPC   , KC_DEL    , XXX       , XXX       , _______   , _______   ,
//,-----------------------------------------------------------------------.                             ,-----------------------------------------------------------------------.
    XXX       , XXX       , XXX       , XXX       , XXX       , XXX       ,                               XXX       , XXX       , XXX       , XXX       , XXX       , _______   ,
//,-----------+-----------+-----------+-----------+-----------+-----------+-----------.     ,-----------+-----------+-----------+-----------+-----------+-----------+-----------.
                            XXX       , XXX       , XXX       , XXX       , XXX       ,       XXX       , _______   , XXXXXXX   , XXXXXXX   ,             XXX
//,-----------------------------------------------------------------------------------.     ,-----------------------------------------------------------------------------------.
  ),

};


// clang-format on


// layer state setting ------------------------------
layer_state_t layer_state_set_user(layer_state_t state) {

    // Auto enable scroll mode when the highest layer is 3
    keyball_set_scroll_mode(get_highest_layer(state) == 5);

    //LED------------------------------
    uint8_t layer = biton32(state);
    switch (layer) {
        case _DEF:
            rgblight_sethsv(HSV_CYAN);
            break;
        case _SYM:
            rgblight_sethsv(HSV_BLUE);
            break;
        case _CUR:
            rgblight_sethsv(HSV_CHARTREUSE);
            break;
        case _MOUSE:
            rgblight_sethsv(HSV_PURPLE);
            break;
        case _ADJ:
            rgblight_sethsv(HSV_GOLDENROD);
            break;
        case _FUNC:
            rgblight_sethsv(HSV_CORAL);
            break;
        case _DEL:
            rgblight_sethsv(HSV_MAGENTA);
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
        case _DEF:
            oled_write_ln_P(PSTR("Default   "), false);
            break;
        case _SYM:
            oled_write_ln_P(PSTR("Symbol    "), false);
            break;
        case _CUR:
            oled_write_ln_P(PSTR("Num/Cursor"), false);
            break;
        case _MOUSE:
            oled_write_ln_P(PSTR("Mouse     "), false);
            break;
        case _ADJ:
            oled_write_ln_P(PSTR("Adjust    "), false);
            break;
        case _FUNC:
            oled_write_ln_P(PSTR("Func/Num  "), false);
            break;
        case _DEL:
            oled_write_ln_P(PSTR("Cursor2   "), false);
            break;
    }
}
/*
static const char *format_4d(int d) {
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
*/
void oledkit_render_info_user(void) {
    oled_render_layer_state();
//    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
//    oled_write_P(PSTR("arrow_count: "),false);
//    oled_write(format_4d(arrow_count), false);
}
#endif


// my keycode process setting------------------------------
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch(keycode){

            // windowの変更関連
            case CHAPP_AC:
                if (record->tap.count && record->event.pressed) {
                    register_code(KC_LCTL);
                    register_code(KC_RCTL);
                    register_code(KC_F4);
                    unregister_code(KC_F4);
                    unregister_code(KC_RCTL);
                    unregister_code(KC_LCTL);
                    return false;        // Return false to ignore further processing of key
                }
                break;

            // app windowの変更
            case CHWIN_Fn:
                if (record->tap.count && record->event.pressed) {
                    register_code(KC_LGUI);
                    register_code(AT);
                    unregister_code(AT);
                    unregister_code(KC_LGUI);
                }
                break;

            case SPD_UP:
                if (record->event.pressed) {
                    spd_up = true;
                } else {
                    spd_up = false;
                    switch(arrow_held) {
                        case RIGHT:
                            register_code(KC_RIGHT);
                        break;
                        case LEFT:
                            register_code(KC_LEFT);
                        break;
                        case UP:
                            register_code(KC_UP);
                        break;
                        case DOWN:
                            register_code(KC_DOWN);
                        break;
                        case NONE:
                        break;
                    }
                }
                break;
            case KC_RIGHT:
                if (record->event.pressed){
                    arrow_held = RIGHT;
                    arrow_count++;
                        return true;
                } else {
                    arrow_count--;
                    unregister_code(KC_RIGHT);
                    if (arrow_count == 0){
                        arrow_held = NONE;
                    }
                }
                break;
            case KC_LEFT:
                if (record->event.pressed){
                    arrow_held = LEFT;
                    arrow_count++;
                        return true;
                } else {
                    arrow_count--;
                    unregister_code(KC_LEFT);
                    if (arrow_count == 0){
                        arrow_held = NONE;
                    }
                }
                break;
            case KC_UP:
                if (record->event.pressed){
                    arrow_held = UP;
                    arrow_count++;
                        return true;
                } else {
                    arrow_count--;
                    unregister_code(KC_UP);
                    if (arrow_count == 0){
                        arrow_held = NONE;
                    }
                }
                break;
            case KC_DOWN:
                if (record->event.pressed){
                    arrow_held = DOWN;
                    arrow_count++;
                        return true;
                } else {
                    arrow_count--;
                    unregister_code(KC_DOWN);
                    if (arrow_count == 0){
                        arrow_held = NONE;
                    }
                }
                break;
        // ctrl+h backspace------------------------------
            case KC_LCTL:
            case MOD_LCTL:
            case KANA_C:
            case TAB_C:
            case CLN_C:
                if (record->event.pressed){
                    ctrl_held = true;
                }else{
                    ctrl_held = false;
                    ctrl_h_held = false;
                }
                return true;
            case KC_H:
                if (record->event.pressed && ctrl_held){
                    ctrl_h_held = true;
                    return false;
                }else{
                    ctrl_h_held = false;
                    ctrl_held_delay = 0;
                    return true;
                }

        /*
        // MTで対応しきれないキーコードの対処法
        // docに記載あり
            case LSFT_T(DESK_R):
            if (record->tap.count && record->event.pressed) {
                tap_code16(DESK_R); // Send KC_DQUO on tap
                return false;        // Return false to ignore further processing of key
            }
            break;
        */

    }
    return true;
}

//-------------------------------------------------------------
//  matrix_scan_user関数の中でフラグをチェックして必要なマクロを実行する。
//  この関数はQMKががキーの押下をチェックするたび実行される。
//  https://gist.github.com/ypsilon-takai/c4a249087ea19eabd1deb3f9f273de52
//

// Runs constantly in the background, in a loop.
void matrix_scan_user(void) {

    if (spd_up) {
        switch(arrow_held) {
            case RIGHT:
                SEND_STRING(SS_TAP(X_RIGHT)SS_TAP(X_RIGHT)SS_DELAY(20));
            break;
            case LEFT:
                SEND_STRING(SS_TAP(X_LEFT)SS_TAP(X_LEFT)SS_DELAY(20));
            break;
            case UP:
                SEND_STRING(SS_TAP(X_UP)SS_TAP(X_UP)SS_DELAY(20));
            break;
            case DOWN:
                SEND_STRING(SS_TAP(X_DOWN)SS_TAP(X_DOWN)SS_DELAY(20));
            break;
            case NONE:
            break;
        }
    }

    if (ctrl_h_held) {
        if (ctrl_held_delay == 0) {
            unregister_code(KC_LCTL);
            register_code(KC_BSPC);
            unregister_code(KC_BSPC);
            register_code(KC_LCTL); // もし左Ctrlキーが元々押されていた場合、再度登録
        }
        if (ctrl_held_delay > M_REPEAT_TH) {
            unregister_code(KC_LCTL);
            SEND_STRING(SS_TAP(X_BSPC)SS_DELAY(20));
            register_code(KC_LCTL);
        }
        ctrl_held_delay++;
    }

};

/* Copyright 2023 kamidai (@d_kamiichi)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// マクロ用変数------------------------------
// SPD_UP用
/* static bool spd_up = false; */
/* static int arrow_count = 0; */
/* enum arrow_held { */
/*     NO_DIR = 0, */
/*     UP, */
/*     RIGHT, */
/*     LEFT, */
/*     DOWN */
/* }; */
/* enum arrow_held arrow_held = NO_DIR; */

// mod override用変数
uint8_t mod_state;
static bool bspckey_registered = false;
static bool delkey_registered = false;
static bool tabkey_registered = false;


// マクロキーを設定
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    current_keycode = keycode;
    mod_state = get_mods();

    switch (keycode) {

        case KC_H:
            if (record->event.pressed) {
                if (mod_state & MOD_MASK_CTRL) {
                    if (mod_state & MOD_MASK_SHIFT) {
                        unregister_code(KC_LCTL);
                        unregister_code(KC_LSFT);
                        register_code(KC_DEL);
                        delkey_registered = true;
                    return false;
                    } else {
                        unregister_code(KC_LCTL);
                        register_code(KC_BSPC);
                        bspckey_registered = true;
                        return false;
                    }
                }
            } else {
                if (bspckey_registered) {
                    unregister_code(KC_BSPC);
                    register_code(KC_LCTL);
                    bspckey_registered = false;
                    return false;
                }
                if (delkey_registered) {
                    unregister_code(KC_DEL);
                    register_code(KC_LCTL);
                    register_code(KC_LSFT);
                    delkey_registered = false;
                    return false;
                }
            }
            return true;

        case KC_I:
            if (record->event.pressed) {
                if (mod_state & MOD_MASK_CTRL) {
                    unregister_code(KC_LCTL);
                    register_code(KC_TAB);
                    tabkey_registered = true;
                    return false;
                }
            } else { // on release of KC_BSPC
                if (tabkey_registered) {
                    unregister_code(KC_TAB);
                    register_code(KC_LCTL);
                    tabkey_registered = false;
                    return false;
                }
            }
            return true;

        // macro key
        // SFT + M_CLICK
        case S_M_CLICK:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code_delay(KC_NO, 3); // delayを入れないとshiftが後に入力される
                register_code(KC_BTN3);
                return false;        // Return false to ignore further processing of key
            } else {
                unregister_code(KC_BTN3);
                unregister_code(KC_LSFT);
                return false;        // Return false to ignore further processing of key
            }
            break;

        // ->
        case R_ARROW:
            if (record->event.pressed) {
                tap_code(KC_MINUS);
                tap_code16(S(KC_DOT));
                return false;        // Return false to ignore further processing of key
            }
            break;

        // =>
        case R_D_ARR:
            if (record->event.pressed) {
                tap_code16(S(KC_MINUS));
                tap_code16(S(KC_DOT));
                return false;        // Return false to ignore further processing of key
            }
            break;

        case DEEPL:
            if (record->event.pressed) {
                tap_code16(G(KC_C));
                tap_code16(G(KC_C));
                return false;
            }
            break;

        // 単押しでesc、長押しでNumP
        case _Esc_NumP:
            if (record->event.pressed && !record->tap.count) {
                layer_on(_NumP);
                return false;
            } else if (record->event.pressed) {
                tap_code(KC_ESC);
                return false;
            }
            break;


        // 以下スワイプジェスチャー
        // クリックすると state が SWIPE になり、離したら NONE になる
        // SWIPEの実装はswipe.hに記載する
        case APP_SWIPE:
            if (record->event.pressed) {
                // キーダウン時
                swipe_timer = timer_read();
                is_swiped = false;
                canceller = false;
                state = SWIPE;
            } else {
                // キーアップ時
                // mission control
                state = NONE;
                canceller = false;
                rgblight_sethsv(HSV_YELLOW);
                if (is_swiped == false && timer_elapsed(swipe_timer) < TAPPING_TERM){
                    tap_code16(C(KC_UP));
                }
                repeat_speed = NORMAL;
            }
            break;

        case VOL_SWIPE:
            if (record->event.pressed) {
                // キーダウン時
                swipe_timer = timer_read();
                is_swiped = false;
                state = SWIPE;
            } else {
                // キーアップ時
                state = NONE;
                rgblight_sethsv(HSV_YELLOW);
                if (is_swiped == false && timer_elapsed(swipe_timer) < TAPPING_TERM){
                    tap_code(KC_MPLY);
                }
                repeat_speed = NORMAL;
            }
            break;

        case BROWSE_SWIPE:
            if (record->event.pressed) {
                // キーダウン時
                swipe_timer = timer_read();
                is_swiped = false;
                state = SWIPE;
            } else {
                // キーアップ時
                state = NONE;
                rgblight_sethsv(HSV_YELLOW);
                if (is_swiped == false && timer_elapsed(swipe_timer) < TAPPING_TERM){
                    tap_code16(G(KC_L));
                }
                repeat_speed = NORMAL;
            }
            break;

        case TAB_SWIPE:
            if (record->event.pressed) {
                // キーダウン時
                swipe_timer = timer_read();
                is_swiped = false;
                state = SWIPE;
            } else {
                // キーアップ時
                state = NONE;
                rgblight_sethsv(HSV_YELLOW);
                if (is_swiped == false && timer_elapsed(swipe_timer) < TAPPING_TERM){
                   tap_code16(G(KC_T));
                }
                repeat_speed = NORMAL;
            }
            break;

        case MAG_SWIPE:
            if (record->event.pressed) {
                // キーダウン時
                swipe_timer = timer_read();
                is_swiped = false;
                state = SWIPE;
            } else {
                // キーアップ時
                state = NONE;
                rgblight_sethsv(HSV_YELLOW);
                if (is_swiped == false && timer_elapsed(swipe_timer) < TAPPING_TERM){
                    tap_code16(A(C(KC_ENT)));
                }
                repeat_speed = NORMAL;
            }
            break;

        /* // <- */
        /* case L_ARROW: */
        /*     if (record->event.pressed) { */
        /*         register_code(KC_LSFT); */
        /*         register_code(KC_COMM); */
        /*         unregister_code(KC_COMM); */
        /*         unregister_code(KC_LSFT); */
        /*         register_code(KC_MINUS); */
        /*         unregister_code(KC_MINUS); */
        /*         return false;        // Return false to ignore further processing of key */
        /*     } */
        /*     break; */

        /* // <= */
        /* case L_D_ARR: */
        /*     if (record->event.pressed) { */
        /*         register_code(KC_LSFT); */
        /*         register_code(KC_COMM); */
        /*         unregister_code(KC_COMM); */
        /*         register_code(KC_MINUS); */
        /*         unregister_code(KC_MINUS); */
        /*         unregister_code(KC_LSFT); */
        /*         return false;        // Return false to ignore further processing of key */
        /*     } */
        /*     break; */

        /*     // _Mou_NumP */
        /* case LT(_Mou, KC_NO): */
        /*     if (record->tap.count && record->event.pressed){ */
        /*         layer_invert(_NumP); */
        /*         return false; */
        /*     } */
        /*     return true; */

        /* case SPD_UP: */
        /*     if (record->event.pressed) { */
        /*         spd_up = true; */
        /*     } else { */
        /*         spd_up = false; */
        /*         switch(arrow_held) { */
        /*             case RIGHT: */
        /*                 register_code(KC_RIGHT); */
        /*             break; */
        /*             case LEFT: */
        /*                 register_code(KC_LEFT); */
        /*             break; */
        /*             case UP: */
        /*                 register_code(KC_UP); */
        /*             break; */
        /*             case DOWN: */
        /*                 register_code(KC_DOWN); */
        /*             break; */
        /*             case NO_DIR: */
        /*             break; */
        /*         } */
        /*     } */
        /*     break; */
        /* case KC_RIGHT: */
        /*     if (record->event.pressed){ */
        /*         arrow_held = RIGHT; */
        /*         arrow_count++; */
        /*             return true; */
        /*     } else { */
        /*         arrow_count--; */
        /*         unregister_code(KC_RIGHT); */
        /*         if (arrow_count == 0){ */
        /*             arrow_held = NO_DIR; */
        /*         } */
        /*     } */
        /*     break; */
        /* case KC_LEFT: */
        /*     if (record->event.pressed){ */
        /*         arrow_held = LEFT; */
        /*         arrow_count++; */
        /*             return true; */
        /*     } else { */
        /*         arrow_count--; */
        /*         unregister_code(KC_LEFT); */
        /*         if (arrow_count == 0){ */
        /*             arrow_held = NO_DIR; */
        /*         } */
        /*     } */
        /*     break; */
        /* case KC_UP: */
        /*     if (record->event.pressed){ */
        /*         arrow_held = UP; */
        /*         arrow_count++; */
        /*             return true; */
        /*     } else { */
        /*         arrow_count--; */
        /*         unregister_code(KC_UP); */
        /*         if (arrow_count == 0){ */
        /*             arrow_held = NO_DIR; */
        /*         } */
        /*     } */
        /*     break; */
        /* case KC_DOWN: */
        /*     if (record->event.pressed){ */
        /*         arrow_held = DOWN; */
        /*         arrow_count++; */
        /*             return true; */
        /*     } else { */
        /*         arrow_count--; */
        /*         unregister_code(KC_DOWN); */
        /*         if (arrow_count == 0){ */
        /*             arrow_held = NO_DIR; */
        /*         } */
        /*     } */
        /*     break; */

    }
    return true;
}


//-------------------------------------------------------------
//  matrix_scan_user関数の中でフラグをチェックして必要なマクロを実行する。
//  この関数はQMKががキーの押下をチェックするたび実行される。
//  https://gist.github.com/ypsilon-takai/c4a249087ea19eabd1deb3f9f273de52
//

// Runs constantly in the background, in a loop.
/* void matrix_scan_user(void) { */

/*     if (spd_up) { */
/*         switch(arrow_held) { */
/*             case RIGHT: */
/*                 SEND_STRING(SS_TAP(X_RIGHT)SS_TAP(X_RIGHT)SS_DELAY(20)); */
/*             break; */
/*             case LEFT: */
/*                 SEND_STRING(SS_TAP(X_LEFT)SS_TAP(X_LEFT)SS_DELAY(20)); */
/*             break; */
/*             case UP: */
/*                 SEND_STRING(SS_TAP(X_UP)SS_TAP(X_UP)SS_DELAY(20)); */
/*             break; */
/*             case DOWN: */
/*                 SEND_STRING(SS_TAP(X_DOWN)SS_TAP(X_DOWN)SS_DELAY(20)); */
/*             break; */
/*             case NO_DIR: */
/*             break; */
/*         } */
/*     } */

/* }; */

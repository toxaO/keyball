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


#include "os_detection.h"
#include "my_keycode.h"

// mod override用変数
uint8_t mod_state;
static bool bspckey_registered = false;
static bool delkey_registered = false;
static bool tabkey_registered = false;
static bool kana_c_pressed = false;
static bool l_ctrl_pressed = false;
static bool sft_pressed = false;

static uint16_t first_ctrl_tap_time = 0;
static uint16_t first_shift_tap_time = 0;

// マクロキーを設定
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    /* swipe_mode = keycode; */
    mod_state = get_mods();

    switch (keycode) {
        case _Cur_ENT:
            if (!record->tap.count && record->event.pressed) {
                if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
                    layer_on(_mCur);
                } else {
                    layer_on(_wCur);
                }
                return false;
            } else {
              layer_off(_mCur);
              layer_off(_wCur);
            }
            return true;

        case _Mou_SCLN:
            if (!record->tap.count && record->event.pressed) {
                if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
                    layer_on(_mMou);
                } else {
                    layer_on(_wMou);
                }
                return false;
            } else {
              layer_off(_mMou);
              layer_off(_wMou);
            }
            return true;

        case MO(_mMou):
            if (record->event.pressed) {
                if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
                    layer_on(_mMou);
                } else {
                    layer_on(_wMou);
                }
            } else {
              layer_off(_mMou);
              layer_off(_wMou);
            }
            return false;

        case KC_H:
            if (record->event.pressed) {
              // KANA_CとLCTLが両方押下されていたら+ctrl
              if (kana_c_pressed && l_ctrl_pressed) {
                if (mod_state & MOD_MASK_SHIFT) {
                  // +SFT
                  unregister_code(KC_LSFT);
                  register_code(KC_DEL);
                  delkey_registered = true;
                  return false;
                } else {
                  // SFTなし
                  register_code(KC_BSPC);
                  bspckey_registered = true;
                  return false;
                }
              }

              if (kana_c_pressed || l_ctrl_pressed) {
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
                    unregister_code(KC_LCTL);
                    bspckey_registered = false;
                }
                if (delkey_registered) {
                    unregister_code(KC_DEL);
                    unregister_code(KC_LCTL);
                    unregister_code(KC_LSFT);
                    delkey_registered = false;
                }
                if (kana_c_pressed || l_ctrl_pressed) {
                    register_code(KC_LCTL);
                }
                if (sft_pressed) {
                    register_code(KC_LSFT);
                }
            }
            return true;

        case KC_I:
            if (record->event.pressed) {
                if (l_ctrl_pressed && kana_c_pressed) {
                  register_code(KC_TAB);
                  tabkey_registered = true;
                  return false;
                }
                if (l_ctrl_pressed || kana_c_pressed) {
                  unregister_code(KC_LCTL);
                  register_code(KC_TAB);
                  tabkey_registered = true;
                  return false;
                }
            } else { // on release of KC_BSPC
                if (tabkey_registered) {
                    unregister_code(KC_TAB);
                    unregister_code(KC_LCTL);
                    tabkey_registered = false;
                    if (kana_c_pressed || l_ctrl_pressed) {
                        register_code(KC_LCTL);
                    }
                }
            }
            return true;

        case KC_LCTL:
        case CTL_T(KC_ESC):
            if (record->event.pressed) {
              l_ctrl_pressed = true;
            } else {
              l_ctrl_pressed = false;
            }
            return true;

        case KANA_C:
            if (record->event.pressed) {
              kana_c_pressed = true;
            } else {
              kana_c_pressed = false;
            }
            return true;

        case KC_LSFT:
        case EISU_S:
            if (record->event.pressed) {
              sft_pressed = true;
            } else {
              sft_pressed = false;
            }
            return true;

        case EISU_S_N:
            if (record->event.pressed) {
              if (TIMER_DIFF_16(record->event.time, first_shift_tap_time) < 500) {
                layer_on(_NumP);
              } else {
                register_code(KC_LSFT);
                sft_pressed = true;
                first_shift_tap_time = record->event.time;
              }
            } else {
              unregister_code(KC_LSFT);
              sft_pressed = false;
              if (TIMER_DIFF_16(record->event.time, first_shift_tap_time) < TAPPING_TERM) {
                tap_code(KC_LNG2);
              }
              layer_off(_NumP);
            }
            return false;

        case KANA_C_N:
            if (record->event.pressed) {
              if (TIMER_DIFF_16(record->event.time, first_ctrl_tap_time) < 500) {
                tap_code(KC_LNG2);
                layer_on(_NumP);
              } else {
                register_code(KC_LCTL);
                kana_c_pressed = true;
                first_ctrl_tap_time = record->event.time;
              }
            } else {
              unregister_code(KC_LCTL);
              kana_c_pressed = false;
              if (TIMER_DIFF_16(record->event.time, first_ctrl_tap_time) < TAPPING_TERM) {
                tap_code(KC_LNG1);
              }
              layer_off(_NumP);
            }
            return false;

        // 以下スワイプジェスチャー
        // クリックすると state が SWIPE になり、離したら NONE になる
        // SWIPEの実装はswipe.hに記載する
        case APP_SWIPE:
            if (record->event.pressed) {
                // キーダウン時
              swipe_mode = APP_SW;
              swipe_timer = timer_read();
              is_swiped = false;
              canceller = false;
              state = SWIPE;
            } else {
                // キーアップ時
              swipe_mode = NO_SW;
              state = NONE;
              canceller = false;
              if (is_swiped == false && timer_elapsed(swipe_timer) < TAPPING_TERM){
                tap_code16_os(G(KC_TAB), m_MIS_CON, m_MIS_CON, KC_NO, KC_NO);
                /* if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){ */
                /*   // mission control */
                /*   tap_code16(m_MIS_CON); */
                /* } else { */
                /*   // task control */
                /*   tap_code16(G(KC_TAB)); */
                /* } */
              }
              repeat_speed = NORMAL;
            }
            break;

        case VOL_SWIPE:
            if (record->event.pressed) {
                // キーダウン時
                swipe_mode = VOL_SW;
                swipe_timer = timer_read();
                is_swiped = false;
                state = SWIPE;
            } else {
                // キーアップ時
                swipe_mode = NO_SW;
                state = NONE;
                if (is_swiped == false && timer_elapsed(swipe_timer) < TAPPING_TERM){
                    tap_code(KC_MPLY);
                }
                repeat_speed = NORMAL;
            }
            break;

        case BROWSE_SWIPE:
            if (record->event.pressed) {
                // キーダウン時
                swipe_mode = BRO_SW;
                swipe_timer = timer_read();
                is_swiped = false;
                state = SWIPE;
            } else {
                // キーアップ時
                swipe_mode = NO_SW;
                state = NONE;
                if (is_swiped == false && timer_elapsed(swipe_timer) < TAPPING_TERM){
                  tap_code16_os(C(KC_L), G(KC_L), G(KC_L), KC_NO, KC_NO);
                  /* if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){ */
                  /*   tap_code16(G(KC_L)); */
                  /* } else { */
                  /*   tap_code16(C(KC_L)); */
                  /* } */
                }
                repeat_speed = NORMAL;
            }
            break;

        case TAB_SWIPE:
            if (record->event.pressed) {
                // キーダウン時
                swipe_mode = TAB_SW;
                swipe_timer = timer_read();
                is_swiped = false;
                state = SWIPE;
            } else {
                // キーアップ時
                swipe_mode = NO_SW;
                state = NONE;
                if (is_swiped == false && timer_elapsed(swipe_timer) < TAPPING_TERM){
                  tap_code16_os(C(KC_T), G(KC_T), G(KC_T), KC_NO, KC_NO);
                  /* if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){ */
                  /*   tap_code16(G(KC_T)); */
                  /* } else { */
                  /*   tap_code16(C(KC_T)); */
                  /* } */
                }
                repeat_speed = NORMAL;
            }
            break;

        case WIN_SWIPE:
            if (record->event.pressed) {
                // キーダウン時
                swipe_mode = WIN_SW;
                swipe_timer = timer_read();
                is_swiped = false;
                state = SWIPE;
            } else {
                // キーアップ時
                swipe_mode = NO_SW;
                state = NONE;
                if (is_swiped == false && timer_elapsed(swipe_timer) < TAPPING_TERM){
                  tap_code16_os(G(KC_Z), A(C(KC_ENT)), A(C(KC_ENT)), KC_NO, KC_NO);
                  /* if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){ */
                  /*   tap_code16(A(C(KC_ENT))); */
                  /* } else { */
                  /*   tap_code16(G(KC_Z)); */
                  /* } */
                  if (host_os == OS_WINDOWS){
                    unregister_code(KC_LGUI);
                  }
                }
                repeat_speed = NORMAL;
            }
            break;

        case MULTI_A:
            if (record->event.pressed) {
              switch(swipe_mode) {
                case APP_SW:
                  if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
                    tap_code16(m_L_DESK); // spotlight
                  } else {
                    tap_code16(w_L_DESK); // windows key
                  }
                  break;

                case VOL_SW:
                  break;

                case TAB_SW:
                  tap_code16(S(C(KC_TAB))); // next tab
                  break;

                case BRO_SW:
                  if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
                    tap_code16(G(KC_LEFT)); // browse back
                  } else {
                    tap_code16(KC_WBAK); // windows key
                  }
                  break;

                case WIN_SW:
                  break;

                default:
                if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
                  tap_code16(G(KC_Z));
                } else {
                  tap_code16(C(KC_Z));
                }
                  break;
              }
            } else {
            }
            break;

        case MULTI_B:
            if (record->event.pressed) {
              switch(swipe_mode) {
                case APP_SW:
                  if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
                    tap_code16(m_R_DESK); // spotlight
                  } else {
                    tap_code16(w_R_DESK); // windows key
                  }
                  break;

                case VOL_SW:
                  break;

                case TAB_SW:
                  tap_code16(C(KC_TAB)); // previous tab
                  break;

                case BRO_SW:
                  if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
                    tap_code16(G(KC_RIGHT)); // browse back
                  } else {
                    tap_code16(KC_WFWD); // windows key
                  }
                  break;

                case WIN_SW:
                  break;

                default:
                if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
                  tap_code16(S(G(KC_Z)));
                } else {
                  tap_code16(S(C(KC_Z)));
                }
                  break;
              }
            } else {
            }
            break;

    }
    return true;
}

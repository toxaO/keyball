#include "quantum.h"
// #include "swipe.h"
#include "swipe_user.h"
#include "os_detection.h"
#include "my_keycode.h"
#include "util.h"
#include QMK_KEYBOARD_H
#include "eeconfig.h"

#include "lib/keyball/keyball.h"

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

uint16_t swipe_timer; // スワイプキーがTAPPING_TERMにあるかを判定する (≒ mod_tap)
bool canceller = false;

// マクロキーを設定
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    /* swipe_mode = keycode; */
    mod_state = get_mods();

    switch (keycode) {
        case _Cur_ENT:
            if (!record->tap.count && record->event.pressed) {
              switch (host_os) {
                case OS_MACOS:
                case OS_IOS:
                  layer_on(_mCur);
                  break;

                case OS_WINDOWS:
                  layer_on(_wCur);
                  break;

              }
                return false;

            } else {
              layer_off(_mCur);
              layer_off(_wCur);

            }
            return true;

        case _Mou_SCLN:
            if (!record->tap.count && record->event.pressed) {
              switch (host_os) {
                case OS_MACOS:
                case OS_IOS:
                  layer_on(_mMou);
                  break;

                case OS_WINDOWS:
                  layer_on(_wMou);
                  break;

              }
                return false;
            } else {
              layer_off(_mMou);
              layer_off(_wMou);

            }
            return true;

        case MO(_mMou):
            if (record->event.pressed) {
              switch (host_os) {
                case OS_MACOS:
                case OS_IOS:
                  layer_on(_mMou);
                  break;

                case OS_WINDOWS:
                  layer_on(_wMou);
                  break;

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
        case APP_SWIPE:
            if (record->event.pressed) {
                keyball_swipe_begin(KBS_TAG_APP);
                swipe_timer = timer_read();
                canceller = false;  // ← APP の起動/キャンセル用のあなたの状態
            } else {
                keyball_swipe_end();
                if (timer_elapsed(swipe_timer) < TAPPING_TERM
                    && !keyball_swipe_fired_since_begin()) {
                    tap_code16_os(G(KC_TAB), m_MIS_CON, m_MIS_CON, KC_NO, KC_NO);
                }
            }
            break;

        case VOL_SWIPE:
            if (record->event.pressed) {
                keyball_swipe_begin(KBS_TAG_VOL);
                swipe_timer = timer_read();
            } else {
                keyball_swipe_end();
                if (timer_elapsed(swipe_timer) < TAPPING_TERM
                    && !keyball_swipe_fired_since_begin()) {
                    tap_code(KC_MPLY);
                }
            }
            break;

        case BROWSE_SWIPE:
            if (record->event.pressed) {
                keyball_swipe_begin(KBS_TAG_BRO);
                swipe_timer = timer_read();
            } else {
                keyball_swipe_end();
                if (timer_elapsed(swipe_timer) < TAPPING_TERM
                    && !keyball_swipe_fired_since_begin()) {
                    tap_code16_os(C(KC_L), G(KC_L), G(KC_L), KC_NO, KC_NO);
                }
            }
            break;

        case TAB_SWIPE:
            if (record->event.pressed) {
                keyball_swipe_begin(KBS_TAG_TAB);
                swipe_timer = timer_read();
            } else {
                keyball_swipe_end();
                if (timer_elapsed(swipe_timer) < TAPPING_TERM
                    && !keyball_swipe_fired_since_begin()) {
                    tap_code16_os(C(KC_T), G(KC_T), G(KC_T), KC_NO, KC_NO);
                }
            }
            break;

        case WIN_SWIPE:
            if (record->event.pressed) {
                keyball_swipe_begin(KBS_TAG_WIN);
                swipe_timer = timer_read();
            } else {
                keyball_swipe_end();
                if (host_os == OS_WINDOWS) {
                    unregister_code(KC_LGUI);
                }
                if (timer_elapsed(swipe_timer) < TAPPING_TERM
                    && !keyball_swipe_fired_since_begin()) {
                    tap_code16_os(G(KC_Z), A(C(KC_ENT)), A(C(KC_ENT)), KC_NO, KC_NO);
                }
            }
            break;

        case MULTI_A:
            if (record->event.pressed) {
                if (keyball_swipe_is_active()) {
                    switch (keyball_swipe_mode_tag()) {
                        case KBS_TAG_APP:
                            if (host_os == OS_MACOS || host_os == OS_IOS) tap_code16(m_L_DESK);
                            else                                           tap_code16(w_L_DESK);
                            break;
                        case KBS_TAG_TAB:
                            tap_code16(S(C(KC_TAB)));
                            break;
                        case KBS_TAG_BRO:
                            if (host_os == OS_MACOS || host_os == OS_IOS) tap_code16(G(KC_LEFT));
                            else                                           tap_code16(KC_WBAK);
                            break;
                        case KBS_TAG_VOL:
                        case KBS_TAG_WIN:
                        default:
                            // スワイプ中だが他モードなら何もしない/通常処理にフォールバックでもOK
                            break;
                    }
                } else {
                    // 非スワイプ時の通常処理
                    if (host_os == OS_MACOS || host_os == OS_IOS) tap_code16(G(KC_Z));
                    else                                           tap_code16(C(KC_Z));
                }
            }
            break;

        case MULTI_B:
            if (record->event.pressed) {
                if (keyball_swipe_is_active()) {
                    switch (keyball_swipe_mode_tag()) {
                        case KBS_TAG_APP:
                            if (host_os == OS_MACOS || host_os == OS_IOS) tap_code16(m_R_DESK);
                            else                                           tap_code16(w_R_DESK);
                            break;
                        case KBS_TAG_TAB:
                            tap_code16(C(KC_TAB));
                            break;
                        case KBS_TAG_BRO:
                            if (host_os == OS_MACOS || host_os == OS_IOS) tap_code16(G(KC_RIGHT));
                            else                                           tap_code16(KC_WFWD);
                            break;
                        case KBS_TAG_VOL:
                        case KBS_TAG_WIN:
                        default:
                            break;
                    }
                } else {
                    // 非スワイプ時の通常処理
                    if (host_os == OS_MACOS || host_os == OS_IOS) tap_code16(S(G(KC_Z)));
                    else                                           tap_code16(S(C(KC_Z)));
                }
            }
            break;



        // クリックすると state が SWIPE になり、離したら NONE になる
        // SWIPEの実装はswipe.hに記載する
        // case APP_SWIPE:
        //     if (record->event.pressed) {
        //         // キーダウン時
        //       swipe_mode = APP_SW;
        //       swipe_timer = timer_read();
        //       canceller = false;
        //       state = SWIPE;
        //     } else {
        //         // キーアップ時
        //       swipe_mode = NO_SW;
        //       state = NONE;
        //       canceller = false;
        //       if (timer_elapsed(swipe_timer) < TAPPING_TERM){
        //         tap_code16_os(G(KC_TAB), m_MIS_CON, m_MIS_CON, KC_NO, KC_NO);
        //       }
        //     }
        //     break;

        // case VOL_SWIPE:
        //     if (record->event.pressed) {
        //         // キーダウン時
        //         swipe_mode = VOL_SW;
        //         swipe_timer = timer_read();
        //         state = SWIPE;
        //     } else {
        //         // キーアップ時
        //         swipe_mode = NO_SW;
        //         state = NONE;
        //         if (timer_elapsed(swipe_timer) < TAPPING_TERM){
        //             tap_code(KC_MPLY);
        //         }
        //     }
        //     break;

        // case BROWSE_SWIPE:
        //     if (record->event.pressed) {
        //         // キーダウン時
        //         swipe_mode = BRO_SW;
        //         swipe_timer = timer_read();
        //         state = SWIPE;
        //     } else {
        //         // キーアップ時
        //         swipe_mode = NO_SW;
        //         state = NONE;
        //         if (timer_elapsed(swipe_timer) < TAPPING_TERM){
        //           tap_code16_os(C(KC_L), G(KC_L), G(KC_L), KC_NO, KC_NO);
        //         }
        //     }
        //     break;

        // case TAB_SWIPE:
        //     if (record->event.pressed) {
        //         // キーダウン時
        //         swipe_mode = TAB_SW;
        //         swipe_timer = timer_read();
        //         state = SWIPE;
        //     } else {
        //         // キーアップ時
        //         swipe_mode = NO_SW;
        //         state = NONE;
        //         if (timer_elapsed(swipe_timer) < TAPPING_TERM){
        //           tap_code16_os(C(KC_T), G(KC_T), G(KC_T), KC_NO, KC_NO);
        //         }
        //     }
        //     break;

        // case WIN_SWIPE:
        //     if (record->event.pressed) {
        //         // キーダウン時
        //         swipe_mode = WIN_SW;
        //         swipe_timer = timer_read();
        //         state = SWIPE;
        //     } else {
        //         // キーアップ時
        //         swipe_mode = NO_SW;
        //         state = NONE;
        //         if (host_os == OS_WINDOWS){
        //           unregister_code(KC_LGUI);
        //         }
        //         if (timer_elapsed(swipe_timer) < TAPPING_TERM){
        //           tap_code16_os(G(KC_Z), A(C(KC_ENT)), A(C(KC_ENT)), KC_NO, KC_NO);
        //         }
        //     }
        //     break;

        // case MULTI_A:
        //     if (record->event.pressed) {
        //       switch(swipe_mode) {
        //         case APP_SW:
        //           if (host_os == OS_MACOS || host_os == OS_IOS){
        //             tap_code16(m_L_DESK); // spotlight
        //           } else {
        //             tap_code16(w_L_DESK); // windows key
        //           }
        //           break;

        //         case VOL_SW:
        //           break;

        //         case TAB_SW:
        //           tap_code16(S(C(KC_TAB))); // next tab
        //           break;

        //         case BRO_SW:
        //           if (host_os == OS_MACOS || host_os == OS_IOS){
        //             tap_code16(G(KC_LEFT)); // browse back
        //           } else {
        //             tap_code16(KC_WBAK); // windows key
        //           }
        //           break;

        //         case WIN_SW:
        //           break;

        //         default:
        //         if (host_os == OS_MACOS || host_os == OS_IOS){
        //           tap_code16(G(KC_Z));
        //         } else {
        //           tap_code16(C(KC_Z));
        //         }
        //           break;
        //       }
        //     } else {
        //     }
        //     break;

        // case MULTI_B:
        //     if (record->event.pressed) {
        //       switch(swipe_mode) {
        //         case APP_SW:
        //           if (host_os == OS_MACOS || host_os == OS_IOS){
        //             tap_code16(m_R_DESK); // spotlight
        //           } else {
        //             tap_code16(w_R_DESK); // windows key
        //           }
        //           break;

        //         case VOL_SW:
        //           break;

        //         case TAB_SW:
        //           tap_code16(C(KC_TAB)); // previous tab
        //           break;

        //         case BRO_SW:
        //           if (host_os == OS_MACOS || host_os == OS_IOS){
        //             tap_code16(G(KC_RIGHT)); // browse back
        //           } else {
        //             tap_code16(KC_WFWD); // windows key
        //           }
        //           break;

        //         case WIN_SW:
        //           break;

        //         default:
        //         if (host_os == OS_MACOS || host_os == OS_IOS){
        //           tap_code16(S(G(KC_Z)));
        //         } else {
        //           tap_code16(S(C(KC_Z)));
        //         }
        //           break;
        //       }
        //     } else {
        //     }
        //     break;

        case EEPROM_RST:
            if (record->event.pressed) {
              reset_eeprom();
            } else {
            }
            break;

    }
    return true;
}


#include "quantum.h"
// #include "swipe.h"
#include "swipe_user.h"
#include "os_detection.h"
#include "../keycode_user.h"
#include "../features/util_user.h"
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
        case ESC_LNG2:
            if (record->event.pressed) {
                tap_code(KC_ESC);
                tap_code(KC_LNG2);
            }
            return false;

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

        // スワイプジェスチャー（APP_SW〜WIN_SW）、MULTI_A〜D はキーボードレベルで処理されます

    }
    return true;
}

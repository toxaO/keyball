#include "quantum.h"
// #include "swipe.h"
#include "swipe_user.h"
#include "os_detection.h"
#include "../keycode_user.h"
#include "../features/util_user.h"
#include QMK_KEYBOARD_H
#include "eeconfig.h"

#include "lib/keyball/keyball.h"
#include "deferred_exec.h"

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

static uint16_t pad_a_keydown_ms = 0; // deprecated: KC_A用だったが互換のため残置
static uint16_t multi_c_keydown_ms = 0;
static bool multi_c_active = false;

// FLICK 用ヘルパ状態
typedef struct {
    uint16_t down_ms;
} flick_state_t;

static bool handle_flick_key(kb_swipe_tag_t tag, uint16_t tap_kc, keyrecord_t *record, flick_state_t *st) {
    if (record->event.pressed) {
        st->down_ms = timer_read();
        keyball_swipe_begin(tag);
    } else {
        bool fired = keyball_swipe_fired_since_begin();
        uint16_t elapsed = timer_elapsed(st->down_ms);
        keyball_swipe_end();
        if (!fired && elapsed < TAPPING_TERM) {
            tap_code16(tap_kc);
        }
    }
    return false; // 常にここで完結
}

// MULTI_C（スワイプ有効時・FLICKタグ時）の単/二度押し制御用（遅延送出）
static deferred_token mc_token = INVALID_DEFERRED_TOKEN;
static uint16_t       mc_first_tap_ms = 0;
static bool           mc_waiting_single = false;
static uint32_t multi_c_single_cb(uint32_t trigger_time, void *cb_arg) {
    keyball_swipe_fire_once(KB_SWIPE_UP);
    mc_token = INVALID_DEFERRED_TOKEN;
    mc_waiting_single = false;
    return 0; // repeatなし
}

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

        // FLICK_*: 擬似フリック入力（_Pad想定）。
        // タップ=ベース文字、スワイプ方向は swipe_user.c のタグ別処理へ委譲。
        case FLICK_A: {
            static flick_state_t st; (void)pad_a_keydown_ms;
            return handle_flick_key(KBS_TAG_FLICK_A, KC_A, record, &st);
        }
        case FLICK_D: {
            static flick_state_t st;
            return handle_flick_key(KBS_TAG_FLICK_D, KC_D, record, &st);
        }
        case FLICK_G: {
            static flick_state_t st;
            return handle_flick_key(KBS_TAG_FLICK_G, KC_G, record, &st);
        }
        case FLICK_J: {
            static flick_state_t st;
            return handle_flick_key(KBS_TAG_FLICK_J, KC_J, record, &st);
        }
        case FLICK_M: {
            static flick_state_t st;
            return handle_flick_key(KBS_TAG_FLICK_M, KC_M, record, &st);
        }
        case FLICK_P: {
            static flick_state_t st;
            return handle_flick_key(KBS_TAG_FLICK_P, KC_P, record, &st);
        }
        case FLICK_S: {
            static flick_state_t st;
            return handle_flick_key(KBS_TAG_FLICK_S, KC_S, record, &st);
        }
        case FLICK_V: {
            static flick_state_t st;
            return handle_flick_key(KBS_TAG_FLICK_V, KC_V, record, &st);
        }

        case MULTI_A:
        case MULTI_B:
            // MULTI_A/B は keyboard レベル（multi_user.c）で処理する
            return true;

        case MULTI_C:
            if (keyball_swipe_is_active()) {
                kb_swipe_tag_t tag = keyball_swipe_mode_tag();
                // FLICK 系タグ有効時: タップ=UP、ダブルタップ=DOWN（UPはダブル時に抑止）
                if (kb_is_flick_tag(tag)) {
                    if (record->event.pressed) {
                        // ダブルタップ判定（直前の単発予約があり、TAPPING_TERM内の再押下）
                        if (mc_waiting_single && mc_token != INVALID_DEFERRED_TOKEN && timer_elapsed(mc_first_tap_ms) < TAPPING_TERM) {
                            cancel_deferred_exec(mc_token);
                            mc_token = INVALID_DEFERRED_TOKEN;
                            mc_waiting_single = false;
                            // ダブルタップ: DOWN相当のみ送出（UPは抑止）
                            keyball_swipe_fire_once(KB_SWIPE_DOWN);
                            return false;
                        }
                        // 単発候補: TAPPING_TERM後にUP相当を遅延送出
                        mc_first_tap_ms = timer_read();
                        mc_waiting_single = true;
                        mc_token = defer_exec(TAPPING_TERM, multi_c_single_cb, NULL);
                        return false;
                    } else {
                        // 離しでは何もしない（遅延送出 or 二度押しで処理済み）
                        return false;
                    }
                }
                return true; // FLICK以外のタグはKB側へ委譲
            }
            // タグなしデフォルト: LT(10, KC_ENTER) ≒ LT(_Set, KC_ENT)
            if (record->event.pressed) {
                multi_c_active = true;
                multi_c_keydown_ms = timer_read();
                layer_on(_Set);
            } else {
                if (multi_c_active) {
                    if (timer_elapsed(multi_c_keydown_ms) < TAPPING_TERM) {
                        tap_code16(KC_ENTER);
                    }
                    layer_off(_Set);
                    multi_c_active = false;
                }
            }
            return false;

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

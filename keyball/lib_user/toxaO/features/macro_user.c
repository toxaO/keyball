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
#include "print.h"

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

static uint16_t multi_c_keydown_ms = 0;
static bool multi_c_active = false;

// FLICK 用ヘルパ状態
typedef struct {
  uint16_t down_ms;
} flick_state_t;

// TG_PA_GU: タップ=TG(_Pad)、ホールド=LGUI押下（離して解除）
static deferred_token tg_pa_token = INVALID_DEFERRED_TOKEN;
static bool           tg_pa_gui_registered = false;
static uint16_t       tg_pa_press_time = 0;

static uint32_t tg_pa_hold_cb(uint32_t trigger_time, void *cb_arg) {
  register_code(KC_LGUI);
  tg_pa_gui_registered = true;
  tg_pa_token = INVALID_DEFERRED_TOKEN;
  return 0;
}

static bool handle_flick_key(kb_swipe_tag_t tag, uint16_t tap_kc, keyrecord_t *record, flick_state_t *st) {
  if (record->event.pressed) {
    st->down_ms = timer_read();
    keyball_swipe_begin(tag);
  } else {
    bool fired = keyball_swipe_fired_since_begin();
    uint16_t elapsed = timer_elapsed(st->down_ms);
    keyball_swipe_end();
    if (!fired && elapsed < TAPPING_TERM) {
      tap_code16_with_oneshot(tap_kc);
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

// tap_code16_osはuser_utilで定義
// tap_code16_with_oneshot(win, mac, os, linux, unsure)でキーコード指定可能

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  /* swipe_mode = keycode; */
  mod_state = get_mods();


  switch (keycode) {

    //------------------------------------------------------------
    // 個人的なやつ
    //------------------------------------------------------------
    case TG_PA_GU:
      if (record->event.pressed) {
        // 押下時: TAPPING_TERM 経過で LGUI を押下する遅延をセット
        tg_pa_gui_registered = false;
        tg_pa_press_time = timer_read();
        if (tg_pa_token != INVALID_DEFERRED_TOKEN) {
          cancel_deferred_exec(tg_pa_token);
        }
        tg_pa_token = defer_exec(TAPPING_TERM, tg_pa_hold_cb, NULL);
      } else {
        // 離した時: タップ or ホールドを確定
        if (tg_pa_token != INVALID_DEFERRED_TOKEN && timer_elapsed(tg_pa_press_time) >= TAPPING_TERM) {
          // 実時間はホールド相当だが、遅延CBが未実行の場合の救済
          cancel_deferred_exec(tg_pa_token);
          tg_pa_token = INVALID_DEFERRED_TOKEN;
          if (!tg_pa_gui_registered) {
            register_code(KC_LGUI);
            tg_pa_gui_registered = true;
          }
          unregister_code(KC_LGUI);
          tg_pa_gui_registered = false;
        } else if (tg_pa_token != INVALID_DEFERRED_TOKEN) {
          // TAPPING_TERM 未満のタップ -> TG(_Pad)
          cancel_deferred_exec(tg_pa_token);
          tg_pa_token = INVALID_DEFERRED_TOKEN;
          layer_invert(_Pad);
        } else if (tg_pa_gui_registered) {
          // ホールド扱い中 -> LGUI を解除
          unregister_code(KC_LGUI);
          tg_pa_gui_registered = false;
        }
      }
      return false;

    case ESC_LNG2:
      if (record->event.pressed) { tap_code16_with_oneshot(KC_ESC); tap_code16_with_oneshot(KC_LNG2); }
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

    // LED_NEXT: 一時的なLED検証機能は撤回

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
          tap_code16_with_oneshot(KC_LNG2);
        }
        layer_off(_NumP);
      }
      return false;

    case KANA_C_N:
      if (record->event.pressed) {
        if (TIMER_DIFF_16(record->event.time, first_ctrl_tap_time) < 500) {
          tap_code16_with_oneshot(KC_LNG2);
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
          tap_code16_with_oneshot(KC_LNG1);
        }
        layer_off(_NumP);
      }
      return false;

    //------------------------------------------------------------
    // multi key 独自実装
    //------------------------------------------------------------
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
                    layer_on(_Scr);
                  } else {
                    if (multi_c_active) {
                      if (timer_elapsed(multi_c_keydown_ms) < TAPPING_TERM) {
                        tap_code16_with_oneshot(KC_ENTER);
                      }
                      layer_off(_Scr);
                      multi_c_active = false;
                    }
                  }
                  return false;

    //------------------------------------------------------------
    // FLICK_*: 擬似フリック入力（_Pad想定）。
    // タップ=ベース文字、スワイプ方向は swipe_user.c のタグ別処理へ委譲。
    //------------------------------------------------------------
    case FLICK_A: {
                    static flick_state_t st;
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

    //------------------------------------------------------------
    // basic shortcut key
    //------------------------------------------------------------
    case RALT(KC_C):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                      case OS_IOS:
                        tap_code16_with_oneshot(LGUI(KC_C));
                        break;
                      default:
                        tap_code16_with_oneshot(LCTL(KC_C));
                        break;
                    }
                  }
                  return false;

    case RALT(KC_V):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                      case OS_IOS:
                        tap_code16_with_oneshot(LGUI(KC_V));
                        break;
                      default:
                        tap_code16_with_oneshot(LCTL(KC_V));
                        break;
                    }
                  }
                  return false;

    case RALT(KC_X):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                      case OS_IOS:
                        tap_code16_with_oneshot(LGUI(KC_X));
                        break;
                      default:
                        tap_code16_with_oneshot(LCTL(KC_X));
                        break;
                    }
                  }
                  return false;

    case RALT(KC_BSPC):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                      case OS_IOS:
                        tap_code16_with_oneshot(RALT(KC_BSPC));
                        break;
                      default:
                        tap_code16_with_oneshot(KC_DEL);
                        break;
                    }
                  }
                  return false;

    case RALT(KC_A):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                      case OS_IOS:
                        tap_code16_with_oneshot(RGUI(KC_A));
                        break;
                      default:
                        tap_code16_with_oneshot(LCTL(KC_A));
                        break;
                    }
                  }
                  return false;

    case RALT(KC_W):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                      case OS_IOS:
                        tap_code16_with_oneshot(RGUI(KC_W));
                        break;
                      default:
                        tap_code16_with_oneshot(LCTL(KC_W));
                        break;
                    }
                  }
                  return false;

    case RALT(KC_Z):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                      case OS_IOS:
                        tap_code16_with_oneshot(RGUI(KC_Z));
                        break;
                      default:
                        tap_code16_with_oneshot(LCTL(KC_Z));
                        break;
                    }
                  }
                  return false;

    case RALT(KC_Y):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                      case OS_IOS:
                        tap_code16_with_oneshot(RGUI(KC_Y));
                        break;
                      default:
                        tap_code16_with_oneshot(LCTL(KC_Y));
                        break;
                    }
                  }
                  return false;

    case RALT(KC_Q):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                      case OS_IOS:
                        tap_code16_with_oneshot(RGUI(KC_Q));
                        break;
                      default:
                        tap_code16_with_oneshot(LCTL(KC_Q));
                        break;
                    }
                  }
                  return false;

    case RALT(KC_F):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                      case OS_IOS:
                        tap_code16_with_oneshot(RGUI(KC_F));
                        break;
                      default:
                        tap_code16_with_oneshot(LCTL(KC_F));
                        break;
                    }
                  }
                  return false;

    case RALT(KC_S):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                      case OS_IOS:
                        tap_code16_with_oneshot(RGUI(KC_S));
                        break;
                      default:
                        tap_code16_with_oneshot(LCTL(KC_S));
                        break;
                    }
                  }
                  return false;

    case RALT(KC_R):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                      case OS_IOS:
                        tap_code16_with_oneshot(RGUI(KC_R));
                        break;
                      default:
                        tap_code16_with_oneshot(LCTL(KC_R));
                        break;
                    }
                  }
                  return false;

    case RALT(KC_T):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                      case OS_IOS:
                        tap_code16_with_oneshot(RGUI(KC_T));
                        break;
                      default:
                        tap_code16_with_oneshot(LCTL(KC_T));
                        break;
                    }
                  }
                  return false;

    case RALT(S(KC_T)):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                      case OS_IOS:
                        tap_code16_with_oneshot(RGUI(S(KC_T)));
                        break;
                      default:
                        tap_code16_with_oneshot(LCTL(S(KC_T)));
                        break;
                    }
                  }
                  return false;

    case RALT(KC_TAB):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                      case OS_IOS:
                        tap_code16_with_oneshot(RGUI(KC_TAB));
                        break;
                      default:
                        tap_code16_with_oneshot(LCTL(KC_TAB));
                        break;
                    }
                  }
                  return false;

    case RALT(S(KC_TAB)):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                      case OS_IOS:
                        tap_code16_with_oneshot(RGUI(S(KC_TAB)));
                        break;
                      default:
                        tap_code16_with_oneshot(LCTL(S(KC_TAB)));
                        break;
                    }
                  }
                  return false;

    //------------------------------------------------------------
    // home end key
    //------------------------------------------------------------
    case RALT(KC_LEFT):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_LEFT));
                      default:
                        tap_code16_with_oneshot(KC_HOME);
                    }
                  }
                  return false;

    case RALT(KC_RIGHT):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_RIGHT));
                      default:
                        tap_code16_with_oneshot(KC_END);
                    }
                  }
                  return false;

    case RALT(KC_UP):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_UP));
                      default:
                        tap_code16_with_oneshot(KC_PGUP);
                    }
                  }
                  return false;

    case RALT(KC_DOWN):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_DOWN));
                      default:
                        tap_code16_with_oneshot(KC_PGDN);
                    }
                  }
                  return false;


    //------------------------------------------------------------
    // function key
    //------------------------------------------------------------
    case RALT(KC_F1):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F1));
                      default:
                        tap_code16_with_oneshot(KC_F1);
                    }
                  }
                  return false;

    case RALT(KC_F2):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F2));
                      default:
                        tap_code16_with_oneshot(KC_F2);
                    }
                  }
                  return false;

    case RALT(KC_F3):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F3));
                      default:
                        tap_code16_with_oneshot(KC_F3);
                    }
                  }
                  return false;

    case RALT(KC_F4):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F4));
                      default:
                        tap_code16_with_oneshot(KC_F4);
                    }
                  }
                  return false;

    case RALT(KC_F5):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F5));
                      default:
                        tap_code16_with_oneshot(KC_F5);
                    }
                  }
                  return false;

    case RALT(KC_F6):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F6));
                      default:
                        tap_code16_with_oneshot(KC_F6);
                    }
                  }
                  return false;

    case RALT(KC_F7):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F7));
                      default:
                        tap_code16_with_oneshot(KC_F7);
                    }
                  }
                  return false;

    case RALT(KC_F8):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F8));
                      default:
                        tap_code16_with_oneshot(KC_F8);
                    }
                  }
                  return false;

    case RALT(KC_F9):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F9));
                      default:
                        tap_code16_with_oneshot(KC_F9);
                    }
                  }
                  return false;

    case RALT(KC_F10):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F10));
                      default:
                        tap_code16_with_oneshot(KC_F10);
                    }
                  }
                  return false;

    case RALT(KC_F11):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F11));
                      default:
                        tap_code16_with_oneshot(KC_F11);
                    }
                  }
                  return false;

    case RALT(KC_F12):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F12));
                      default:
                        tap_code16_with_oneshot(KC_F12);
                    }
                  }
                  return false;

    case RALT(KC_F13):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F13));
                      default:
                        tap_code16_with_oneshot(KC_F13);
                    }
                  }
                  return false;

    case RALT(KC_F14):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F14));
                      default:
                        tap_code16_with_oneshot(KC_F14);
                    }
                  }
                  return false;


    case RALT(KC_F15):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F15));
                      default:
                        tap_code16_with_oneshot(KC_F15);
                    }
                  }
                  return false;

    case RALT(KC_F16):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F16));
                      default:
                        tap_code16_with_oneshot(KC_F16);
                    }
                  }
                  return false;

    case RALT(KC_F17):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F17));
                      default:
                        tap_code16_with_oneshot(KC_F17);
                    }
                  }
                  return false;

    case RALT(KC_F18):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F18));
                      default:
                        tap_code16_with_oneshot(KC_F18);
                    }
                  }
                  return false;

    case RALT(KC_F19):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F19));
                      default:
                        tap_code16_with_oneshot(KC_F19);
                    }
                  }
                  return false;

    case RALT(KC_F20):
                  if(record->event.pressed) {
                    switch(host_os) {
                      case OS_MACOS:
                        tap_code16_with_oneshot(RCTL(KC_F20));
                      default:
                        tap_code16_with_oneshot(KC_F20);
                    }
                  }
                  return false;

  } // /switch(keycode)
  return true;
}

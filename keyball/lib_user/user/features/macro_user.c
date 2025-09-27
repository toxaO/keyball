#include "quantum.h"
// #include "swipe.h"
#include "swipe_user.h"
#include "os_detection.h"
#include "../features/util_user.h"
#include QMK_KEYBOARD_H
#include "eeconfig.h"

#include "lib/keyball/keyball.h"
#include "lib_user/user/keycode_user.h"
#include "deferred_exec.h"
#include "print.h"

// uint16_t swipe_timer; // スワイプキーがTAPPING_TERMにあるかを判定する (≒ mod_tap)
// bool canceller = false;
uint8_t mod_state;

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
                  // タグなしデフォルト
                  if (record->event.pressed) {
                    // 押した時
                  } else {
                    // 離した時
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

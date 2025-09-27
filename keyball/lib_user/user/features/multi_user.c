#include "quantum.h"
#include "../keycode_user.h"
#include "../features/util_user.h"
#include "print.h"
#include "os_detection.h"

#include "lib/keyball/keyball.h"
#include "lib/keyball/keyball_multi.h"
#include "swipe_user.h" // KBS_TAG_PAD_A の定義

void keyball_on_multi_a(kb_swipe_tag_t tag) {
    // フリックキー押下中は、早期リターンで各方向のスワイプ実行処理に入る
    if (kb_is_flick_tag(tag)) { keyball_swipe_fire_once(KB_SWIPE_LEFT); return; }
    if (tag == 0) {
        // 非スワイプ時のデフォルト: Undo
        // Windows/Linux: Ctrl+Z, macOS/iOS: GUI+Z
        tap_code16_os(C(KC_Z), G(KC_Z), G(KC_Z), C(KC_Z), C(KC_Z));
        return;
    }
    switch (tag) {
    case KBS_TAG_APP: tap_code16_os(w_L_DESK, m_L_DESK, m_L_DESK, KC_NO, KC_NO); break;
    case KBS_TAG_TAB: tap_code16(S(C(KC_TAB))); break;
    case KBS_TAG_BRO: tap_code16_os(KC_WBAK, G(KC_LEFT), G(KC_LEFT), KC_NO, KC_NO); break;
    case KBS_TAG_VOL: tap_code(KC_MNXT); break;
    case KBS_TAG_WIN:
        if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_LEFT); }
        else                        { tap_code16(RCTL(LCTL(KC_L))); }
        break;
    default: tap_code16(KC_F1); break;
    }
}

void keyball_on_multi_b(kb_swipe_tag_t tag) {
    if (kb_is_flick_tag(tag)) { keyball_swipe_fire_once(KB_SWIPE_RIGHT); return; }
    if (tag == 0) {
        // 非スワイプ時のデフォルト: Redo
        // Windows/Linux: Ctrl+Y, macOS/iOS: GUI+Shift+Z
        tap_code16_os(C(KC_Y), S(G(KC_Z)), S(G(KC_Z)), C(KC_Y), C(KC_Y));
        return;
    }
    switch (tag) {
    case KBS_TAG_APP: tap_code16_os(w_R_DESK, m_R_DESK, m_R_DESK, KC_NO, KC_NO); break;
    case KBS_TAG_TAB: tap_code16(C(KC_TAB)); break;
    case KBS_TAG_BRO: tap_code16_os(KC_WFWD, G(KC_RIGHT), G(KC_RIGHT), KC_NO, KC_NO); break;
    case KBS_TAG_VOL: tap_code(KC_MPRV); break;
    case KBS_TAG_WIN:
        if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_RIGHT); }
        else                        { tap_code16(RCTL(LCTL(KC_R))); }
        break;
    default: tap_code16(KC_F2); break;
    }
}

void keyball_on_multi_c(kb_swipe_tag_t tag) {
    // FLICK 系は user-level(macro_user.c) 側で処理（遅延送出）。
    // 向こうでtrueを返されるとこちらへくる。falseだとこっちへ来ない。
    if (kb_is_flick_tag(tag)) { return; }
    if (tag == 0) { tap_code16(KC_F3); return; }
    switch (tag) {
    case KBS_TAG_APP: tap_code16_os(w_TASK, LCTL(KC_UP), LCTL(KC_UP), KC_NO, KC_NO); break;
    case KBS_TAG_TAB: tap_code16_os(LAST_TAB, S(G(KC_T)), S(G(KC_T)), KC_NO, KC_NO); break;
    case KBS_TAG_BRO: tap_code16_os(RELOAD, RELOAD, RELOAD, KC_NO, KC_NO); break;
    case KBS_TAG_VOL: tap_code(KC_VOLU); break;
    case KBS_TAG_WIN:
        if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_UP); }
        else                        { tap_code16(RCTL(LCTL(KC_U))); }
        break;
    default: tap_code16(KC_F3); break;
    }
}

void keyball_on_multi_d(kb_swipe_tag_t tag) {
    if (tag == 0) { tap_code16(KC_F4); return; }
    switch (tag) {
    case KBS_TAG_APP: tap_code16_os(w_DESK, LCTL(KC_DOWN), LCTL(KC_DOWN), KC_NO, KC_NO); break;
    case KBS_TAG_TAB: tap_code16_os(CLOSE, CLOSE, CLOSE, KC_NO, KC_NO); break;
    case KBS_TAG_BRO: tap_code16_os(NEW_TAB, NEW_TAB, NEW_TAB, KC_NO, KC_NO); break;
    case KBS_TAG_VOL: tap_code(KC_VOLD); break;
    case KBS_TAG_WIN:
        if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_DOWN); }
        else                        { tap_code16(RCTL(LCTL(KC_D))); }
        break;
    default: tap_code16(KC_F4); break;
    }
}

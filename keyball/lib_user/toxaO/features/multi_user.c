#include "quantum.h"
#include "../keycode_user.h"
#include "../features/util_user.h"
#include "print.h"
#include "os_detection.h"

#include "lib/keyball/keyball.h"
#include "lib/keyball/keyball_multi.h"

void keyball_on_multi_a(kb_swipe_tag_t tag) {
    if (tag == 0) { tap_code16(KC_F1); return; }
    switch (tag) {
    case KBS_TAG_APP: tap_code16_os(w_L_DESK, m_L_DESK, m_L_DESK, KC_NO, KC_NO); break;
    case KBS_TAG_TAB: tap_code16(S(C(KC_TAB))); break;
    case KBS_TAG_BRO: tap_code16_os(KC_WBAK, G(KC_LEFT), G(KC_LEFT), KC_NO, KC_NO); break;
    case KBS_TAG_VOL: tap_code(KC_MNXT); break;
    case KBS_TAG_WIN:
        if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_LEFT); }
        else                        { tap_code16(MGN_L); }
        break;
    default: tap_code16(KC_F1); break;
    }
}

void keyball_on_multi_b(kb_swipe_tag_t tag) {
    if (tag == 0) { tap_code16(KC_F2); return; }
    switch (tag) {
    case KBS_TAG_APP: tap_code16_os(w_R_DESK, m_R_DESK, m_R_DESK, KC_NO, KC_NO); break;
    case KBS_TAG_TAB: tap_code16(C(KC_TAB)); break;
    case KBS_TAG_BRO: tap_code16_os(KC_WFWD, G(KC_RIGHT), G(KC_RIGHT), KC_NO, KC_NO); break;
    case KBS_TAG_VOL: tap_code(KC_MPRV); break;
    case KBS_TAG_WIN:
        if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_RIGHT); }
        else                        { tap_code16(MGN_R); }
        break;
    default: tap_code16(KC_F2); break;
    }
}

void keyball_on_multi_c(kb_swipe_tag_t tag) {
    if (tag == 0) { tap_code16(KC_F3); return; }
    switch (tag) {
    case KBS_TAG_APP: tap_code16_os(w_TASK, m_MIS_CON, m_MIS_CON, KC_NO, KC_NO); break;
    case KBS_TAG_TAB: tap_code16_os(w_LAST_TAB, S(G(KC_T)), S(G(KC_T)), KC_NO, KC_NO); break;
    case KBS_TAG_BRO: tap_code16_os(w_RELOAD, m_RELOAD, m_RELOAD, KC_NO, KC_NO); break;
    case KBS_TAG_VOL: tap_code(KC_VOLU); break;
    case KBS_TAG_WIN:
        if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_UP); }
        else                        { tap_code16(MGN_U); }
        break;
    default: tap_code16(KC_F3); break;
    }
}

void keyball_on_multi_d(kb_swipe_tag_t tag) {
    if (tag == 0) { tap_code16(KC_F4); return; }
    switch (tag) {
    case KBS_TAG_APP: tap_code16_os(w_DESK, m_APP_CON, m_APP_CON, KC_NO, KC_NO); break;
    case KBS_TAG_TAB: tap_code16_os(w_CLOSE, m_CLOSE, m_CLOSE, KC_NO, KC_NO); break;
    case KBS_TAG_BRO: tap_code16_os(w_NEW_TAB, m_NEW_TAB, m_NEW_TAB, KC_NO, KC_NO); break;
    case KBS_TAG_VOL: tap_code(KC_VOLD); break;
    case KBS_TAG_WIN:
        if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_DOWN); }
        else                        { tap_code16(MGN_D); }
        break;
    default: tap_code16(KC_F4); break;
    }
}

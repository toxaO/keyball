#include "quantum.h"
#include "my_keycode.h"
#include "print.h"
#include "os_detection.h"

#include "swipe_user.h"
#include "util.h"
#include "lib/keyball/keyball.h"


void keyball_on_swipe_fire(kb_swipe_tag_t tag, kb_swipe_dir_t dir) {
    dprintf("SWIPE FIRE tag=%u dir=%u\n", (unsigned)tag, (unsigned)dir);
    switch (tag) {
    case KBS_TAG_APP:
        switch (dir) {
        case KB_SWIPE_UP:
            if (canceller) { tap_code(KC_ESC); canceller = false; }
            else           { tap_code16_os(w_Ueli, G(KC_SPACE), G(KC_SPACE), KC_NO, KC_NO); canceller = true; }
            break;
        case KB_SWIPE_DOWN:
            if (canceller) { tap_code(KC_ESC); canceller = false; }
            else           { tap_code16_os(G(KC_TAB), C(KC_DOWN), C(KC_DOWN), KC_NO, KC_NO); canceller = true; }
            break;
        case KB_SWIPE_LEFT:  tap_code16_os(w_R_DESK, m_R_DESK, m_R_DESK, KC_NO, KC_NO); break;
        case KB_SWIPE_RIGHT: tap_code16_os(w_L_DESK, m_L_DESK, m_L_DESK, KC_NO, KC_NO); break;
        default: break;
        }
        break;

    case KBS_TAG_VOL:
        switch (dir) {
        case KB_SWIPE_UP:    tap_code(KC_VOLU); break;
        case KB_SWIPE_DOWN:  tap_code(KC_VOLD); break;
        case KB_SWIPE_LEFT:  tap_code(KC_MNXT); break;
        case KB_SWIPE_RIGHT: tap_code(KC_MPRV); break;
        default: break;
        }
        break;

    case KBS_TAG_BRO:
        switch (dir) {
        case KB_SWIPE_UP:    tap_code16_os(C(KC_C), G(KC_C), G(KC_C), KC_NO, KC_NO); break;
        case KB_SWIPE_DOWN:  tap_code16_os(C(KC_V), G(KC_V), G(KC_V), KC_NO, KC_NO); break;
        case KB_SWIPE_LEFT:  tap_code16_os(KC_WBAK, G(KC_LEFT),  G(KC_LEFT),  KC_NO, KC_NO); break;
        case KB_SWIPE_RIGHT: tap_code16_os(KC_WFWD, G(KC_RIGHT), G(KC_RIGHT), KC_NO, KC_NO); break;
        default: break;
        }
        break;

    case KBS_TAG_TAB:
        switch (dir) {
        case KB_SWIPE_UP:
            dprintf("TAB_SW UP (OS=%d)\n", host_os);
            tap_code16_os(S(C(KC_T)), S(G(KC_T)), S(G(KC_T)), KC_NO, KC_NO);
            break;
        case KB_SWIPE_DOWN:
            dprintf("TAB_SW DOWN (OS=%d)\n", host_os);
            tap_code16_os(w_CLOSE, m_CLOSE, m_CLOSE, KC_NO, KC_NO);
            break;
        case KB_SWIPE_LEFT:
            dprintf("TAB_SW LEFT (OS=%d)\n", host_os);
            tap_code16(S(C(KC_TAB)));
            break;
        case KB_SWIPE_RIGHT:
            dprintf("TAB_SW RIGHT (OS=%d)\n", host_os);
            tap_code16(C(KC_TAB));
            break;
        default: break;
        }
        break;

    case KBS_TAG_WIN:
        switch (dir) {
        case KB_SWIPE_UP:
            if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_UP); }
            else                        { tap_code16(MGN_U); }
            break;
        case KB_SWIPE_DOWN:
            if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_DOWN); }
            else                        { tap_code16(MGN_D); }
            break;
        case KB_SWIPE_LEFT:
            if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_LEFT); }
            else                        { tap_code16(MGN_L); }
            break;
        case KB_SWIPE_RIGHT:
            if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_RIGHT); }
            else                        { tap_code16(MGN_R); }
            break;
        default: break;
        }
        break;

    default:
        break;
    }
}

// セッション終了時のクリーンアップ（Windowsスワイプで押しっぱなしのWinキーを解放）
void keyball_on_swipe_end(kb_swipe_tag_t tag) {
    // Lockレイヤを確実に解除（押しっぱなし等で取りこぼしを防ぐ）
    layer_off(_Lock);
    if (tag == KBS_TAG_WIN) {
        if (host_os == OS_WINDOWS) {
            unregister_code(KC_LGUI);
        }
    }
}

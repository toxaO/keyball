#include "quantum.h"
#include "../keycode_user.h"
#include "../features/util_user.h"
#include "print.h"
#include "os_detection.h"
#include "timer.h"

#include "swipe_user.h"
#include "lib/keyball/keyball.h"
#include "lib/keyball/keyball_swipe.h"


// Swipeの動作はこのファイルで指定します。

bool canceller = false;
static bool bro_double_wait = false;
static uint16_t bro_double_timer = 0;

// tap_code16_osはuser_utilで定義
// tap_code16_with_oneshot(win, mac, os, linux, unsure)でキーコード指定可能

void keyball_on_swipe_fire(kb_swipe_tag_t tag, kb_swipe_dir_t dir) {
    dprintf("SWIPE FIRE tag=%u dir=%u\n", (unsigned)tag, (unsigned)dir);
    switch (tag) {
    case KBS_TAG_SW_APP:
        switch (dir) {
        case KB_SWIPE_UP:
            if (canceller) { tap_code16_with_oneshot(KC_ESC); canceller = false; }
            else           { tap_code16_os(A(KC_SPACE), G(KC_SPACE), G(KC_SPACE), KC_NO, KC_NO); canceller = true; }
            break;
        case KB_SWIPE_DOWN:
            if (canceller) { tap_code16_with_oneshot(KC_ESC); canceller = false; }
            else           { tap_code16_os(G(KC_TAB), C(KC_DOWN), C(KC_DOWN), KC_NO, KC_NO); canceller = true; }
            break;
        case KB_SWIPE_LEFT:  tap_code16_os(w_R_DESK, m_R_DESK, m_R_DESK, KC_NO, KC_NO); break;
        case KB_SWIPE_RIGHT: tap_code16_os(w_L_DESK, m_L_DESK, m_L_DESK, KC_NO, KC_NO); break;
        default: break;
        }
        break;

    case KBS_TAG_SW_VOL:
        switch (dir) {
        case KB_SWIPE_UP:    tap_code16_with_oneshot(KC_VOLU); break;
        case KB_SWIPE_DOWN:  tap_code16_with_oneshot(KC_VOLD); break;
        case KB_SWIPE_LEFT:  tap_code16_with_oneshot(KC_MNXT); break;
        case KB_SWIPE_RIGHT: tap_code16_with_oneshot(KC_MPRV); break;
        default: break;
        }
        break;

    case KBS_TAG_SW_BRO:
        switch (dir) {
        case KB_SWIPE_UP:
            tap_code16_os(C(S(KC_EQUAL)), G(KC_SCLN), G(KC_SCLN), C(S(KC_EQUAL)), C(S(KC_EQUAL)));
            break;
        case KB_SWIPE_DOWN:
            tap_code16_os(C(KC_MINUS), G(KC_MINUS), G(KC_MINUS), C(KC_MINUS), C(KC_MINUS));
            break;
        case KB_SWIPE_LEFT:  tap_code16_os(KC_WBAK, G(KC_LEFT),  G(KC_LEFT),  KC_NO, KC_NO); break;
        case KB_SWIPE_RIGHT: tap_code16_os(KC_WFWD, G(KC_RIGHT), G(KC_RIGHT), KC_NO, KC_NO); break;
        default: break;
        }
        break;

    case KBS_TAG_SW_TAB:
        switch (dir) {
        case KB_SWIPE_UP:
            dprintf("SW_TAB UP (OS=%d)\n", host_os);
            tap_code16_os(S(C(KC_T)), S(G(KC_T)), S(G(KC_T)), KC_NO, KC_NO);
            break;
        case KB_SWIPE_DOWN:
            dprintf("SW_TAB DOWN (OS=%d)\n", host_os);
            tap_code16_os(LCTL(KC_W), LGUI(KC_W), LGUI(KC_W), KC_NO, KC_NO);
            break;
        case KB_SWIPE_LEFT:
            dprintf("SW_TAB LEFT (OS=%d)\n", host_os);
            tap_code16_with_oneshot(S(C(KC_TAB)));
            break;
        case KB_SWIPE_RIGHT:
            dprintf("SW_TAB RIGHT (OS=%d)\n", host_os);
            tap_code16_with_oneshot(C(KC_TAB));
            break;
        default: break;
        }
        break;

    case KBS_TAG_SW_WIN:
        switch (dir) {
        case KB_SWIPE_UP:
            if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code16_with_oneshot(KC_UP); }
            else                        { tap_code16_with_oneshot(RCTL(LCTL(KC_UP))); }
            break;
        case KB_SWIPE_DOWN:
            if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code16_with_oneshot(KC_DOWN); }
            else                        { tap_code16_with_oneshot(RCTL(LCTL(KC_DOWN))); }
            break;
        case KB_SWIPE_LEFT:
            if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code16_with_oneshot(KC_LEFT); }
            else                        { tap_code16_with_oneshot(RCTL(LCTL(KC_LEFT))); }
            break;
        case KB_SWIPE_RIGHT:
            if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code16_with_oneshot(KC_RIGHT); }
            else                        { tap_code16_with_oneshot(RCTL(LCTL(KC_RIGHT))); }
            break;
        default: break;
        }
        break;

    case KBS_TAG_SW_UTIL:
        switch (dir) {
        case KB_SWIPE_UP:
            tap_code16_os(C(KC_C), G(KC_C), G(KC_C), C(KC_C), C(KC_C));
            break;
        case KB_SWIPE_DOWN:
            tap_code16_os(C(KC_V), G(KC_V), G(KC_V), C(KC_V), C(KC_V));
            break;
        case KB_SWIPE_LEFT:
            tap_code16_os(C(KC_Z), G(KC_Z), G(KC_Z), C(KC_Z), C(KC_Z));
            break;
        case KB_SWIPE_RIGHT:
            tap_code16_os(C(KC_Y), S(G(KC_Z)), S(G(KC_Z)), C(S(KC_Z)), C(KC_Y));
            break;
        default:
            break;
        }
        break;

    case KBS_TAG_SW_ARR:
        // Arrow proxy: swipe方向に応じて矢印キーを発火
        switch (dir) {
        case KB_SWIPE_UP:    tap_code16_with_oneshot(KC_UP);    break;
        case KB_SWIPE_DOWN:  tap_code16_with_oneshot(KC_DOWN);  break;
        case KB_SWIPE_LEFT:  tap_code16_with_oneshot(KC_LEFT);  break;
        case KB_SWIPE_RIGHT: tap_code16_with_oneshot(KC_RIGHT); break;
        default: break;
        }
        break;

    // 例: 拡張スワイプキー（SW_EX1/SW_EX2）のユーザーオーバーライド
    // デフォルト動作はユーザーレベルで定義してください（本ファイルで上書き可能）。
    case KBS_TAG_EX1:
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16_with_oneshot(KC_F14); break;
        case KB_SWIPE_DOWN:  tap_code16_with_oneshot(KC_F15); break;
        case KB_SWIPE_UP:    tap_code16_with_oneshot(KC_F16); break;
        case KB_SWIPE_RIGHT: tap_code16_with_oneshot(KC_F17); break;
        default: break;
        }
        break;
    case KBS_TAG_EX2:
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16_with_oneshot(KC_F19); break;
        case KB_SWIPE_DOWN:  tap_code16_with_oneshot(KC_F20); break;
        case KB_SWIPE_UP:    tap_code16_with_oneshot(KC_F21); break;
        case KB_SWIPE_RIGHT: tap_code16_with_oneshot(KC_F22); break;
        default: break;
        }
        break;

    // FLICK_* 系：ベースキーごとの方向マッピング
    case KBS_TAG_FLICK_A:
        // {tap=A, up=@, right=C, left=B, down=None}
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16_with_oneshot(KC_B); break;
        case KB_SWIPE_RIGHT: tap_code16_with_oneshot(KC_C); break;
        case KB_SWIPE_UP:    tap_code16_with_oneshot(AT);   break;
        default: break;
        }
        break;

    case KBS_TAG_FLICK_D:
        // {tap=D, up=(, right=F, left=E, down=)}
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16_with_oneshot(KC_E);      break;
        case KB_SWIPE_RIGHT: tap_code16_with_oneshot(KC_F);      break;
        case KB_SWIPE_UP:    tap_code16_with_oneshot(L_PAREN);   break;
        case KB_SWIPE_DOWN:  tap_code16_with_oneshot(R_PAREN);   break;
        default: break;
        }
        break;

    case KBS_TAG_FLICK_G:
        // {tap=G, up=None, right=I, left=H, down=None}
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16_with_oneshot(KC_H); break;
        case KB_SWIPE_RIGHT: tap_code16_with_oneshot(KC_I); break;
        default: break;
        }
        break;

    case KBS_TAG_FLICK_J:
        // {tap=J, up=None, right=L, left=K, down=None}
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16_with_oneshot(KC_K); break;
        case KB_SWIPE_RIGHT: tap_code16_with_oneshot(KC_L); break;
        default: break;
        }
        break;


    case KBS_TAG_FLICK_M:
        // {tap=M, up=None, right=O, left=N, down=None}
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16_with_oneshot(KC_N); break;
        case KB_SWIPE_RIGHT: tap_code16_with_oneshot(KC_O); break;
        default: break;
        }
        break;

    case KBS_TAG_FLICK_P:
        // {tap=P, up=None, right=R, left=Q, down=None}
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16_with_oneshot(KC_Q); break;
        case KB_SWIPE_UP  : tap_code16_with_oneshot(KC_R); break;
        case KB_SWIPE_RIGHT: tap_code16_with_oneshot(KC_S); break;
        default: break;
        }
        break;

    case KBS_TAG_FLICK_T:
        // {tap=S, up=None, right=U, left=T, down=None}
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16_with_oneshot(KC_U); break;
        case KB_SWIPE_RIGHT: tap_code16_with_oneshot(KC_V); break;
        default: break;
        }
        break;

    case KBS_TAG_FLICK_W:
        // {tap=V, up=Y, right=W, left=Z, down=None}
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16_with_oneshot(KC_X); break;
        case KB_SWIPE_UP:    tap_code16_with_oneshot(KC_Y); break;
        case KB_SWIPE_RIGHT: tap_code16_with_oneshot(KC_Z); break;
        default: break;
        }
        break;

    default:
        break;
    }
}

// セッション終了時のクリーンアップ（Windowsスワイプで押しっぱなしのWinキーを解放）
void keyball_on_swipe_end(kb_swipe_tag_t tag) {
    if (tag == KBS_TAG_SW_WIN) {
        if (host_os == OS_WINDOWS) {
            unregister_code(KC_LGUI);
        }
    }
}
//------------------------------------------------------------
// タップ時の動作はここで指定する。
//------------------------------------------------------------
void keyball_on_swipe_tap(kb_swipe_tag_t tag) {
    switch (tag) {
    case KBS_TAG_SW_APP:
        tap_code16_os(G(KC_TAB), m_MIS_CON, m_MIS_CON, KC_NO, KC_NO);
        break;
    case KBS_TAG_SW_VOL:
        tap_code16_with_oneshot(KC_MPLY);
        break;
    case KBS_TAG_SW_BRO:
        {
            uint16_t now = timer_read();
            if (bro_double_wait && timer_elapsed(bro_double_timer) <= TAPPING_TERM) {
                tap_code16_os(C(KC_R), G(KC_R), G(KC_R), C(KC_R), C(KC_R));
                bro_double_wait = false;
                bro_double_timer = 0;
            } else {
                tap_code16_os(C(KC_L), G(KC_L), G(KC_L), KC_NO, KC_NO);
                bro_double_wait = true;
                bro_double_timer = now;
            }
        }
        break;
    case KBS_TAG_SW_TAB:
        tap_code16_os(C(KC_T), G(KC_T), G(KC_T), KC_NO, KC_NO);
        break;
    case KBS_TAG_SW_WIN:
        tap_code16_os(G(KC_Z), RALT(C(KC_F)), KC_NO, KC_NO, KC_NO);
        break;
    case KBS_TAG_SW_UTIL:
        tap_code16_with_oneshot(KC_ESC);
        tap_code16_with_oneshot(KC_LNG2);
        break;
    case KBS_TAG_SW_ARR:
        tap_code(KC_NO);
        break;
    /*
    // 例: 拡張スワイプキー（SW_EX1/SW_EX2）のユーザーオーバーライド
    // デフォルト（KBレベル）は、EX1: tap=F10 / UP=F11 / RIGHT=F12 / DOWN=F13 / LEFT=F14
    //                           EX2: tap=F15 / UP=F16 / RIGHT=F17 / DOWN=F18 / LEFT=F19
    // 下記のようにケースを追加すれば、ユーザー側で自由に置き換えられます。
    */
    case KBS_TAG_EX1:
        tap_code16_with_oneshot(KC_F13);
        break;
    case KBS_TAG_EX2:
        tap_code16_with_oneshot(KC_F18);
        break;
    default:
        break;
    }
}

// クールタイム指定（タグ・方向に応じてmsを返す。0=無制限）
uint16_t keyball_swipe_get_cooldown_ms(kb_swipe_tag_t tag, kb_swipe_dir_t dir) {
    switch (tag) {
    // 例1: PAD_A は全方向 300ms
    case KBS_TAG_FLICK_A:
    case KBS_TAG_FLICK_D:
    case KBS_TAG_FLICK_G:
    case KBS_TAG_FLICK_J:
    case KBS_TAG_FLICK_M:
    case KBS_TAG_FLICK_P:
    case KBS_TAG_FLICK_T:
    case KBS_TAG_FLICK_W:
    case KBS_TAG_SW_BRO:
        return 300;

    // 例2: VOL は左右のみ 500ms、上下は 0ms（連続発火OK）
    case KBS_TAG_SW_VOL:
        if (dir == KB_SWIPE_LEFT || dir == KB_SWIPE_RIGHT) return 500;
        return 0;

    default:
        return 0; // デフォルト: クールタイムなし
    }
}

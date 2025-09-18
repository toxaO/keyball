#include "quantum.h"
#include "../keycode_user.h"
#include "../features/util_user.h"
#include "print.h"
#include "os_detection.h"

#include "swipe_user.h"
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

    case KBS_TAG_ARR:
        // Arrow proxy: swipe方向に応じて矢印キーを発火
        switch (dir) {
        case KB_SWIPE_UP:    tap_code(KC_UP);    break;
        case KB_SWIPE_DOWN:  tap_code(KC_DOWN);  break;
        case KB_SWIPE_LEFT:  tap_code(KC_LEFT);  break;
        case KB_SWIPE_RIGHT: tap_code(KC_RIGHT); break;
        default: break;
        }
        break;
    /*
    // 例: 拡張スワイプキー（SW_EX1/SW_EX2）のユーザーオーバーライド
    case KBS_TAG_EX1:
        switch (dir) {
        case KB_SWIPE_UP:    tap_code16(KC_F11); break;
        case KB_SWIPE_RIGHT: tap_code16(KC_F12); break;
        case KB_SWIPE_DOWN:  tap_code16(KC_F13); break;
        case KB_SWIPE_LEFT:  tap_code16(KC_F14); break;
        default: break;
        }
        break;
    case KBS_TAG_EX2:
        switch (dir) {
        case KB_SWIPE_UP:    tap_code16(KC_F16); break;
        case KB_SWIPE_RIGHT: tap_code16(KC_F17); break;
        case KB_SWIPE_DOWN:  tap_code16(KC_F18); break;
        case KB_SWIPE_LEFT:  tap_code16(KC_F19); break;
        default: break;
        }
        break;
    */
    case KBS_TAG_PAD_A:
        // _Pad レイヤの KC_A 長押し → スワイプ方向で分岐
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16(KC_B); break;
        case KB_SWIPE_RIGHT: tap_code16(KC_C); break;
        case KB_SWIPE_UP:    tap_code16(AT);   break; // JISの記号定義に合わせて AT を使用
        default: break;
        }
        break;

    // FLICK_* 系：ベースキーごとの方向マッピング
    case KBS_TAG_FLICK_A:
        // {tap=A, up=@, right=C, left=B, down=None}
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16(KC_B); break;
        case KB_SWIPE_RIGHT: tap_code16(KC_C); break;
        case KB_SWIPE_UP:    tap_code16(AT);   break;
        default: break;
        }
        break;

    case KBS_TAG_FLICK_D:
        // {tap=D, up=(, right=F, left=E, down=)}
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16(KC_E);      break;
        case KB_SWIPE_RIGHT: tap_code16(KC_F);      break;
        case KB_SWIPE_UP:    tap_code16(L_PAREN);   break;
        case KB_SWIPE_DOWN:  tap_code16(R_PAREN);   break;
        default: break;
        }
        break;

    case KBS_TAG_FLICK_G:
        // {tap=G, up=None, right=I, left=H, down=None}
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16(KC_H); break;
        case KB_SWIPE_RIGHT: tap_code16(KC_I); break;
        default: break;
        }
        break;

    case KBS_TAG_FLICK_J:
        // {tap=J, up=None, right=L, left=K, down=None}
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16(KC_K); break;
        case KB_SWIPE_RIGHT: tap_code16(KC_L); break;
        default: break;
        }
        break;


    case KBS_TAG_FLICK_M:
        // {tap=M, up=None, right=O, left=N, down=None}
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16(KC_N); break;
        case KB_SWIPE_RIGHT: tap_code16(KC_O); break;
        default: break;
        }
        break;

    case KBS_TAG_FLICK_P:
        // {tap=P, up=None, right=R, left=Q, down=None}
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16(KC_Q); break;
        case KB_SWIPE_RIGHT: tap_code16(KC_R); break;
        default: break;
        }
        break;

    case KBS_TAG_FLICK_S:
        // {tap=S, up=None, right=U, left=T, down=None}
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16(KC_T); break;
        case KB_SWIPE_RIGHT: tap_code16(KC_U); break;
        default: break;
        }
        break;

    case KBS_TAG_FLICK_V:
        // {tap=V, up=Y, right=W, left=Z, down=None}
        switch (dir) {
        case KB_SWIPE_LEFT:  tap_code16(KC_Z); break;
        case KB_SWIPE_RIGHT: tap_code16(KC_W); break;
        case KB_SWIPE_UP:    tap_code16(KC_Y); break;
        default: break;
        }
        break;

    default:
        break;
    }
}

// セッション終了時のクリーンアップ（Windowsスワイプで押しっぱなしのWinキーを解放）
void keyball_on_swipe_end(kb_swipe_tag_t tag) {
    if (tag == KBS_TAG_WIN) {
        if (host_os == OS_WINDOWS) {
            unregister_code(KC_LGUI);
        }
    }
}

// タップ（発火なしで離した）時のフォールバックをユーザー側で上書き
void keyball_on_swipe_tap(kb_swipe_tag_t tag) {
    switch (tag) {
    case KBS_TAG_APP:
        tap_code16_os(G(KC_TAB), m_MIS_CON, m_MIS_CON, KC_NO, KC_NO);
        break;
    case KBS_TAG_VOL:
        tap_code(KC_MPLY);
        break;
    case KBS_TAG_BRO:
        tap_code16_os(C(KC_L), G(KC_L), G(KC_L), KC_NO, KC_NO);
        break;
    case KBS_TAG_TAB:
        tap_code16_os(C(KC_T), G(KC_T), G(KC_T), KC_NO, KC_NO);
        break;
    case KBS_TAG_WIN:
        tap_code16_os(G(KC_Z), A(C(KC_ENT)), A(C(KC_ENT)), KC_NO, KC_NO);
        break;
    case KBS_TAG_ARR:
        // タップ時は何もしない
        break;
    /*
    // 例: 拡張スワイプキー（SW_EX1/SW_EX2）のユーザーオーバーライド
    // デフォルト（KBレベル）は、EX1: tap=F10 / UP=F11 / RIGHT=F12 / DOWN=F13 / LEFT=F14
    //                           EX2: tap=F15 / UP=F16 / RIGHT=F17 / DOWN=F18 / LEFT=F19
    // 下記のようにケースを追加すれば、ユーザー側で自由に置き換えられます。
    case KBS_TAG_EX1:
        // tap 時の置き換え
        tap_code16(KC_F10);
        break;
    case KBS_TAG_EX2:
        tap_code16(KC_F15);
        break;
    */
    default:
        break;
    }
}

// クールタイム指定（タグ・方向に応じてmsを返す。0=無制限）
uint16_t keyball_swipe_get_cooldown_ms(kb_swipe_tag_t tag, kb_swipe_dir_t dir) {
    switch (tag) {
    // 例1: PAD_A は全方向 300ms
    case KBS_TAG_PAD_A:
        return 300;

    // 例2: VOL は左右のみ 500ms、上下は 0ms（連続発火OK）
    case KBS_TAG_VOL:
        if (dir == KB_SWIPE_LEFT || dir == KB_SWIPE_RIGHT) return 500;
        return 0;

    default:
        return 0; // デフォルト: クールタイムなし
    }
}

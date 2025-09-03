#include <stdint.h>
#include <stdbool.h>
#include "util.h"
#include "swipe.h"
#include "quantum.h"
#include "my_keycode.h"
#include "os_detection.h"
#include "swipe_tuning.h"

bool is_swiped = false;
bool canceller = false;

SwipeMode swipe_mode = NO_SW;
SwipeState state = NONE;

uint16_t click_timer;   // タイマー。状態に応じて時間で判定する。 Timer. Time to determine the state of the system.
uint16_t swipe_timer; // スワイプキーがTAPPING_TERMにあるかを判定する (≒ mod_tap)

#define ACC_SAT (1L<<28)  // 充分大きな上限

// スワイプジェスチャーで何が起こるかを実際に処理する関数
// 上、下、左、右、スワイプなしの5つのオプションがあります
// スワイプなしに関してはmacrokey.hに記載する
static inline void fire_swipe_action(SwipeDirection dir) {
    switch (swipe_mode) {
        case APP_SW:
            switch (dir) {
                case SW_UP:
                    if (canceller) { tap_code(KC_ESC); canceller = false; }
                    else { tap_code16_os(w_Ueli, G(KC_SPACE), G(KC_SPACE), KC_NO, KC_NO); canceller = true; }
                    break;
                case SW_DOWN:
                    if (canceller) { tap_code(KC_ESC); canceller = false; }
                    else { tap_code16_os(G(KC_TAB), C(KC_DOWN), C(KC_DOWN), KC_NO, KC_NO); canceller = true; }
                    break;
                case SW_LEFT:  tap_code16_os(w_R_DESK, m_R_DESK, m_R_DESK, KC_NO, KC_NO); break;
                case SW_RIGHT: tap_code16_os(w_L_DESK, m_L_DESK, m_L_DESK, KC_NO, KC_NO); break;
                default: break;
            }
            break;

        case VOL_SW:
            switch (dir) {
                case SW_UP:    tap_code(KC_VOLU); break;
                case SW_DOWN:  tap_code(KC_VOLD); break;
                case SW_LEFT:  tap_code(KC_MNXT); break;
                case SW_RIGHT: tap_code(KC_MPRV); break;
                default: break;
            }
            break;

        case BRO_SW:
            switch (dir) {
                case SW_UP:    tap_code16_os(C(KC_C), G(KC_C), G(KC_C), KC_NO, KC_NO); break;
                case SW_DOWN:  tap_code16_os(C(KC_V), G(KC_V), G(KC_V), KC_NO, KC_NO); break;
                case SW_LEFT:  tap_code16_os(KC_WBAK, G(KC_LEFT),  G(KC_LEFT),  KC_NO, KC_NO); break;
                case SW_RIGHT: tap_code16_os(KC_WFWD, G(KC_RIGHT), G(KC_RIGHT), KC_NO, KC_NO); break;
                default: break;
            }
            break;

        case TAB_SW:
            switch (dir) {
                case SW_UP:    tap_code16_os(S(C(KC_T)), S(G(KC_T)), S(G(KC_T)), KC_NO, KC_NO); break;
                case SW_DOWN:  tap_code16_os(w_CLOSE, m_CLOSE, m_CLOSE, KC_NO, KC_NO); break;
                case SW_LEFT:  tap_code16(S(C(KC_TAB))); break;
                case SW_RIGHT: tap_code16(C(KC_TAB));    break;
                default: break;
            }
            break;

        case WIN_SW:
            switch (dir) {
                case SW_UP:
                    if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_UP); }
                    else { tap_code16(MGN_U); }
                    break;
                case SW_DOWN:
                    if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_DOWN); }
                    else { tap_code16(MGN_D); }
                    break;
                case SW_LEFT:
                    if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_LEFT); }
                    else { tap_code16(MGN_L); }
                    break;
                case SW_RIGHT:
                    if (host_os == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_RIGHT); }
                    else { tap_code16(MGN_R); }
                    break;
                default: break;
            }
            break;

        default:
            break;
    }
}


report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {

    int16_t dx = mouse_report.x;
    int16_t dy = mouse_report.y;

    // 状態（関数スコープ静的）
    static SwipeDirection cur_dir = SW_NO;  // 現在選択中の「厳密な方向」
    static int32_t        accum   = 0;      // その方向への蓄積距離（正数）
    static bool           first_pending = true; // 初回発火待ち？
    static uint8_t        idle_frames   = 0;    // 停止復帰用（任意）

    // --- SWIPE → SWIPING の開始ゲート ---
    if (state == SWIPE) {
        int16_t adx = my_abs(dx), ady = my_abs(dy);
        // 厳密分類：主成分のみ採用（= 一番大きく動いた方向）
        if (adx >= ady) {
            cur_dir = (dx >= 0) ? SW_RIGHT : SW_LEFT;
        } else {
            cur_dir = (dy >= 0) ? SW_DOWN  : SW_UP;
        }
        accum = 0;
        first_pending = true;
        state = SWIPING;
    }

    if (state == SWIPING) {
        int fires = 0;

        // そのフレームの主成分と方向を厳密分類
        int16_t adx = my_abs(dx), ady = my_abs(dy);
        SwipeDirection new_dir;
        int16_t dom = 0;
        if (adx >= ady) {
            new_dir = (dx >= 0) ? SW_RIGHT : SW_LEFT;
            dom     = adx;
        } else {
            new_dir = (dy >= 0) ? SW_DOWN  : SW_UP;
            dom     = ady;
        }

        // 方向が切り替わったら蓄積を0に（他方向は常に0扱い）
        if (new_dir != cur_dir) {
            cur_dir = new_dir;
            accum   = 0;
            // 初回/再初回の概念は維持（first_pending は直前の発火でのみ false になる）
        }

        // 選ばれた方向“だけ”に距離を蓄積（他方向は0）
        // ※ 純粋な距離として正のみに積む
        int64_t tmp = (int64_t)accum + (int64_t)dom;
        if (tmp > ACC_SAT) tmp = ACC_SAT;
        accum = (int32_t)tmp;

        // 閾値（初回だけ FIRST、その後は STEP）
        int16_t th = first_pending ? FIRST_STEP_COUNT : STEP_COUNT;

        // 超えるたびに発火、余剰は持ち越し（ただし1スキャン上限あり）
        while (accum >= th) {
            switch (cur_dir) {
                case SW_RIGHT: fire_swipe_action(SW_RIGHT); break;
                case SW_LEFT:  fire_swipe_action(SW_LEFT);  break;
                case SW_DOWN:  fire_swipe_action(SW_DOWN);  break;
                case SW_UP:    fire_swipe_action(SW_UP);    break;
                default: break;
            }
            // accum -= th;
            accum = 0;
            fires++;
            first_pending = false;  // 以後は STEP_COUNT 基準
        }

        // 停止検出（任意）：完全停止が続いたらSWIPEへ戻す
        if (dx == 0 && dy == 0) {
            if (idle_frames < 255) idle_frames++;
            if (idle_frames >= 2 && accum == 0) {
                state = SWIPE;
                cur_dir = SW_NO;
                first_pending = true;
                idle_frames = 0;
            }
        } else {
            idle_frames = 0;
        }
    }

    // スワイプ中はポインタ停止（必要なら外す）
    if (state == SWIPE || state == SWIPING) {
        mouse_report.x = 0;
        mouse_report.y = 0;
    }
    return mouse_report;

}

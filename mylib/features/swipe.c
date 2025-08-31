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


// === 軸ヒステリシス & ロック ===
typedef enum { AXIS_NONE=0, AXIS_H, AXIS_V } SwipeAxis;

#define ACC_SAT (1L<<28)  // 充分大きな上限

// 軸状態
static SwipeAxis current_axis = AXIS_NONE;
static uint8_t   axis_switch_run = 0;       // 連続優勢カウント
static uint8_t   axis_lock_frames = 0;   // 発火後ロック残り


// 追加：ローパスと累積（符号付き）
static int32_t lp_x = 0, lp_y = 0;    // ローパス用の内部状態
static int32_t accum_x = 0, accum_y = 0;  // 発火用の距離累積（正負で左右/上下）
static int8_t    dir_x = 0, dir_y = 0;  // 積算の符号記憶（往復稼ぎ防止）

static inline void saturating_add(int32_t *acc, int32_t delta) {
    int64_t t = (int64_t)(*acc) + delta;
    if (t >  ACC_SAT) t =  ACC_SAT;
    if (t < -ACC_SAT) t = -ACC_SAT;
    *acc = (int32_t)t;
}

static inline void accumulate_with_dir_guard(int32_t *acc, int16_t delta, int8_t *last_dir) {
    int8_t d = (delta > 0) - (delta < 0);
    if (d != 0 && d != *last_dir) { *acc = 0; *last_dir = d; }
    saturating_add(acc, delta);
}

static inline SwipeAxis dominant_axis(int16_t x, int16_t y) {
    int32_t ax = my_abs(x), ay = my_abs(y);
    if (ax == 0 && ay == 0) return AXIS_NONE;
    if (ax > 0 && ay * 100 <= ax * AXIS_H_RATIO_X100) return AXIS_H;
    if (ax > 0 && ay * 100 >= ax * AXIS_V_RATIO_X100) return AXIS_V;
    return AXIS_NONE; // 曖昧
}

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

    // デッドゾーン
    if (my_abs(dx) < DEADZONE) dx = 0;
    if (my_abs(dy) < DEADZONE) dy = 0;

    // ローパス（単一の係数でOK）
    lp_x += ((int32_t)dx - (lp_x >> LP_SHIFT));
    lp_y += ((int32_t)dy - (lp_y >> LP_SHIFT));
    int16_t fdx = (int16_t)(lp_x >> LP_SHIFT);
    int16_t fdy = (int16_t)(lp_y >> LP_SHIFT);

    // 起動判定（X/Y共通しきい値）
    if (state == SWIPE) {
        if (my_abs(fdx) >= START_COUNT || my_abs(fdy) >= START_COUNT) {
            accum_x = accum_y = 0;
            dir_x = dir_y = 0;
            state = SWIPING;
        }
    }

    // 距離累積 & 発火（X/Y同じSTEP_COUNT）
    if (state == SWIPING) {
        int fires = 0;
        int16_t step = STEP_COUNT;

        // --- 軸の決定（ヒステリシス & 発火後ロック） ---
        SwipeAxis want = dominant_axis(fdx, fdy);

        if (axis_lock_frames > 0) {
            // ロック中は軸固定＆直交軸を減衰
            if (current_axis == AXIS_H) accum_y >>= ORTHO_DECAY_SHIFT;
            else if (current_axis == AXIS_V) accum_x >>= ORTHO_DECAY_SHIFT;
            axis_lock_frames--;
        } else {
            if (current_axis == AXIS_NONE) {
                if (want != AXIS_NONE) { current_axis = want; axis_switch_run = 0; accum_x = accum_y = 0; dir_x = dir_y = 0; }
            } else if (want == current_axis || want == AXIS_NONE) {
                axis_switch_run = 0; // 変わらない/曖昧ならカウントリセット
            } else {
                if (++axis_switch_run >= AXIS_SWITCH_HYST) {
                    current_axis = want; axis_switch_run = 0;
                    accum_x = accum_y = 0; dir_x = dir_y = 0; // 切替時は前軸の残留を捨てる
                }
            }
        }

        // --- 累積（現在の軸のみ本加算、直交軸は減衰） ---
        if (current_axis == AXIS_H) {
            accumulate_with_dir_guard(&accum_x, fdx, &dir_x);
            accum_y >>= ORTHO_DECAY_SHIFT;
        } else if (current_axis == AXIS_V) {
            accumulate_with_dir_guard(&accum_y, fdy, &dir_y);
            accum_x >>= ORTHO_DECAY_SHIFT;
        } else { // 未決定なら両方少しずつ
            accum_x += fdx; accum_y += fdy;
        }

        // ---- 発火（単発寄り: マージン & ゼロクリア）----
        // X（右/左）
        if (fires < MAX_FIRES_PER_SCAN && accum_x >= (step + FIRE_MARGIN_COUNT)) {
            fire_swipe_action(SW_RIGHT);
            accum_x = 0; fires++; axis_lock_frames = POST_FIRE_LOCK_FRAMES; current_axis = AXIS_H;
        } else if (fires < MAX_FIRES_PER_SCAN && accum_x <= -(step + FIRE_MARGIN_COUNT)) {
            fire_swipe_action(SW_LEFT);
            accum_x = 0; fires++; axis_lock_frames = POST_FIRE_LOCK_FRAMES; current_axis = AXIS_H;
        }

        // Y（下/上）
        if (fires < MAX_FIRES_PER_SCAN && accum_y >= (step + FIRE_MARGIN_COUNT)) {
            fire_swipe_action(SW_DOWN);
            accum_y = 0; fires++; axis_lock_frames = POST_FIRE_LOCK_FRAMES; current_axis = AXIS_V;
        } else if (fires < MAX_FIRES_PER_SCAN && accum_y <= -(step + FIRE_MARGIN_COUNT)) {
            fire_swipe_action(SW_UP);
            accum_y = 0; fires++; axis_lock_frames = POST_FIRE_LOCK_FRAMES; current_axis = AXIS_V;
        }

        // ---- 停止判定 ----
        if (fdx == 0 && fdy == 0 && my_abs(accum_x) < step/2 && my_abs(accum_y) < step/2) {
            accum_x /= 2; accum_y /= 2;
            if (accum_x == 0 && accum_y == 0) { current_axis = AXIS_NONE; state = SWIPE; axis_switch_run = 0; axis_lock_frames = 0; }
        }
    }

    // スワイプ中はポインタ停止（必要なら外してOK）
    if (state == SWIPE || state == SWIPING) {
        mouse_report.x = 0;
        mouse_report.y = 0;
    }
    return mouse_report;
}

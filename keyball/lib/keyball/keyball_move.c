#include <stdint.h>
#include "timer.h"
#include "keyball.h"
#include "keyball_move.h"

#define _CONSTRAIN(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

static inline int16_t clamp_xy(int16_t v) {
  return (int16_t)_CONSTRAIN(v, MOUSE_REPORT_XY_MIN, MOUSE_REPORT_XY_MAX);
}

int32_t g_move_gain_lo_fp = KEYBALL_MOVE_GAIN_LO_FP;
int16_t g_move_th1 = KEYBALL_MOVE_TH1;
int16_t g_move_th2 = KEYBALL_MOVE_TH2;

void keyball_on_apply_motion_to_mouse_move(report_mouse_t *report,
                                           report_mouse_t *output,
                                           bool is_left) {
#if KEYBALL_MOVE_SHAPING_ENABLE
    // 32bit蓄積（商/余り用）
    static int32_t acc_x = 0, acc_y = 0;
    static int16_t last_sx = 0, last_sy = 0;
    static uint32_t last_ts = 0;

    int16_t sx = (int16_t)report->x;
    int16_t sy = (int16_t)report->y;

    // 小さなノイズを抑えるための円形デッドゾーン（kbpf.move_deadzone）
    if (kbpf.move_deadzone) {
      int32_t ax = (int32_t)(sx < 0 ? -sx : sx);
      int32_t ay = (int32_t)(sy < 0 ? -sy : sy);
      int32_t dz = (int32_t)kbpf.move_deadzone;
      int32_t radius_sq = dz * dz;
      int32_t mag_sq = ax * ax + ay * ay;
      if (mag_sq <= radius_sq) {
        sx = 0;
        sy = 0;
      }
    }

    // アイドル・方向反転で蓄積を捨てる（跳ね防止）
    uint32_t now = timer_read32();
    if (TIMER_DIFF_32(now, last_ts) > KEYBALL_MOVE_IDLE_RESET_MS) {
      acc_x = acc_y = 0;
    }
    if (sx && last_sx && ((sx ^ last_sx) < 0)) acc_x = 0;
    if (sy && last_sy && ((sy ^ last_sy) < 0)) acc_y = 0;
    last_sx = sx; last_sy = sy;
    last_ts = now;

    // 速度近似（高コストなsqrt回避）
    int16_t ax = (sx < 0 ? (int16_t)-sx : sx);
    int16_t ay = (sy < 0 ? (int16_t)-sy : sy);
    int16_t mag = 0;
    if (ax || ay) {
      int16_t max_comp = (ax > ay) ? ax : ay;
      int16_t min_comp = (ax > ay) ? ay : ax;
      // 斜め方向でも滑らかになるように L1/L2 の妥協値を採用
      mag = (int16_t)(max_comp + (min_comp >> 1));
      if (mag < max_comp) {
        mag = max_comp;
      }
    }

    // ゲイン算出（固定小数点）
    int32_t g_lo = g_move_gain_lo_fp;  // 例: 64
    int32_t g_hi = KEYBALL_MOVE_GAIN_HI_FP;  // 例: 256
    int32_t gain_fp;

    if (mag <= g_move_th1) {
      gain_fp = g_lo;
    } else if (mag >= g_move_th2) {
      gain_fp = g_hi;
    } else {
      // 線形補間
      int32_t num = (int32_t)(mag - g_move_th1);
      int32_t den = (int32_t)(g_move_th2 - g_move_th1);
      if (den < 1) den = 1; // 保険
      gain_fp = g_lo + ((g_hi - g_lo) * num) / den;
    }

    // 固定小数点で適用（商/余り）
    int32_t gain_fp_x = gain_fp;
    int32_t gain_fp_y = gain_fp;

#ifdef KEYBALL_MOVE_AXIS_GAIN_X_FP
    gain_fp_x = (gain_fp_x * KEYBALL_MOVE_AXIS_GAIN_X_FP + (KMF_DEN / 2)) / KMF_DEN;
#endif
#ifdef KEYBALL_MOVE_AXIS_GAIN_Y_FP
    gain_fp_y = (gain_fp_y * KEYBALL_MOVE_AXIS_GAIN_Y_FP + (KMF_DEN / 2)) / KMF_DEN;
#endif

    acc_x += (int32_t)sx * gain_fp_x;
    acc_y += (int32_t)sy * gain_fp_y;

    int16_t out_x = (int16_t)(acc_x / KMF_DEN);
    int16_t out_y = (int16_t)(acc_y / KMF_DEN);

    acc_x -= (int32_t)out_x * KMF_DEN;
    acc_y -= (int32_t)out_y * KMF_DEN;

    // クランプして反映
    output->x = (int8_t)clamp_xy(out_x);
    output->y = (int8_t)clamp_xy(out_y);

    // 左右で「移動」は反転しない（従来のscrollとは別）
    (void)is_left;
#else
    // 旧仕様：そのまま
    output->x = report->x;
    output->y = report->y;
#endif
}

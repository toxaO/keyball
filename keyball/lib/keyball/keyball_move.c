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

void keyball_on_apply_motion_to_mouse_move(report_mouse_t *report,
                                           report_mouse_t *output,
                                           bool is_left) {
#if KEYBALL_MOVE_SHAPING_ENABLE
    // 32bit蓄積（商/余り用）
    static int32_t acc_x = 0, acc_y = 0;
    static uint8_t last_sx = 0, last_sy = 0;
    static uint32_t last_ts = 0;

    int16_t sx = (int16_t)report->x;
    int16_t sy = (int16_t)report->y;

    // アイドル・方向反転で蓄積を捨てる（跳ね防止）
    uint32_t now = timer_read32();
    if (TIMER_DIFF_32(now, last_ts) > KEYBALL_MOVE_IDLE_RESET_MS) {
      acc_x = acc_y = 0;
    }
    if ((int8_t)sx && (int8_t)last_sx && ((sx ^ last_sx) < 0)) acc_x = 0;
    if ((int8_t)sy && (int8_t)last_sy && ((sy ^ last_sy) < 0)) acc_y = 0;
    last_sx = (uint8_t)sx; last_sy = (uint8_t)sy;
    last_ts = now;

    // 速度近似（高コストなsqrt回避）
    int16_t ax = (sx < 0 ? -sx : sx);
    int16_t ay = (sy < 0 ? -sy : sy);
    int16_t mag = (ax > ay) ? ax : ay;

    // ゲイン算出（固定小数点）
    int32_t g_lo = g_move_gain_lo_fp;  // 例: 64
    int32_t g_hi = KEYBALL_MOVE_GAIN_HI_FP;  // 例: 256
    int32_t gain_fp;

    if (mag <= g_move_th1) {
      gain_fp = g_lo;
    } else if (mag >= KEYBALL_MOVE_TH2) {
      gain_fp = g_hi;
    } else {
      // 線形補間
      int32_t num = (int32_t)(mag - g_move_th1);
      int32_t den = (int32_t)(KEYBALL_MOVE_TH2 - g_move_th1);
      if (den < 1) den = 1; // 保険
      gain_fp = g_lo + ((g_hi - g_lo) * num) / den;
    }

    // 固定小数点で適用（商/余り）
    acc_x += (int32_t)sx * gain_fp;
    acc_y += (int32_t)sy * gain_fp;

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

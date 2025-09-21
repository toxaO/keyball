#include "keyball_scroll.h"
#include "keyball.h"
#include "os_detection.h"
#include "quantum.h"
#include "timer.h"
#include <stdint.h>
#include <stdlib.h>

#define _CONSTRAIN(amt, low, high)                                             \
  ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
#define CONSTRAIN_HV(val)                                                      \
  (mouse_hv_report_t) _CONSTRAIN(val, MOUSE_REPORT_HV_MIN, MOUSE_REPORT_HV_MAX)

#define array_size(a) (sizeof(a) / sizeof((a)[0]))

extern const uint8_t SCROLL_DIV_MAX;

static inline uint8_t clamp_sdiv(uint8_t v) {
  if (v > SCROLL_DIV_MAX)
    v = SCROLL_DIV_MAX;
  return v;
}

// deadzone/hysteresis は廃止（互換のため定義自体は削除）

// Debug info ---------------------------------------------------------------
static int16_t g_dbg_sx = 0, g_dbg_sy = 0; // raw scroll input after filters
static int16_t g_dbg_h = 0, g_dbg_v = 0;   // final output values
static int32_t g_dbg_acc_h = 0, g_dbg_acc_v = 0;   // accumulated values
static int8_t  g_dbg_t = 0; // 直行方向への強さ

// 12/10 ≒ 1.2 倍を整数近似するための補助関数
static inline uint32_t kb_mul12_div10(uint32_t v) { return (uint32_t)((v * 12u + 5u) / 10u); }

void keyball_scroll_get_dbg(int16_t *sx, int16_t *sy, int16_t *h, int16_t *v) {
  if (sx)
    *sx = g_dbg_sx;
  if (sy)
    *sy = g_dbg_sy;
  if (h)
    *h = g_dbg_h;
  if (v)
    *v = g_dbg_v;
}

void keyball_scroll_get_dbg_inner(int32_t *ah, int32_t *av, int8_t *t) {
  if (ah)
    *ah = g_dbg_acc_h;
  if (av)
    *av = g_dbg_acc_v;
  if (t)
    *t = g_dbg_t;
}

bool keyball_get_scroll_mode(void) { return keyball.scroll_mode; }

void keyball_set_scroll_mode(bool mode) {
  if (mode != keyball.scroll_mode) {
    keyball.scroll_mode_changed = timer_read32();
  }
  keyball.scroll_mode = mode;
}

keyball_scrollsnap_mode_t keyball_get_scrollsnap_mode(void) {
#if KEYBALL_SCROLLSNAP_ENABLE == 2
  return keyball.scrollsnap_mode;
#else
  return KEYBALL_SCROLLSNAP_MODE_FREE;
#endif
}

void keyball_set_scrollsnap_mode(keyball_scrollsnap_mode_t mode) {
#if KEYBALL_SCROLLSNAP_ENABLE == 2
  keyball.scrollsnap_mode = mode;
#endif
}

uint8_t keyball_get_scroll_div(void) {
  return clamp_sdiv(kbpf.scroll_step[keyball_os_idx()]);
}

void keyball_set_scroll_div(uint8_t div) {
  div = clamp_sdiv(div);
  uint8_t i = keyball_os_idx();
  kbpf.scroll_step[i] = div;
  dprintf("keyball: scroll_step set OS=%u -> %u\n", i, div);
}

void keyball_on_apply_motion_to_mouse_scroll(report_mouse_t *report,
                                             report_mouse_t *output,
                                             bool is_left) {
  // 新ロジック：
  // - 対角線付近(0.5..2.0倍)の比率では出力を抑制
  // - 主成分(横/縦)のみ蓄積してしきい値到達で出力
  // - sdiv は「スクロール感度レベル(SL)」として interval/value に乗算スケール
  // - OS 分岐は行わず、パラメータは kbpf に OS 別保存
  // - デッドゾーン／ヒステリシスは今回未使用（パラメータのみ維持）

  static int32_t acc_h = 0, acc_v = 0; // 蓄積（高分解能用）
  static int16_t prev_x = 0, prev_y = 0;
  static uint32_t last_ts = 0;

  int16_t sx = (int16_t)report->x;
  int16_t sy = (int16_t)report->y;
  // デッドゾーン適用（小ノイズ抑制）
  {
    uint8_t dz = kbpf.scroll_deadzone;
    int16_t ax = (sx < 0 ? -sx : sx);
    int16_t ay = (sy < 0 ? -sy : sy);
    if (ax <= (int16_t)dz) sx = 0;
    if (ay <= (int16_t)dz) sy = 0;
  }
  g_dbg_sx = sx;
  g_dbg_sy = sy;

  // アイドルで蓄積をリセット
  uint32_t now = timer_read32();
  if (TIMER_DIFF_32(now, last_ts) > KEYBALL_SCROLL_IDLE_RESET_MS) {
    acc_h = 0;
    acc_v = 0;
  }
  last_ts = now;

  // ST(スクロールステップ) による強いスケーリング（中心=4）。
  // f = num/den。f が大きいほど速く、小さいほど遅い。
  // ST=1..7 に対して: [1/8, 1/4, 1/2, 1, 2, 4, 8]
  static const uint16_t st_num[] = {1, 1, 1, 1, 2, 4, 8};
  static const uint16_t st_den[] = {8, 4, 2, 1, 1, 1, 1};
  uint8_t st = keyball_get_scroll_div();
  if (st == 0) st = 1; // 0 は最小と同等
  if (st > 7) st = 7;
  uint16_t f_num = st_num[st - 1];
  uint16_t f_den = st_den[st - 1];

  // OS 分岐はしない。OS 別の値は kbpf のスロット差し替えにより表現
  uint8_t os = keyball_os_idx();
  uint16_t base_interval = kbpf.scroll_interval[os]; // 1..200 程度
  uint16_t base_value    = kbpf.scroll_value[os];    // 1..200 程度
  if (base_interval < 1) base_interval = 1;
  if (base_value < 1)    base_value    = 1;

  // 実効値（ST によりベース値を倍率変換）
  // 速くする＝しきい値/分母を小さくするため、interval/value を 1/f で縮める
  uint32_t eff_interval = ((uint32_t)base_interval * f_den + (f_num/2)) / f_num;
  if (eff_interval < 1) eff_interval = 1;
  uint32_t eff_value    = ((uint32_t)base_value    * f_den + (f_num/2)) / f_num;
  if (eff_value < 1) eff_value = 1;

  uint32_t thr_iv  = kb_mul12_div10(eff_interval); // 発火しきい値
  uint32_t denom_v = kb_mul12_div10(eff_value);    // 出力の分母
  if (denom_v == 0) denom_v = 1;

  // 主成分選択 / スクロールスナップ適用（有効モードを先に決定 → それに基づき蓄積）
  keyball_scrollsnap_mode_t snap_base = keyball_get_scrollsnap_mode();
  static uint8_t tension = 0;
  static uint32_t snap_free_until = 0; // timestamp when temporary FREE ends
  int32_t absx = (sx >= 0) ? sx : -sx;
  int32_t absy = (sy >= 0) ? sy : -sy;

  // テンション（直交移動量の蓄積）
  if (absx == 0 && absy == 0) {
    if (tension > 0) tension--; // アイドルで減衰
  } else {
    uint8_t inc = 0;
    if (snap_base == KEYBALL_SCROLLSNAP_MODE_VERTICAL) {
      inc = (absx > 10) ? 10 : (uint8_t)absx;
    } else if (snap_base == KEYBALL_SCROLLSNAP_MODE_HORIZONTAL) {
      inc = (absy > 10) ? 10 : (uint8_t)absy;
    }
    if (inc > 0) {
      uint16_t nt = (uint16_t)tension + inc;
      tension = (nt > 255) ? 255 : (uint8_t)nt;
    } else if (tension > 0) {
      tension--;
    }
  }

  // このフレームの適用モード（snap_eff）を決定
  keyball_scrollsnap_mode_t snap_eff = snap_base;
  if (snap_base != KEYBALL_SCROLLSNAP_MODE_FREE) {
    if (kbpf.scrollsnap_thr == 0) {
      snap_eff = KEYBALL_SCROLLSNAP_MODE_FREE; // 常時FREE指定
    } else if (snap_free_until && TIMER_DIFF_32(now, snap_free_until) < (uint32_t)kbpf.scrollsnap_rst_ms) {
      snap_eff = KEYBALL_SCROLLSNAP_MODE_FREE; // FREE継続期間
    } else if (tension >= kbpf.scrollsnap_thr) {
      snap_eff = KEYBALL_SCROLLSNAP_MODE_FREE; // 閾値到達でFREEへ遷移
      snap_free_until = now;
      tension = 0;
    } else {
      snap_free_until = 0; // ウィンドウ終了
    }
  } else {
    snap_eff = KEYBALL_SCROLLSNAP_MODE_FREE;
  }

  // 蓄積（snap_effに応じて）
  if (snap_eff == KEYBALL_SCROLLSNAP_MODE_VERTICAL) {
    if ((acc_v > 0 && sy < 0) || (acc_v < 0 && sy > 0)) acc_v = 0;
    acc_v += sy;
  } else if (snap_eff == KEYBALL_SCROLLSNAP_MODE_HORIZONTAL) {
    if ((acc_h > 0 && sx < 0) || (acc_h < 0 && sx > 0)) acc_h = 0;
    acc_h += sx;
  } else { // FREE
    if ((acc_v > 0 && sy < 0) || (acc_v < 0 && sy > 0)) acc_v = 0;
    if ((acc_h > 0 && sx < 0) || (acc_h < 0 && sx > 0)) acc_h = 0;
    acc_v += sy;
    acc_h += sx;
  }

  g_dbg_acc_h = acc_h;
  g_dbg_acc_v = acc_v;

  int16_t out_h = 0, out_v = 0;
  // しきい値超過時に出力し、蓄積はフラッシュ
  if ((uint32_t)((acc_v >= 0) ? acc_v : -acc_v) >= thr_iv) {
    int32_t emit = acc_v / (int32_t)denom_v; // 比例出力
    // 追加ゲイン: emit に f を乗算（丸め込み）
    emit = (int32_t)((emit * (int32_t)f_num + (int32_t)(f_den / 2)) / (int32_t)f_den);
    if (emit != 0) {
      out_v = (int16_t)emit;
      acc_v = 0;
    }
  }
  if ((uint32_t)((acc_h >= 0) ? acc_h : -acc_h) >= thr_iv) {
    int32_t emit = acc_h / (int32_t)denom_v;
    emit = (int32_t)((emit * (int32_t)f_num + (int32_t)(f_den / 2)) / (int32_t)f_den);
    if (emit != 0) {
      out_h = (int16_t)emit;
      acc_h = 0;
    }
  }

  // 個別ゲイン（最終段の微調整用）
  if (out_h != 0) {
    // 水平方向の最終ゲインは、垂直ゲインを基準にパーセントで調整
    // eff_hor_gain_fp = VER_GAIN_FP * (pct/100)
    uint32_t pct = kbpf.scroll_hor_gain_pct ? kbpf.scroll_hor_gain_pct : 100u; // guard
    int32_t eff_hor_gain_fp = (int32_t)(((uint32_t)KEYBALL_SCROLL_VER_GAIN_FP * pct + 50u) / 100u);
    int32_t tmp = (int32_t)out_h * eff_hor_gain_fp;
    out_h = (int16_t)((tmp + (tmp >= 0 ? 128 : -128)) / 256);
  }
  if (out_v != 0) {
    int32_t tmp = (int32_t)out_v * (int32_t)KEYBALL_SCROLL_VER_GAIN_FP;
    out_v = (int16_t)((tmp + (tmp >= 0 ? 128 : -128)) / 256);
  }

  output->h = -CONSTRAIN_HV(out_h);
  output->v = CONSTRAIN_HV(out_v);

  // invert（左右/設定に応じた反転）
  if (is_left) {
    output->h = -output->h;
    output->v = -output->v;
  } else if (kbpf.scroll_invert[keyball_os_idx()]) {
    output->h = -output->h;
    output->v = -output->v;
  }

  // スクロールモード時の反転（既存仕様を踏襲）
  if (keyball.scroll_mode) {
    output->h = -output->h;
    output->v = -output->v;
  }

  g_dbg_h = output->h;
  g_dbg_v = output->v;
  g_dbg_t = tension;
  (void)prev_x; (void)prev_y; // 現状未使用（将来のフィルタ用に残置）
}

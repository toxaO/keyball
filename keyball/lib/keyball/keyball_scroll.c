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

uint8_t g_scroll_deadzone = KB_SCROLL_DEADZONE;
uint8_t g_scroll_hysteresis = KB_SCROLL_HYST;

// Debug info ---------------------------------------------------------------
static int16_t g_dbg_sx = 0, g_dbg_sy = 0; // raw scroll input after filters
static int16_t g_dbg_h = 0, g_dbg_v = 0;   // final output values

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
  return clamp_sdiv(kbpf.sdiv[keyball_os_idx()]);
}

void keyball_set_scroll_div(uint8_t div) {
  div = clamp_sdiv(div);
  uint8_t i = keyball_os_idx();
  kbpf.sdiv[i] = div;
  dprintf("keyball: sdiv set OS=%u -> %u\n", i, div);
}

void keyball_on_apply_motion_to_mouse_scroll(report_mouse_t *report,
                                             report_mouse_t *output,
                                             bool is_left) {
  int16_t out_x = 0;
  int16_t out_y = 0;

  // 32bitにして余裕を持たせる（高速回転や高CPIでのあふれ対策）
  static int32_t acc_x_mac = 0, acc_y_mac = 0;
  static int32_t acc_x_gen = 0, acc_y_gen = 0;
  static uint8_t last_sdiv = 0;
  static uint32_t last_ts = 0;
  static int8_t last_dir_x = 0, last_dir_y = 0;

  uint32_t now = timer_read32();
  int16_t sx = (int16_t)report->x;
  int16_t sy = (int16_t)report->y;
  uint8_t sdiv = keyball_get_scroll_div();

  // デッドゾーンとヒステリシスを適用するか
  bool disable_filters = (g_scroll_deadzone == 0 && g_scroll_hysteresis == 0);

  // デッドゾーン適用
  if (!disable_filters) {
    if (abs(sx) <= g_scroll_deadzone)
      sx = 0;
    if (abs(sy) <= g_scroll_deadzone)
      sy = 0;
  }

  // ヒステリシス処理（方向反転のゆらぎ抑制）
  int8_t dir_x = (sx > 0) - (sx < 0);
  int8_t dir_y = (sy > 0) - (sy < 0);
  if (!disable_filters) {
    if (dir_x && dir_x != last_dir_x) {
      if (last_dir_x && abs(sx) <= g_scroll_hysteresis) {
        sx = 0;
        dir_x = 0;
      } else {
        acc_x_mac = acc_x_gen = 0;
        last_dir_x = dir_x;
      }
    } else if (dir_x) {
      last_dir_x = dir_x;
    }
    if (dir_y && dir_y != last_dir_y) {
      if (last_dir_y && abs(sy) <= g_scroll_hysteresis) {
        sy = 0;
        dir_y = 0;
      } else {
        acc_y_mac = acc_y_gen = 0;
        last_dir_y = dir_y;
      }
    } else if (dir_y) {
      last_dir_y = dir_y;
    }
  } else {
    last_dir_x = dir_x;
    last_dir_y = dir_y;
  }

  g_dbg_sx = sx;
  g_dbg_sy = sy;

  // 感度変更やアイドルで余りリセット
  if (sdiv != last_sdiv ||
      TIMER_DIFF_32(now, last_ts) > KEYBALL_SCROLL_IDLE_RESET_MS) {
    acc_x_mac = acc_y_mac = 0;
    acc_x_gen = acc_y_gen = 0;
    last_sdiv = sdiv;
    last_dir_x = last_dir_y = 0;
  }
  last_ts = now;

  switch (detected_host_os()) {
  case OS_MACOS: {
    // Increase the number of steps slightly to widen the adjustment range
    // (adjust to your preference)
    static const uint16_t mac_div[] = {64, 48, 32, 24, 16, 12, 8, 6, 4, 3, 2, 1};
    uint8_t idx = sdiv;
    if (idx >= array_size(mac_div))
      idx = array_size(mac_div) - 1;
    uint16_t sdiv_mac = mac_div[idx];
    // Accumulate raw deltas but drop any fractional remainder so that the
    // scroll does not "charge" before moving (i.e. avoid high-resolution
    // behaviour on macOS).

    acc_x_mac += sx;
    acc_y_mac += sy;
    if (sdiv_mac) {
      if (acc_x_mac >= sdiv_mac) {
        out_x = 1;
        acc_x_mac = 0;
      } else if (acc_x_mac <= -(int32_t)sdiv_mac) {
        out_x = -1;
        acc_x_mac = 0;
      }
      if (acc_y_mac >= sdiv_mac) {
        out_y = 1;
        acc_y_mac = 0;
      } else if (acc_y_mac <= -(int32_t)sdiv_mac) {
        out_y = -1;
        acc_y_mac = 0;
      }
    }
  } break;
  default: {
    int16_t sdiv_gen = (int16_t)(KEYBALL_SCROLL_FINE_DEN << sdiv);
    acc_x_gen += sx;
    acc_y_gen += sy;
    if (sdiv_gen) {
      out_x = (int16_t)(acc_x_gen / sdiv_gen);
    }
    if (sdiv_gen) {
      out_y = (int16_t)(acc_y_gen / sdiv_gen);
    }
    acc_x_gen -= (int32_t)out_x * sdiv_gen;
    acc_y_gen -= (int32_t)out_y * sdiv_gen;
  }
  }

  // スナップスクロール関連
#if KEYBALL_SCROLLSNAP_ENABLE == 1
  // 「初速」っぽい調整
  if (keyball.scroll_snap_last == 0) {
    keyball.scroll_snap_tension_h = out_y;
    keyball.scroll_snap_last = now;
  } else if (TIMER_DIFF_32(now, keyball.scroll_snap_last) >=
             KEYBALL_SCROLLSNAP_RESET_TIMER) {
    keyball.scroll_snap_last = 0;
    keyball.scroll_snap_tension_h = 0;
  } else if (abs(keyball.scroll_snap_tension_h) <
             KEYBALL_SCROLLSNAP_TENSION_THRESHOLD) {
    keyball.scroll_snap_tension_h += out_y;
    out_y = 0;
  }
#elif KEYBALL_SCROLLSNAP_ENABLE == 2
  switch (keyball_get_scrollsnap_mode()) {
  case KEYBALL_SCROLLSNAP_MODE_HORIZONTAL:
    out_y = 0;
    break;
  case KEYBALL_SCROLLSNAP_MODE_VERTICAL:
    out_x = 0;
    break;
  default:
    break;
  }
#endif

  output->h = -CONSTRAIN_HV(out_x);
  output->v = CONSTRAIN_HV(out_y);

  // invert
  if (is_left) {
    output->h = -output->h;
    output->v = -output->v;
  } else if (kbpf.inv[keyball_os_idx()]) {
    output->h = -output->h;
    output->v = -output->v;
  }

  // macOS rounds to ±1 per report and chops it up finely (leaving
  // acceleration to the OS)
  if (detected_host_os() == OS_MACOS) {
    output->h = (output->h > 0) - (output->h < 0); // -1,0,+1
    output->v = (output->v > 0) - (output->v < 0);
  }
  if (output->h == 0 && output->v == 0) {
    acc_x_mac = acc_y_mac = 0;
    acc_x_gen = acc_y_gen = 0;
  }

  // invert v
  if (keyball.scroll_mode) {
    output->h = -output->h;
    output->v = -output->v;
  }

  g_dbg_h = output->h;
  g_dbg_v = output->v;
}

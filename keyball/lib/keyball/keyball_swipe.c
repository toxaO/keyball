#include <stdint.h>
#include "quantum.h"
#include "print.h"
#include "timer.h"

#include "keyball.h"
#include "keyball_swipe.h"
#ifdef HAPTIC_ENABLE
#    include "haptic.h"
#    ifdef HAPTIC_DRV2605L
#        include "drivers/haptic/drv2605l.h"
#    endif
#    if defined(SPLIT_KEYBOARD) && defined(SPLIT_HAPTIC_ENABLE)
extern uint8_t split_haptic_play;
#    endif
static uint32_t s_swipe_haptic_last_ms = 0;
static bool     s_swipe_haptic_next_primary = true;

void keyball_swipe_haptic_pulse(void) {
  if (!haptic_get_enable()) {
    s_swipe_haptic_last_ms      = 0;
    s_swipe_haptic_next_primary = true;
    return;
  }

  uint32_t now = timer_read32();
  uint16_t idle_ms = kbpf.swipe_haptic_idle_ms;
  if (s_swipe_haptic_last_ms == 0) {
    s_swipe_haptic_next_primary = true;
  } else if (timer_elapsed32(s_swipe_haptic_last_ms) >= idle_ms) {
    s_swipe_haptic_next_primary = true;
  }

#if defined(HAPTIC_DRV2605L)
  uint8_t primary = kbpf.swipe_haptic_mode;
  if (primary < 1u || primary >= (uint8_t)DRV2605L_EFFECT_COUNT) {
    primary = DRV2605L_DEFAULT_MODE;
  }
  uint8_t repeat = kbpf.swipe_haptic_mode_repeat;
  if (repeat < 1u || repeat >= (uint8_t)DRV2605L_EFFECT_COUNT) {
    repeat = primary;
  }
  uint8_t effect = s_swipe_haptic_next_primary ? primary : repeat;
  drv2605l_pulse(effect);
#    if defined(SPLIT_KEYBOARD) && defined(SPLIT_HAPTIC_ENABLE)
  split_haptic_play = effect;
#    endif
  keyball_request_remote_haptic(effect);
#else
  // フォールバック: 別ドライバでは通常のハプティック再生
  haptic_play();
#endif

  s_swipe_haptic_last_ms      = now;
  s_swipe_haptic_next_primary = false;
}

void keyball_swipe_haptic_reset_sequence(void) {
  s_swipe_haptic_last_ms      = 0;
  s_swipe_haptic_next_primary = true;
}

void keyball_swipe_haptic_prepare_repeat(void) {
  s_swipe_haptic_next_primary = false;
  s_swipe_haptic_last_ms      = timer_read32();
}

static inline void keyball_swipe_haptic_feedback(void) {
  keyball_swipe_haptic_pulse();
}
#else
static inline void keyball_swipe_haptic_feedback(void) {}
#endif

static inline int16_t kb_abs16(int16_t v) { return (v < 0) ? -v : v; }
static inline void kb_sat_add_pos32(int32_t *acc, int32_t delta) {
  int64_t t = (int64_t)(*acc) + (int64_t)delta;
  if (t > (1L<<27)) t = (1L<<27);
  if (t < 0) t = 0;
  *acc = (int32_t)t;
}

typedef struct {
  bool            active;
  kb_swipe_tag_t  tag;
  bool            fired_any;
  kb_swipe_dir_t  last_dir;
  int32_t acc_r, acc_l, acc_d, acc_u;
  uint32_t last_ms_up;
  uint32_t last_ms_down;
  uint32_t last_ms_left;
  uint32_t last_ms_right;
} kb_swipe_session_t;

static kb_swipe_session_t g_sw = {0};
static uint32_t g_sw_idle_timer = 0;

void keyball_swipe_begin(kb_swipe_tag_t mode_tag) {
  g_sw.active   = true;
  g_sw.tag      = mode_tag;
  g_sw.fired_any= false;
  g_sw.last_dir = KB_SWIPE_NONE;
  g_sw.acc_r = g_sw.acc_l = g_sw.acc_d = g_sw.acc_u = 0;
  g_sw.last_ms_up = g_sw.last_ms_down = g_sw.last_ms_left = g_sw.last_ms_right = 0;
}
void keyball_swipe_end(void) {
  kb_swipe_tag_t prev_tag = g_sw.tag;
  g_sw.active   = false;
  g_sw.tag      = 0;
  g_sw.fired_any= false;
  g_sw.last_dir = KB_SWIPE_NONE;
  g_sw.acc_r = g_sw.acc_l = g_sw.acc_d = g_sw.acc_u = 0;
  // user側クリーンアップフック（終了前のタグを渡す）
  if (keyball_on_swipe_end) {
    keyball_on_swipe_end(prev_tag);
  }
}
bool           keyball_swipe_is_active(void)         { return g_sw.active; }
kb_swipe_tag_t keyball_swipe_mode_tag(void)          { return g_sw.tag; }
kb_swipe_dir_t keyball_swipe_direction(void)         { return g_sw.last_dir; }
bool           keyball_swipe_fired_since_begin(void) { return g_sw.fired_any; }
bool           keyball_swipe_consume_fired(void)     { bool f = g_sw.fired_any; g_sw.fired_any = false; return f; }

kb_swipe_params_t keyball_swipe_get_params(void){
    kb_swipe_params_t p = {
        .step     = kbpf.swipe_step,
        .deadzone = kbpf.swipe_deadzone,
        .reset_ms = kbpf.swipe_reset_ms,
        .freeze   = (kbpf.swipe_freeze & 1) != 0
    };
    return p;
}

void keyball_swipe_set_step(uint16_t v){
  if (v < 10) v = 10;
  if (v > 2000) v = 2000;
  kbpf.swipe_step = v;
  uprintf("SW step=%u\r\n", kbpf.swipe_step);
}

void keyball_swipe_set_deadzone(uint8_t v){
  if (v > 32) v = 32;
  kbpf.swipe_deadzone = v;
  uprintf("SW deadzone=%u\r\n", kbpf.swipe_deadzone);
}

void keyball_swipe_set_reset_ms(uint8_t v){
  if (v > 250) v = 250;
  kbpf.swipe_reset_ms = v;
  uprintf("SW reset_ms=%u\r\n", kbpf.swipe_reset_ms);
}

void keyball_swipe_set_freeze(bool on){
  kbpf.swipe_freeze = on ? 1 : 0;
  uprintf("SW freeze=%u\r\n", kbpf.swipe_freeze ? 1 : 0);
}

void keyball_swipe_toggle_freeze(void){
  kbpf.swipe_freeze ^= 1;
  uprintf("SW freeze=%u\r\n", kbpf.swipe_freeze ? 1 : 0);
}

static inline uint32_t* kb_sw_dir_time_ptr(kb_swipe_dir_t dir) {
  switch (dir) {
    case KB_SWIPE_UP:    return &g_sw.last_ms_up;
    case KB_SWIPE_DOWN:  return &g_sw.last_ms_down;
    case KB_SWIPE_LEFT:  return &g_sw.last_ms_left;
    case KB_SWIPE_RIGHT: return &g_sw.last_ms_right;
    default: return &g_sw.last_ms_up; // fallback
  }
}

// try to fire once per step; if cooldown中なら今回は発火せず蓄積を抑制
static void kb_sw_try_fire(kb_swipe_dir_t dir,
    int32_t *acc_target,
    int32_t *a1, int32_t *a2, int32_t *a3) {

  while (*acc_target >= kbpf.swipe_step) {
    // クールタイム判定
    uint16_t cd = 0;
    if (keyball_swipe_get_cooldown_ms) {
      cd = keyball_swipe_get_cooldown_ms(g_sw.tag, dir);
    }
    uint32_t now = timer_read32();
    uint32_t *plast = kb_sw_dir_time_ptr(dir);
    if (cd > 0 && timer_elapsed32(*plast) < cd) {
      // 今回は発火せず、連続ループを防ぐためにしきい値直前まで抑制
      int32_t step_minus1 = (kbpf.swipe_step > 0) ? (int32_t)kbpf.swipe_step - 1 : 0;
      if (*acc_target > step_minus1) *acc_target = step_minus1;
      *a1 = *a2 = *a3 = 0;
      break;
    }

    // swipe発火時の処理（ユーザーフック + ハプティック）
#ifdef SPLIT_KEYBOARD
    if (is_keyboard_master()) {
      if (keyball_on_swipe_fire) {
        keyball_on_swipe_fire(g_sw.tag, dir);
      }
      keyball_swipe_haptic_feedback();
    }
#else
    if (keyball_on_swipe_fire) {
      keyball_on_swipe_fire(g_sw.tag, dir);
    }
    keyball_swipe_haptic_feedback();
#endif
    g_sw.fired_any = true;
    g_sw.last_dir  = dir;
    *plast = now;

    *acc_target -= kbpf.swipe_step;
    if (*acc_target < 0) *acc_target = 0;
    *a1 = *a2 = *a3 = 0;
  }
}

void keyball_swipe_apply(report_mouse_t *report, report_mouse_t *output, bool is_left) {
  int16_t sx = (int16_t)report->x;
  int16_t sy = (int16_t)report->y;
  uint32_t now = timer_read32();

  if (kb_abs16(sx) < kbpf.swipe_deadzone) sx = 0;
  if (kb_abs16(sy) < kbpf.swipe_deadzone) sy = 0;

  if (sx == 0 && sy == 0) {
    if (timer_elapsed32(g_sw_idle_timer) > kbpf.swipe_reset_ms) {
      g_sw.acc_r = g_sw.acc_l = g_sw.acc_d = g_sw.acc_u = 0;
    }
    return;
  }

  g_sw_idle_timer = now;

  if (sx > 0)      { kb_sat_add_pos32(&g_sw.acc_r, sx);  g_sw.acc_l = 0; }
  else if (sx < 0) { kb_sat_add_pos32(&g_sw.acc_l, -sx); g_sw.acc_r = 0; }

  if (sy > 0)      { kb_sat_add_pos32(&g_sw.acc_d, sy);  g_sw.acc_u = 0; }
  else if (sy < 0) { kb_sat_add_pos32(&g_sw.acc_u, -sy); g_sw.acc_d = 0; }

  bool prefer_x = (kb_abs16((int16_t)report->x) >= kb_abs16((int16_t)report->y));
  if (prefer_x) {
    kb_sw_try_fire(KB_SWIPE_RIGHT, &g_sw.acc_r, &g_sw.acc_l, &g_sw.acc_u, &g_sw.acc_d);
    kb_sw_try_fire(KB_SWIPE_LEFT,  &g_sw.acc_l, &g_sw.acc_r, &g_sw.acc_u, &g_sw.acc_d);
    kb_sw_try_fire(KB_SWIPE_DOWN,  &g_sw.acc_d, &g_sw.acc_u, &g_sw.acc_r, &g_sw.acc_l);
    kb_sw_try_fire(KB_SWIPE_UP,    &g_sw.acc_u, &g_sw.acc_d, &g_sw.acc_r, &g_sw.acc_l);
  } else {
    kb_sw_try_fire(KB_SWIPE_DOWN,  &g_sw.acc_d, &g_sw.acc_u, &g_sw.acc_r, &g_sw.acc_l);
    kb_sw_try_fire(KB_SWIPE_UP,    &g_sw.acc_u, &g_sw.acc_d, &g_sw.acc_r, &g_sw.acc_l);
    kb_sw_try_fire(KB_SWIPE_RIGHT, &g_sw.acc_r, &g_sw.acc_l, &g_sw.acc_u, &g_sw.acc_d);
    kb_sw_try_fire(KB_SWIPE_LEFT,  &g_sw.acc_l, &g_sw.acc_r, &g_sw.acc_u, &g_sw.acc_d);
  }
}

void keyball_swipe_get_accum(uint32_t *r, uint32_t *l, uint32_t *d, uint32_t *u) {
  if (r) *r = (g_sw.acc_r < 0) ? 0 : (uint32_t)g_sw.acc_r;
  if (l) *l = (g_sw.acc_l < 0) ? 0 : (uint32_t)g_sw.acc_l;
  if (d) *d = (g_sw.acc_d < 0) ? 0 : (uint32_t)g_sw.acc_d;
  if (u) *u = (g_sw.acc_u < 0) ? 0 : (uint32_t)g_sw.acc_u;
}

void keyball_swipe_fire_once(kb_swipe_dir_t dir) {
  if (!g_sw.active) return;
#ifdef SPLIT_KEYBOARD
  if (!is_keyboard_master()) return;
#endif
  // クールタイム判定（単発）
  uint16_t cd = 0;
  if (keyball_swipe_get_cooldown_ms) {
    cd = keyball_swipe_get_cooldown_ms(g_sw.tag, dir);
  }
  uint32_t *plast = kb_sw_dir_time_ptr(dir);
  if (cd > 0 && timer_elapsed32(*plast) < cd) {
    return; // クールタイム中は破棄
  }
  if (keyball_on_swipe_fire) {
    keyball_on_swipe_fire(g_sw.tag, dir);
  }
  keyball_swipe_haptic_feedback();
  g_sw.fired_any = true;
  g_sw.last_dir  = dir;
  *plast = timer_read32();
}

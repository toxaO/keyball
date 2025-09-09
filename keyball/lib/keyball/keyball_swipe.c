#include <stdint.h>
#include "quantum.h"
#include "print.h"
#include "timer.h"
#include "oled_driver.h"

#include "keyball.h"
#include "keyball_swipe.h"

static kb_oled_mode_t g_oled_mode = KB_OLED_MODE_NORMAL;
static uint8_t        g_sw_dbg_page = 0;
static bool           g_sw_dbg_en   = true;

#define KB_SW_DBG_PAGE_COUNT   3
#define KB_OLED_UI_DEBOUNCE_MS 100

static uint32_t g_oled_ui_ts = 0;

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
} kb_swipe_session_t;

static kb_swipe_session_t g_sw = {0};

void keyball_swipe_begin(kb_swipe_tag_t mode_tag) {
  g_sw.active   = true;
  g_sw.tag      = mode_tag;
  g_sw.fired_any= false;
  g_sw.last_dir = KB_SWIPE_NONE;
  g_sw.acc_r = g_sw.acc_l = g_sw.acc_d = g_sw.acc_u = 0;
}
void keyball_swipe_end(void) {
  g_sw.active   = false;
  g_sw.tag      = 0;
  g_sw.fired_any= false;
  g_sw.last_dir = KB_SWIPE_NONE;
  g_sw.acc_r = g_sw.acc_l = g_sw.acc_d = g_sw.acc_u = 0;
}
bool           keyball_swipe_is_active(void)         { return g_sw.active; }
kb_swipe_tag_t keyball_swipe_mode_tag(void)          { return g_sw.tag; }
kb_swipe_dir_t keyball_swipe_direction(void)         { return g_sw.last_dir; }
bool           keyball_swipe_fired_since_begin(void) { return g_sw.fired_any; }
bool           keyball_swipe_consume_fired(void)     { bool f = g_sw.fired_any; g_sw.fired_any = false; return f; }

kb_swipe_params_t keyball_swipe_get_params(void){
    kb_swipe_params_t p = {
        .step     = kbpf.step,
        .deadzone = kbpf.deadzone,
        .freeze   = (kbpf.freeze & 1) != 0
    };
    return p;
}

void keyball_swipe_set_step(uint16_t v){
  if (v < 10) v = 10;
  if (v > 2000) v = 2000;
  kbpf.step = v;
  uprintf("SW step=%u\r\n", kbpf.step);
}

void keyball_swipe_set_deadzone(uint8_t v){
  if (v > 32) v = 32;
  kbpf.deadzone = v;
  uprintf("SW deadzone=%u\r\n", kbpf.deadzone);
}

void keyball_swipe_set_freeze(bool on){
  kbpf.freeze = on ? 1 : 0;
  uprintf("SW freeze=%u\r\n", kbpf.freeze ? 1 : 0);
}

void keyball_swipe_toggle_freeze(void){
  kbpf.freeze ^= 1;
  uprintf("SW freeze=%u\r\n", kbpf.freeze ? 1 : 0);
}

static inline bool ui_op_ready(void){
  if (TIMER_DIFF_32(timer_read32(), g_oled_ui_ts) < KB_OLED_UI_DEBOUNCE_MS) return false;
  g_oled_ui_ts = timer_read32();
  return true;
}

void keyball_oled_set_mode(kb_oled_mode_t m){
  g_oled_mode = m;
  g_sw_dbg_en = (m == KB_OLED_MODE_DEBUG);
  oled_clear();
}

void keyball_oled_mode_toggle(void){
  if (!ui_op_ready()) return;
  keyball_oled_set_mode((g_oled_mode == KB_OLED_MODE_DEBUG) ? KB_OLED_MODE_NORMAL : KB_OLED_MODE_DEBUG);
}

kb_oled_mode_t keyball_oled_get_mode(void){ return g_oled_mode; }

void keyball_swipe_dbg_next_page(void){
  if (!ui_op_ready()) return;
  g_sw_dbg_page = (g_sw_dbg_page + 1) % KB_SW_DBG_PAGE_COUNT;
  oled_clear();
}

void keyball_swipe_dbg_prev_page(void){
  if (!ui_op_ready()) return;
  g_sw_dbg_page = (g_sw_dbg_page + KB_SW_DBG_PAGE_COUNT - 1) % KB_SW_DBG_PAGE_COUNT;
  oled_clear();
}

uint8_t keyball_swipe_dbg_get_page(void){ return g_sw_dbg_page; }

void keyball_swipe_dbg_toggle(void)         { g_sw_dbg_en = !g_sw_dbg_en; }
void keyball_swipe_dbg_show(bool on)        { g_sw_dbg_en = on; }

static void kb_sw_try_fire(kb_swipe_dir_t dir,
    int32_t *acc_target,
    int32_t *a1, int32_t *a2, int32_t *a3) {

  while (*acc_target >= kbpf.step) {
    if (keyball_on_swipe_fire) {
      keyball_on_swipe_fire(g_sw.tag, dir);
    }
    g_sw.fired_any = true;
    g_sw.last_dir  = dir;

    *acc_target -= kbpf.step;
    if (*acc_target < 0) *acc_target = 0;
    *a1 = *a2 = *a3 = 0;
  }
}

void keyball_swipe_apply(report_mouse_t *report, report_mouse_t *output, bool is_left) {
  int16_t sx = (int16_t)report->x;
  int16_t sy = (int16_t)report->y;

  if (kb_abs16(sx) < kbpf.deadzone) sx = 0;
  if (kb_abs16(sy) < kbpf.deadzone) sy = 0;

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

#if defined(OLED_ENABLE) || defined(OLED_DRIVER_ENABLE)
// 再入防止（レンダ中に再呼び出されてもスキップ）
static bool g_sw_dbg_in_render = false;

// 0..9999 に丸めた無符号
static inline unsigned clip0_9999(int32_t v){
  if (v <= 0) return 0u;
  if (v >= 9999) return 9999u;
  return (unsigned)v;
}
static inline const char* kb_dir_str(kb_swipe_dir_t d){
  return (d==KB_SWIPE_UP)?"UP ":(d==KB_SWIPE_DOWN)?"DN ":
    (d==KB_SWIPE_LEFT)?"LT ":(d==KB_SWIPE_RIGHT)?"RT ":"NON";
}

void keyball_oled_render_swipe_debug(void){
  if (g_oled_mode != KB_OLED_MODE_DEBUG) return;
  if (!g_sw_dbg_en) return;
  if (g_sw_dbg_in_render) return;
  g_sw_dbg_in_render = true;

  char line[32];
  oled_set_cursor(0, 0);

  switch (g_sw_dbg_page) {
    case 0: {
              uint16_t cpi = keyball_get_cpi();
              snprintf(line, sizeof(line), "CPI:%u", (unsigned)cpi);
              oled_write_ln(line, false);

#ifdef KEYBALL_MOVE_SHAPING_ENABLE
              extern int32_t g_move_gain_lo_fp;
              extern int16_t g_move_th1;
              snprintf(line, sizeof(line), "Glo:%ld Th1:%d", (long)g_move_gain_lo_fp, (int)g_move_th1);
#else
              snprintf(line, sizeof(line), "MoveShape:OFF");
#endif
              oled_write_ln(line, false);

              snprintf(line, sizeof(line), "Div:%u Inv:%u",
                       (unsigned)keyball_get_scroll_div(),
                       (unsigned)(kbpf.inv[osi()] ? 1 : 0));
              oled_write_ln(line, false);

              snprintf(line, sizeof(line), "Pg:%u/%u", (unsigned)(g_sw_dbg_page+1), (unsigned)KB_SW_DBG_PAGE_COUNT);
              oled_write_ln(line, false);
            } break;

    case 1: {
              kb_swipe_params_t p = keyball_swipe_get_params();
              snprintf(line, sizeof(line), "A:%u Tg:%u Fz:%u",
                  keyball_swipe_is_active()?1u:0u,
                  (unsigned)keyball_swipe_mode_tag(),
                  p.freeze?1u:0u);
              oled_write_ln(line, false);

              snprintf(line, sizeof(line), "St:%u Dz:%u",
                  (unsigned)p.step, (unsigned)p.deadzone);
              oled_write_ln(line, false);

              snprintf(line, sizeof(line), "Dir:%s Fd:%u",
                  kb_dir_str(keyball_swipe_direction()),
                  keyball_swipe_fired_since_begin()?1u:0u);
              oled_write_ln(line, false);

              snprintf(line, sizeof(line), "Pg:%u/%u", (unsigned)(g_sw_dbg_page+1), (unsigned)KB_SW_DBG_PAGE_COUNT);
              oled_write_ln(line, false);
            } break;

    case 2: {
              unsigned ar = clip0_9999(g_sw.acc_r);
              unsigned al = clip0_9999(g_sw.acc_l);
              unsigned ad = clip0_9999(g_sw.acc_d);
              unsigned au = clip0_9999(g_sw.acc_u);

              snprintf(line, sizeof(line), "R%4u L%4u", ar, al);
              oled_write_ln(line, false);
              snprintf(line, sizeof(line), "D%4u U%4u", ad, au);
              oled_write_ln(line, false);

              oled_write_ln("Acc", false);
              snprintf(line, sizeof(line), "Pg:%u/%u", (unsigned)(g_sw_dbg_page+1), (unsigned)KB_SW_DBG_PAGE_COUNT);
              oled_write_ln(line, false);
            } break;
  }

  g_sw_dbg_in_render = false;
}
#else
void keyball_oled_render_swipe_debug(void) {}
#endif

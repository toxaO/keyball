#include <stdint.h>
#include <stdbool.h>
#include "quantum.h"
#include "timer.h"
#include "oled_driver.h"

#include "keyball_oled.h"

static kb_oled_mode_t g_oled_mode = KB_OLED_MODE_NORMAL;
static bool           g_dbg_en   = true;
static uint8_t        g_oled_page = 0;

#define KB_OLED_PAGE_COUNT      3
#define KB_OLED_UI_DEBOUNCE_MS  100

static uint32_t g_oled_ui_ts = 0;

static inline bool ui_op_ready(void) {
    if (TIMER_DIFF_32(timer_read32(), g_oled_ui_ts) < KB_OLED_UI_DEBOUNCE_MS) {
        return false;
    }
    g_oled_ui_ts = timer_read32();
    return true;
}

void keyball_oled_set_mode(kb_oled_mode_t m) {
    g_oled_mode = m;
    g_dbg_en    = (m == KB_OLED_MODE_DEBUG);
    g_oled_page = 0;
    oled_clear();
}

void keyball_oled_mode_toggle(void) {
    if (!ui_op_ready()) return;
    keyball_oled_set_mode((g_oled_mode == KB_OLED_MODE_DEBUG) ? KB_OLED_MODE_NORMAL : KB_OLED_MODE_DEBUG);
}

kb_oled_mode_t keyball_oled_get_mode(void) { return g_oled_mode; }

void keyball_oled_next_page(void) {
    if (!ui_op_ready()) return;
    g_oled_page = (g_oled_page + 1) % KB_OLED_PAGE_COUNT;
    oled_clear();
}

void keyball_oled_prev_page(void) {
    if (!ui_op_ready()) return;
    g_oled_page = (g_oled_page + KB_OLED_PAGE_COUNT - 1) % KB_OLED_PAGE_COUNT;
    oled_clear();
}

uint8_t keyball_oled_get_page(void) { return g_oled_page; }
uint8_t keyball_oled_get_page_count(void) { return KB_OLED_PAGE_COUNT; }

void keyball_oled_dbg_toggle(void) { g_dbg_en = !g_dbg_en; }
void keyball_oled_dbg_show(bool on) { g_dbg_en = on; }
bool keyball_oled_dbg_enabled(void) { return g_dbg_en; }

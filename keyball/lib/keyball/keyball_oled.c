#include <stdint.h>
#include <stdbool.h>
#include "quantum.h"
#include "timer.h"
#include "oled_driver.h"

#include "keyball_oled.h"
#include "keyball.h"
#include <stdio.h>

static kb_oled_mode_t g_oled_mode = KB_OLED_MODE_NORMAL;
static bool           g_dbg_en   = true;
static uint8_t        g_oled_page = 0;

#define KB_OLED_PAGE_COUNT      7
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

// ---------------------------------------------------------------------------
// Simple info renderers shared by keymaps

#ifdef OLED_ENABLE
static char to_1x(uint8_t x) {
    x &= 0x0f;
    return x < 10 ? (char)(x + '0') : (char)(x + 'a' - 10);
}
static const char BL = '\xB0'; // Blank indicator

void keyball_oled_render_ballinfo(void) {
    oled_write_P(PSTR("CPI:"), false);
    {
        char b[6];
        snprintf(b, sizeof b, "%4u", (unsigned)keyball_get_cpi());
        oled_write(b, false);
    }
    // sdiv は除数ではなく "感度レベル(ST)" として運用
    oled_write_P(PSTR(" ST:"), false);
    {
        char b[4];
        snprintf(b, sizeof b, "%u", (unsigned)keyball_get_scroll_div());
        oled_write(b, false);
    }
    oled_write_P(PSTR(" INV:"), false);
    oled_write_char(kbpf.scroll_invert[keyball_os_idx()] ? '1' : '0', false);
    oled_write_ln_P(PSTR(""), false);
}

void keyball_oled_render_keyinfo(void) {
    // Format: `Key :  R{row}  C{col} K{kc} {name}{name}{name}`
    oled_write_P(PSTR("Key \xB1"), false);
    oled_write_char('\xB8', false);
    oled_write_char(to_1x(keyball.last_pos.row), false);
    oled_write_char('\xB9', false);
    oled_write_char(to_1x(keyball.last_pos.col), false);
    oled_write_P(PSTR("\xBA\xBB"), false);
    oled_write_char(to_1x(keyball.last_kc >> 4), false);
    oled_write_char(to_1x(keyball.last_kc), false);
    oled_write_P(PSTR("  "), false);
    oled_write(keyball.pressing_keys, false);
}

void keyball_oled_render_layerinfo(void) {
    oled_write_P(PSTR("L\xB6\xB7r\xB1"), false);
    for (uint8_t i = 1; i < 8; i++) {
        oled_write_char((layer_state_is(i) ? to_1x(i) : BL), false);
    }
    oled_write_char(' ', false);
#    ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
    oled_write_P(PSTR("\xC2\xC3"), false);
    if (get_auto_mouse_enable()) {
        oled_write_P(PSTR("ON "), false);
    } else {
        oled_write_P(PSTR("OFF"), false);
    }
    {
        char b[8];
        unsigned v = (unsigned)(get_auto_mouse_timeout() / 10);
        snprintf(b, sizeof b, "%u0", v);
        oled_write(b, false);
    }
#    else
    oled_write_P(PSTR("\xC2\xC3\xB4\xB5 ---"), false);
#    endif
}

void keyball_oled_render_ballsubinfo(void) {
    char b[20];
    snprintf(b, sizeof b, "GL:%3ld  TH1:%2d", (long)g_move_gain_lo_fp, (int)g_move_th1);
    oled_write_ln(b, false);
}
#endif // OLED_ENABLE

// Debug rendering ---------------------------------------------------------
static bool g_dbg_in_render = false;

static inline unsigned clip0_9999(uint32_t v) {
    return (v >= 9999u) ? 9999u : v;
}
static inline const char *kb_dir_str(kb_swipe_dir_t d) {
    return (d == KB_SWIPE_UP)    ? "UP " :
           (d == KB_SWIPE_DOWN) ? "DN " :
           (d == KB_SWIPE_LEFT) ? "LT " :
           (d == KB_SWIPE_RIGHT)? "RT " : "NON";
}

void keyball_oled_render_debug(void) {
    if (keyball_oled_get_mode() != KB_OLED_MODE_DEBUG) return;
    if (!keyball_oled_dbg_enabled()) return;
    if (g_dbg_in_render) return;
    g_dbg_in_render = true;

    char line[32];
    oled_set_cursor(0, 0);

    uint8_t page = keyball_oled_get_page();
    switch (page) {
        case 0: { // 1: Mouse Config
            oled_write_ln("[Mouse Config]", false);
            uint16_t cpi = keyball_get_cpi();
            snprintf(line, sizeof(line), "CPI:%u OS:%u", (unsigned)cpi, (unsigned)keyball_os_idx());
            oled_write_ln(line, false);

#ifdef KEYBALL_MOVE_SHAPING_ENABLE
            snprintf(line, sizeof(line), "Glo:%u Th1:%u",
                     (unsigned)kbpf.move_gain_lo_fp[keyball_os_idx()],
                     (unsigned)kbpf.move_th1[keyball_os_idx()]);
            oled_write_ln(line, false);
#else
            oled_write_ln("MoveShape:OFF", false);
#endif

            snprintf(line, sizeof(line), "Pg:%u/%u",
                     (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_ln(line, false);
        } break;

        case 1: { // 2: Auto Mouse Layer
            oled_write_ln("[Auto Mouse Layer]", false);
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
            // Enabled / Timeout
            snprintf(line, sizeof(line), "EN:%u TO:%u",
                     get_auto_mouse_enable() ? 1u : 0u,
                     (unsigned)get_auto_mouse_timeout());
            oled_write_ln(line, false);
            // Target Layer index
            snprintf(line, sizeof(line), "TG:%u", (unsigned)get_auto_mouse_layer());
            oled_write_ln(line, false);
#else
            oled_write_ln("Disabled", false);
#endif
            snprintf(line, sizeof(line), "Pg:%u/%u",
                     (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_ln(line, false);
        } break;

        case 2: { // 3: Scroll Param
            oled_write_ln("[Scroll Param]", false);
            uint8_t os = keyball_os_idx();
            uint8_t st = keyball_get_scroll_div();
            uint8_t inv = kbpf.scroll_invert[os] ? 1 : 0;
            uint8_t preset = kbpf.scroll_preset[os];
            snprintf(line, sizeof(line), "ST:%u Inv:%u", (unsigned)st, (unsigned)inv);
            oled_write_ln(line, false);
            const char *plabel = (preset == 2) ? "MAC" : (preset == 1) ? "FINE" : "NORM";
            snprintf(line, sizeof(line), "PST:%s", plabel);
            oled_write_ln(line, false);
            snprintf(line, sizeof(line), "Pg:%u/%u",
                     (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_ln(line, false);
        } break;

        case 3: { // 4: Scroll Snap
            oled_write_ln("[Scroll Snap]", false);
#if KEYBALL_SCROLLSNAP_ENABLE == 2
            keyball_scrollsnap_mode_t m = keyball_get_scrollsnap_mode();
            const char *ml = (m == KEYBALL_SCROLLSNAP_MODE_VERTICAL) ? "VRT" :
                             (m == KEYBALL_SCROLLSNAP_MODE_HORIZONTAL) ? "HOR" : "FRE";
            snprintf(line, sizeof(line), "Mode:%s", ml);
            oled_write_ln(line, false);
            snprintf(line, sizeof(line), "Thr:%u Rst:%u",
                     (unsigned)KEYBALL_SCROLLSNAP_TENSION_THRESHOLD,
                     (unsigned)KEYBALL_SCROLLSNAP_RESET_TIMER);
            oled_write_ln(line, false);
#else
            oled_write_ln("Disabled", false);
#endif
            snprintf(line, sizeof(line), "Pg:%u/%u",
                     (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_ln(line, false);
        } break;

        case 4: { // 5: Scroll Raw Monitor
            oled_write_ln("[Scroll Monitor]", false);
            int16_t sx, sy, h, v;
            keyball_scroll_get_dbg(&sx, &sy, &h, &v);
            snprintf(line, sizeof(line), "Sx:%4d Sy:%4d", sx, sy);
            oled_write_ln(line, false);
            snprintf(line, sizeof(line), "H:%4d V:%4d", h, v);
            oled_write_ln(line, false);
            snprintf(line, sizeof(line), "Pg:%u/%u",
                     (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_ln(line, false);
        } break;

        case 5: { // 6: Swipe Config
            kb_swipe_params_t p = keyball_swipe_get_params();
            oled_write_ln("[Swipe Config]", false);
            snprintf(line, sizeof(line), "St:%u Dz:%u",
                     (unsigned)p.step, (unsigned)p.deadzone);
            oled_write_ln(line, false);
            snprintf(line, sizeof(line), "Rt:%u Fz:%u",
                     (unsigned)p.reset_ms, p.freeze ? 1u : 0u);
            oled_write_ln(line, false);
            snprintf(line, sizeof(line), "Pg:%u/%u",
                     (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_ln(line, false);
        } break;

        case 6: { // 7: Swipe Monitor
            uint32_t ar, al, ad, au;
            keyball_swipe_get_accum(&ar, &al, &ad, &au);
            snprintf(line, sizeof(line), "Ac:%u Md:%u Fired?:%u",
                     keyball_swipe_is_active() ? 1u : 0u,
                     (unsigned)keyball_swipe_mode_tag(),
                     keyball_swipe_fired_since_begin() ? 1u : 0u);
            oled_write_ln(line, false);
            snprintf(line, sizeof(line), "Dir:%s", kb_dir_str(keyball_swipe_direction()));
            oled_write_ln(line, false);
            snprintf(line, sizeof(line), "R%4u L%4u", clip0_9999(ar), clip0_9999(al));
            oled_write_ln(line, false);
            snprintf(line, sizeof(line), "D%4u U%4u Pg:%u/%u",
                     clip0_9999(ad), clip0_9999(au),
                     (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_ln(line, false);
        } break;
    }

    g_dbg_in_render = false;
}

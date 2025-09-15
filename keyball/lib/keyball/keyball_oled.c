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
static bool           g_oled_vertical = false; // 右手マスター用の縦向き表示フラグ

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

#ifdef OLED_ENABLE
// 回転初期化（右手マスターのみ90度回転＝縦向き表示）
oled_rotation_t oled_init_kb(oled_rotation_t rotation) {
    // まず既存ユーザ初期化を反映
    rotation = oled_init_user(rotation);

    // 右手かつマスターのときに90度回転（時計回り）
    if (is_keyboard_master() && !is_keyboard_left()) {
        g_oled_vertical = true;
        return OLED_ROTATION_270; // 現在から180度追加で回転
    }

    g_oled_vertical = false;
    return rotation;
}
#endif

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
        case 0: { // 1: Mouse Config（指定体裁）
            oled_write_P("mouse", false);
            oled_write_ln(" conf", false);
            oled_write_ln("cpi:", false);
            snprintf(line, sizeof(line), " %u", (unsigned)keyball_get_cpi());
            oled_write_ln(line, false);
#ifdef KEYBALL_MOVE_SHAPING_ENABLE
            {
                uint8_t  g   = kbpf.move_gain_lo_fp[keyball_os_idx()];
                uint16_t pct = (uint16_t)((g * 100u + 127u) / 255u); // 四捨五入
                oled_write_ln("Glo:", false);
                snprintf(line, sizeof(line), "  %u%%", (unsigned)pct);
                oled_write_ln(line, false);
            }
            oled_write_ln("Th1:", false);
            snprintf(line, sizeof(line), "    %u", (unsigned)kbpf.move_th1[keyball_os_idx()]);
            oled_write_ln(line, false);
#else
            oled_write_ln("Glo:", false);
            oled_write_ln("  OFF", false);
            oled_write_ln("Th1:", false);
            oled_write_ln("    -", false);
#endif
            oled_write_ln("", false);
            oled_write_ln("", false);
            oled_write_ln("page", false);
            snprintf(line, sizeof(line), "  %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_P(line, false);
        } break;

        case 1: { // 2: Auto Mouse Layer（指定体裁）
            oled_write_ln("AML", false);
            oled_write_ln(" conf", false);
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
            oled_write_ln("en:", false);
            snprintf(line, sizeof(line), "    %u", get_auto_mouse_enable() ? 1u : 0u);
            oled_write_ln(line, false);
            oled_write_ln("TO:", false);
            snprintf(line, sizeof(line), " %u", (unsigned)get_auto_mouse_timeout());
            oled_write_ln(line, false);
            oled_write_P("TG_L:", false);
            snprintf(line, sizeof(line), "    %u", (unsigned)get_auto_mouse_layer());
            oled_write_ln(line, false);
#else
            oled_write_ln("en:", false);
            oled_write_ln("    0", false);
            oled_write_ln("TO:", false);
            oled_write_ln(" 0", false);
            oled_write_P("TG_L:", false);
            oled_write_ln("    0", false);
#endif
            oled_write_ln("", false);
            oled_write_ln("", false);
            oled_write_ln("page", false);
            snprintf(line, sizeof(line), "  %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_P(line, false);
        } break;

        case 2: { // 3: Scroll Param（指定体裁）
            oled_write_ln("Scrl", false);
            oled_write_ln(" conf", false);
            uint8_t os = keyball_os_idx();
            uint8_t inv = kbpf.scroll_invert[os] ? 1 : 0;
            uint8_t preset = kbpf.scroll_preset[os];
            oled_write_ln("Inv:", false);
            snprintf(line, sizeof(line), "    %u", (unsigned)inv);
            oled_write_ln(line, false);
            oled_write_ln("PST:", false);
            const char *pl = (preset == 2) ? " mac" : (preset == 1) ? " fine" : " norm";
            oled_write_ln(pl, false);
            oled_write_ln("", false);
            oled_write_ln("", false);
            oled_write_ln("", false);
            oled_write_ln("", false);
            oled_write_ln("", false);
            oled_write_ln("", false);
            oled_write_ln("page", false);
            snprintf(line, sizeof(line), "  %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_P(line, false);
        } break;

        case 3: { // 4: Scroll Snap（指定体裁）
            oled_write_ln("SSNP", false);
            oled_write_ln(" conf", false);
#if KEYBALL_SCROLLSNAP_ENABLE == 2
            keyball_scrollsnap_mode_t m = keyball_get_scrollsnap_mode();
            const char *ml = (m == KEYBALL_SCROLLSNAP_MODE_VERTICAL) ? " VRT" :
                             (m == KEYBALL_SCROLLSNAP_MODE_HORIZONTAL) ? " HOR" : " FRE";
            oled_write_P("Mode:", false);
            oled_write_ln(ml, false);
            oled_write_ln("Thr:", false);
            snprintf(line, sizeof(line), "    %u", (unsigned)KEYBALL_SCROLLSNAP_TENSION_THRESHOLD);
            oled_write_ln(line, false);
            oled_write_ln("Rst:", false);
            snprintf(line, sizeof(line), "  %u", (unsigned)KEYBALL_SCROLLSNAP_RESET_TIMER);
            oled_write_ln(line, false);
#else
            oled_write_P("Mode:", false);
            oled_write_ln(" OFF", false);
            oled_write_ln("Thr:", false);
            oled_write_ln("    -", false);
            oled_write_ln("Rst:", false);
            oled_write_ln("  -", false);
#endif
            oled_write_ln("", false);
            oled_write_ln("", false);
            oled_write_ln("", false);
            oled_write_ln("page", false);
            snprintf(line, sizeof(line), " %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_ln(line, false);
        } break;

        case 4: { // 5: Scroll Raw Monitor（指定体裁）
            oled_write_P("Scrl:", false);
            oled_write_ln(" moni", false);
            int16_t sx, sy, h, v;
            keyball_scroll_get_dbg(&sx, &sy, &h, &v);
            oled_write_ln("sx:", false);
            snprintf(line, sizeof(line), "   %d", (int)sx);
            oled_write_ln(line, false);
            oled_write_ln("sy:", false);
            snprintf(line, sizeof(line), "   %d", (int)sy);
            oled_write_ln(line, false);
            oled_write_ln("H:", false);
            snprintf(line, sizeof(line), "   %d", (int)h);
            oled_write_ln(line, false);
            oled_write_ln("V:", false);
            snprintf(line, sizeof(line), "   %d", (int)v);
            oled_write_ln(line, false);
            oled_write_ln("", false);
            oled_write_ln("", false);
            oled_write_ln("", false);
            oled_write_ln("page", false);
            snprintf(line, sizeof(line), "  %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_P(line, false);
        } break;

        case 5: { // 6: Swipe Config（指定体裁）
            kb_swipe_params_t p = keyball_swipe_get_params();
            oled_write_P("Swipe", false);
            oled_write_ln(" conf", false);
            oled_write_ln("St:", false);
            snprintf(line, sizeof(line), "  %u", (unsigned)p.step);
            oled_write_P(line, false);
            oled_write_ln("Dz:", false);
            snprintf(line, sizeof(line), "    %u", (unsigned)p.deadzone);
            oled_write_P(line, false);
            oled_write_ln("Rt:", false);
            snprintf(line, sizeof(line), "   %u", (unsigned)p.reset_ms);
            oled_write_P(line, false);
            oled_write_ln("Frz:", false);
            snprintf(line, sizeof(line), "    %u", p.freeze ? 1u : 0u);
            oled_write_ln(line, false);
            oled_write_ln("", false);
            oled_write_ln("", false);
            oled_write_ln("page", false);
            snprintf(line, sizeof(line), "%u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_ln(line, false);
        } break;

        case 6: { // 7: Swipe Monitor（指定体裁）
            uint32_t ar, al, ad, au;
            keyball_swipe_get_accum(&ar, &al, &ad, &au);
            oled_write_P("Swipe", false);
            oled_write_ln(" moni", false);
            snprintf(line, sizeof(line), "Ac: %u", keyball_swipe_is_active() ? 1u : 0u);
            oled_write_P(line, false);
            snprintf(line, sizeof(line), "Md: %u", (unsigned)keyball_swipe_mode_tag());
            oled_write_P(line, false);
            oled_write_ln("Dir:", false);
            {
                const char *d = (keyball_swipe_direction() == KB_SWIPE_UP) ? "  Up" :
                                 (keyball_swipe_direction() == KB_SWIPE_DOWN) ? "  Dn" :
                                 (keyball_swipe_direction() == KB_SWIPE_LEFT) ? "  Lt" :
                                 (keyball_swipe_direction() == KB_SWIPE_RIGHT) ? "  Rt" : "  Non";
                oled_write_ln(d, false);
            }
            snprintf(line, sizeof(line), "U:  %u", clip0_9999(au));
            oled_write_P(line, false);
            snprintf(line, sizeof(line), "D:  %u", clip0_9999(ad));
            oled_write_P(line, false);
            snprintf(line, sizeof(line), "R:  %u", clip0_9999(ar));
            oled_write_P(line, false);
            snprintf(line, sizeof(line), "L:  %u", clip0_9999(al));
            oled_write_ln(line, false);
            oled_write_ln("", false);
            oled_write_ln("page", false);
            snprintf(line, sizeof(line), "  %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_ln(line, false);
        } break;
    }

    g_dbg_in_render = false;
}

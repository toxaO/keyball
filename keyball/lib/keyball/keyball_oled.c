#include <stdint.h>
#include <stdbool.h>
#include "quantum.h"
#include "timer.h"
#include "oled_driver.h"

#include "keyball_oled.h"
#include "keyball.h"
#include <stdio.h>
#ifdef RGBLIGHT_ENABLE
#    include "rgblight.h"
#endif

// 外部定数（定義は keyball.c）
extern const uint16_t CPI_MAX;
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
extern const uint16_t AML_TIMEOUT_MIN;
extern const uint16_t AML_TIMEOUT_MAX;
extern const uint16_t AML_TIMEOUT_QU;
#endif

static kb_oled_mode_t g_oled_mode = KB_OLED_MODE_NORMAL;
static bool           g_dbg_en   = true;
static uint8_t        g_oled_page = 0;
static bool           g_oled_vertical = false; // 右手マスター用の縦向き表示フラグ

// デバッグUI: ページごとの選択インデックス
#define KB_UI_PAGES 10
static uint8_t g_ui_sel_idx[KB_UI_PAGES] = {0};

static inline uint8_t ui_items_on_page(uint8_t p) {
    switch (p) {
        case 0: return 3; // CPI, Glo, Th1
        case 1: return 4; // AML: en, TO, TH, TG_L
        case 2: return 2; // Scroll: Inv, PST
        case 3: return 1; // SSNP: Mode のみ（Thr/Rstは固定）
        case 4: return 0; // Monitor
        case 5: return 4; // Swipe: St, Dz, Rt, Frz
        case 6: return 0; // Monitor
        case 7:
#ifdef RGBLIGHT_ENABLE
            return 5; // RGB: on/off, H, S, V, Mode
#else
            return 0;
#endif
        case 8: return 1; // Default layer: def
        case 9: return 0; // Send monitor (no selector)
    }
    return 0;
}

#define KB_OLED_PAGE_COUNT      10
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
    oled_clear();
}

void keyball_oled_mode_toggle(void) {
    if (!ui_op_ready()) return;
    keyball_oled_set_mode((g_oled_mode == KB_OLED_MODE_SETTING) ? KB_OLED_MODE_NORMAL : KB_OLED_MODE_SETTING);
}

kb_oled_mode_t keyball_oled_get_mode(void) { return g_oled_mode; }

void keyball_oled_next_page(void) {
    if (!ui_op_ready()) return;
    g_oled_page = (g_oled_page + 1) % KB_OLED_PAGE_COUNT;
    // 選択位置を0に初期化
    if (g_ui_sel_idx[g_oled_page] >= ui_items_on_page(g_oled_page)) {
        g_ui_sel_idx[g_oled_page] = 0;
    }
    oled_clear();
}

void keyball_oled_prev_page(void) {
    if (!ui_op_ready()) return;
    g_oled_page = (g_oled_page + KB_OLED_PAGE_COUNT - 1) % KB_OLED_PAGE_COUNT;
    if (g_ui_sel_idx[g_oled_page] >= ui_items_on_page(g_oled_page)) {
        g_ui_sel_idx[g_oled_page] = 0;
    }
    oled_clear();
}

uint8_t keyball_oled_get_page(void) { return g_oled_page; }
uint8_t keyball_oled_get_page_count(void) { return KB_OLED_PAGE_COUNT; }

// Setting helpers (ex-dbg)
void keyball_oled_setting_toggle(void) { g_dbg_en = !g_dbg_en; }
void keyball_oled_setting_show(bool on) { g_dbg_en = on; }
bool keyball_oled_setting_enabled(void) { return g_dbg_en; }

// 値行のプレフィックス（選択表示）
static inline void oled_write_val_ln(bool selected, const char* text) {
    oled_write_char(selected ? '>' : ' ', false);
    oled_write_ln(text, false);
}

static inline void oled_write_val_P(bool selected, const char* text) {
    oled_write_char(selected ? '>' : ' ', false);
    oled_write_P(text, false);
}

// Alignment helpers（5文字幅の行を基準に左/右寄せする）
typedef enum { OLED_ALIGN_LEFT = 0, OLED_ALIGN_CENTER = 1, OLED_ALIGN_RIGHT = 2 } oled_align_t;
static inline uint8_t kb_line_width(void) { return 5; }

__attribute__((unused)) static void oled_write_align_P(const char *ptext, uint8_t row, oled_align_t align) {
    char tmp[16];
    uint8_t width = kb_line_width();
    uint8_t lenP = (uint8_t)strlen_P(ptext);
    uint8_t len  = (lenP > width) ? width : lenP;
    uint8_t padL = (align == OLED_ALIGN_RIGHT) ? (uint8_t)(width - len) : 0;
    if (padL > width) padL = 0;
    // clear and move to row start
    oled_set_cursor(0, row);
    for (uint8_t i = 0; i < padL; ++i) oled_write_char(' ', false);
    // copy at most width chars from PROGMEM
    for (uint8_t i = 0; i < len; ++i) tmp[i] = (char)pgm_read_byte(ptext + i);
    tmp[len] = '\0';
    oled_write(tmp, false);
}

__attribute__((unused)) static void oled_write_align(const char *text, uint8_t row, oled_align_t align) {
    uint8_t width = kb_line_width();
    uint8_t len  = (uint8_t)strlen(text);
    if (len > width) len = width;
    uint8_t padL = (align == OLED_ALIGN_RIGHT) ? (uint8_t)(width - len) : 0;
    if (padL > width) padL = 0;
    oled_set_cursor(0, row);
    for (uint8_t i = 0; i < padL; ++i) oled_write_char(' ', false);
    for (uint8_t i = 0; i < len; ++i) oled_write_char(text[i], false);
}

// Auto-align helpers: 先頭が空白なら右寄せ、そうでなければ左寄せ
static void oled_clear_line(uint8_t row) {
    uint8_t width = kb_line_width();
    oled_set_cursor(0, row);
    for (uint8_t i = 0; i < width; ++i) oled_write_char(' ', false);
}

static void oled_write_auto_P(const char *ptext, uint8_t row) {
    char first = (char)pgm_read_byte(ptext);
    oled_align_t a = (first == ' ') ? OLED_ALIGN_RIGHT : OLED_ALIGN_LEFT;
    oled_clear_line(row);
    oled_write_align_P(ptext, row, a);
}

static void oled_write_auto(const char *text, uint8_t row) {
    oled_align_t a = (text && text[0] == ' ') ? OLED_ALIGN_RIGHT : OLED_ALIGN_LEFT;
    oled_clear_line(row);
    oled_write_align(text, row, a);
}

static void oled_write_val_auto(bool selected, const char *text, uint8_t row) {
    char buf[24];
    buf[0] = selected ? '>' : ' ';
    snprintf(buf + 1, sizeof(buf) - 1, "%s", text);
    oled_clear_line(row);
    oled_write_auto(buf, row);
}

// Right edge (no right margin) for page indicator
static void oled_write_rightmost(const char *text, uint8_t row) {
    // right aligned in 5-char width
    uint8_t width = kb_line_width();
    uint8_t len  = (uint8_t)strlen(text);
    if (len > width) len = width;
    uint8_t padL = (len >= width) ? 0 : (uint8_t)(width - len);
    oled_set_cursor(0, row);
    for (uint8_t i = 0; i < padL; ++i) oled_write_char(' ', false);
    for (uint8_t i = 0; i < len; ++i) oled_write_char(text[i], false);
}
__attribute__((unused)) static void oled_write_rightmost_P(const char *ptext, uint8_t row) {
    char tmp[16];
    uint8_t width = kb_line_width();
    uint8_t lenP = (uint8_t)strlen_P(ptext);
    uint8_t len  = (lenP > width) ? width : lenP;
    uint8_t padL = (len >= width) ? 0 : (uint8_t)(width - len);
    for (uint8_t i = 0; i < len; ++i) tmp[i] = (char)pgm_read_byte(ptext + i);
    tmp[len] = '\0';
    oled_set_cursor(0, row);
    for (uint8_t i = 0; i < padL; ++i) oled_write_char(' ', false);
    oled_write(tmp, false);
}

// UI操作（矢印キーをOLEDデバッグUIに割当て）
bool keyball_oled_handle_ui_key(uint16_t keycode, keyrecord_t *record) {
    if (keyball_oled_get_mode() != KB_OLED_MODE_SETTING) return false; // 通常表示では介入しない
    if (!record->event.pressed) return false; // 押下のみ処理

    uint8_t page = keyball_oled_get_page();
    uint8_t cnt  = ui_items_on_page(page);
    uint8_t sel  = g_ui_sel_idx[page];

    // セレクタ移動
    if (keycode == KC_UP || keycode == KC_DOWN) {
        if (cnt == 0) return false; // 対象なし
        if (!ui_op_ready()) return true; // 入力は消費
        if (keycode == KC_UP) {
            sel = (uint8_t)((sel + cnt - 1) % cnt);
        } else {
            sel = (uint8_t)((sel + 1) % cnt);
        }
        g_ui_sel_idx[page] = sel;
        return true; // ホストへは送らない
    }

    // 値調整 / ページ送り（Shift+←/→）
    if (keycode == KC_LEFT || keycode == KC_RIGHT) {
        // Shift 併用ならページ移動（内部でデバウンスするためここでは呼ばない）
        if (get_mods() & MOD_MASK_SHIFT) {
            if (keycode == KC_LEFT) {
                keyball_oled_prev_page();
            } else {
                keyball_oled_next_page();
            }
            return true; // ホストへは送らない
        }
        if (!ui_op_ready()) return true; // 入力は消費
        int dir = (keycode == KC_RIGHT) ? +1 : -1;

        switch (page) {
            case 0: { // Mouse conf
                uint8_t os = keyball_os_idx();
                if (sel == 0) {
                    // CPI ±100
                    uint16_t cpi = keyball_get_cpi();
                    int32_t ncpi = (int32_t)cpi + dir * 100;
                    if (ncpi < 100) ncpi = 100;
                    if (ncpi > CPI_MAX) ncpi = CPI_MAX;
                    keyball_set_cpi((uint16_t)ncpi);
                } else if (sel == 1) {
                    // Glo ±1% (16..255)
                    uint16_t fp = kbpf.move_gain_lo_fp[os];
                    // 1% ≒ 2.55 → 3刻みで近似
                    int32_t nfp = (int32_t)fp + dir * 3;
                    if (nfp < 16) nfp = 16;
                    if (nfp > 255) nfp = 255;
                    kbpf.move_gain_lo_fp[os] = (uint8_t)nfp;
                    g_move_gain_lo_fp        = (uint8_t)nfp;
                } else if (sel == 2) {
                    // Th1 ±1 (0..th2-1)
                    uint8_t th2 = kbpf.move_th2[os];
                    int32_t th1 = (int32_t)kbpf.move_th1[os] + dir;
                    if (th1 < 0) th1 = 0;
                    if (th1 > (int32_t)th2 - 1) th1 = (int32_t)th2 - 1;
                    kbpf.move_th1[os] = (uint8_t)th1;
                    g_move_th1        = (int16_t)th1;
                }
            } break;

            case 1: { // AML
                if (sel == 0) {
                    // en: 0/1
                    bool en = get_auto_mouse_enable();
                    en = (dir > 0) ? true : false;
                    set_auto_mouse_enable(en);
                    kbpf.aml_enable = en ? 1 : 0;
                } else if (sel == 1) {
                    // TO: step variable: <3000 => +100/-100, >=3000 => +500/-500（3000→+は3500）
                    int32_t cur = (int32_t)get_auto_mouse_timeout();
                    // HOLD handling (60000 internal)
                    if (cur >= 60000 && dir < 0) {
                        cur = 9500; // leave HOLD, jump down to 9500 immediately
                    } else if (dir != 0) {
                        if (dir > 0) {
                            cur += (cur < 3000) ? 100 : 500;
                        } else {
                            cur -= (cur <= 3000) ? 100 : 500;
                        }
                    }
                    // range clamp and HOLD promotion
                    if (cur < 100) cur = 100;
                    if (cur > 9500 && cur < 60000) cur = 60000; // promote to HOLD
                    if (cur > 60000) cur = 60000;
                    if (cur >= 60000) {
                        set_auto_mouse_timeout(65535);
                    } else {
                        set_auto_mouse_timeout((uint16_t)cur);
                    }
                    kbpf.aml_timeout = (uint16_t)cur;
                } else if (sel == 2) {
                    // TH: ±1 (clamp 1..100)
                    int32_t th = (int32_t)kbpf.aml_threshold + dir * 1;
                    if (th < 1) th = 1;
                    if (th > 100) th = 100;
                    kbpf.aml_threshold = (uint8_t)th;
                } else if (sel == 3) {
                    // TG_L: 0..31
                    int32_t tg = (int32_t)get_auto_mouse_layer() + dir;
                    if (tg < 0) tg = 0;
                    if (tg > 31) tg = 31;
                    set_auto_mouse_layer((uint8_t)tg);
                    kbpf.aml_layer = (uint8_t)tg;
                }
            } break;

            case 2: { // Scroll conf
                uint8_t os = keyball_os_idx();
                if (sel == 0) {
                    // Inv: 0/1 toggle
                    uint8_t v = kbpf.scroll_invert[os] ? 1 : 0;
                    v = (dir > 0) ? 1 : 0;
                    kbpf.scroll_invert[os] = v;
                } else if (sel == 1) {
                    // PST: 0..2 cycle（norm/fine/mac）
                    int32_t p = (int32_t)kbpf.scroll_preset[os] + dir;
                    if (p < 0) p = 0;
                    if (p > 2) p = 2;
                    kbpf.scroll_preset[os] = (uint8_t)p;
                }
            } break;

            case 3: { // Scroll Snap
#if KEYBALL_SCROLLSNAP_ENABLE == 2
                keyball_scrollsnap_mode_t m = keyball_get_scrollsnap_mode();
                int32_t nm = (int32_t)m + dir;
                if (nm < 0) nm = 0;
                if (nm > 2) nm = 2;
                keyball_set_scrollsnap_mode((keyball_scrollsnap_mode_t)nm);
                kbpf.scrollsnap_mode = (uint8_t)nm;
#endif
            } break;

            case 5: { // Swipe conf
                kb_swipe_params_t p = keyball_swipe_get_params();
                if (sel == 0) {
                    int32_t v = (int32_t)p.step + dir * 10; // ±10
                    if (v < 1) v = 1;
                    if (v > 2000) v = 2000;
                    keyball_swipe_set_step((uint16_t)v);
                    kbpf.swipe_step = (uint16_t)v;
                } else if (sel == 1) {
                    int32_t v = (int32_t)p.deadzone + dir * 1; // ±1
                    if (v < 0) v = 0;
                    if (v > 32) v = 32;
                    keyball_swipe_set_deadzone((uint8_t)v);
                    kbpf.swipe_deadzone = (uint8_t)v;
                } else if (sel == 2) {
                    int32_t v = (int32_t)p.reset_ms + dir * 5; // ±5ms
                    if (v < 0) v = 0;
                    if (v > 250) v = 250;
                    keyball_swipe_set_reset_ms((uint8_t)v);
                    kbpf.swipe_reset_ms = (uint8_t)v;
                } else if (sel == 3) {
                    bool v = p.freeze ? true : false;
                    v = (dir > 0) ? true : false;
                    keyball_swipe_set_freeze(v);
                    kbpf.swipe_freeze = v ? 1 : 0;
                }
            } break;

#ifdef RGBLIGHT_ENABLE
            case 7: { // RGB conf
                if (sel == 0) {
                    if (dir > 0) {
                        rgblight_enable_noeeprom();
                    } else {
                        rgblight_disable_noeeprom();
                    }
                } else if (sel == 1 || sel == 2 || sel == 3) {
                    uint8_t h = rgblight_get_hue();
                    uint8_t s = rgblight_get_sat();
                    uint8_t v = rgblight_get_val();
                    int32_t nh = h, ns = s, nv = v;
                    const int step = 15; // H/S/V は±5刻み
                    if (sel == 1) nh = h + dir * step;
                    if (sel == 2) ns = s + dir * step;
                    if (sel == 3) nv = v + dir * step;
                    if (nh < 0) nh = 0;
                    if (nh > 255) nh = 255;
                    if (ns < 0) ns = 0;
                    if (ns > 255) ns = 255;
                    if (nv < 0) nv = 0;
                    if (nv > 255) nv = 255;
                    rgblight_sethsv_noeeprom((uint8_t)nh, (uint8_t)ns, (uint8_t)nv);
                } else if (sel == 4) {
                    if (dir > 0) rgblight_step(); else rgblight_step_reverse();
                }
            } break;
#endif

            case 8: { // Default layer conf
                if (sel == 0) {
                    int32_t v = (int32_t)kbpf.default_layer + dir;
                    if (v < 0) v = 0;
                    if (v > 31) v = 31;
                    kbpf.default_layer = (uint8_t)v;
                    default_layer_set((uint32_t)1u << kbpf.default_layer);
                }
            } break;

            default:
                break;
        }
        return true; // ホストへは送らない
    }

    return false;
}

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

void keyball_oled_render_setting(void) {
    if (keyball_oled_get_mode() != KB_OLED_MODE_SETTING) return;
    if (!keyball_oled_setting_enabled()) return;
    if (g_dbg_in_render) return;
    g_dbg_in_render = true;

    char line[32];
    oled_set_cursor(0, 0);

    uint8_t page = keyball_oled_get_page();
        switch (page) {
        case 0: { // 1: Mouse Config（指定体裁）
            uint8_t sel = g_ui_sel_idx[page];
            uint8_t row = 0;
            oled_write_auto_P(PSTR("mouse"), row++);
            oled_write_auto_P(PSTR(" conf"), row++);
            oled_write_auto_P(PSTR("cpi:"), row++);
            snprintf(line, sizeof(line), "%u", (unsigned)keyball_get_cpi());
            oled_write_val_auto(sel == 0, line, row++);
#ifdef KEYBALL_MOVE_SHAPING_ENABLE
            {
                uint8_t  g   = kbpf.move_gain_lo_fp[keyball_os_idx()];
                uint16_t pct = (uint16_t)((g * 100u + 127u) / 255u); // 四捨五入
                oled_write_auto_P(PSTR("Glo:"), row++);
                snprintf(line, sizeof(line), " %u%%", (unsigned)pct);
                oled_write_val_auto(sel == 1, line, row++);
            }
            oled_write_auto_P(PSTR("Th1:"), row++);
            snprintf(line, sizeof(line), "   %u", (unsigned)kbpf.move_th1[keyball_os_idx()]);
            oled_write_val_auto(sel == 2, line, row++);
#else
            oled_write_auto_P(PSTR("Glo:"), row++);
            oled_write_val_auto(sel == 1, "  OFF", row++);
            oled_write_auto_P(PSTR("Th1:"), row++);
            oled_write_val_auto(sel == 2, "   -", row++);
#endif
            // 追加: ポインタ移動量
            {
                char b[12];
                snprintf(b, sizeof b, "x:  %d", (int)keyball.last_mouse.x);
                oled_write_auto(b, row++);
                snprintf(b, sizeof b, "y:  %d", (int)keyball.last_mouse.y);
                oled_write_auto(b, row++);
            }
            // ページ表示
            snprintf(line, sizeof(line), " %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_rightmost(line, row++);
        } break;

        case 1: { // 2: Auto Mouse Layer（指定体裁）
            uint8_t sel = g_ui_sel_idx[page];
            uint8_t row = 0;
            oled_write_auto_P(PSTR("AML"), row++);
            oled_write_auto_P(PSTR(" conf"), row++);
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
            oled_write_auto_P(PSTR("en:"), row++);
            snprintf(line, sizeof(line), "   %u", get_auto_mouse_enable() ? 1u : 0u);
            oled_write_val_auto(sel == 0, line, row++);
            oled_write_auto_P(PSTR("TO:"), row++);
            {
                uint32_t to = (uint32_t)get_auto_mouse_timeout();
                if (to >= 60000u) {
                    oled_write_val_auto(sel == 1, " Hold", row++);
                } else {
                    snprintf(line, sizeof(line), "%u", (unsigned)to);
                    oled_write_val_auto(sel == 1, line, row++);
                }
            }
            oled_write_auto_P(PSTR("TH:"), row++);
            {
                snprintf(line, sizeof(line), "   %u", (unsigned)kbpf.aml_threshold);
                oled_write_val_auto(sel == 2, line, row++);
            }
            oled_write_auto_P(PSTR("TG_L:"), row++);
            snprintf(line, sizeof(line), "   %u", (unsigned)get_auto_mouse_layer());
            oled_write_val_auto(sel == 3, line, row++);
#else
            oled_write_auto_P(PSTR("en:"), row++);
            oled_write_val_auto(sel == 0, "    0", row++);
            oled_write_auto_P(PSTR("TO:"), row++);
            oled_write_val_auto(sel == 1, " 0", row++);
            oled_write_auto_P(PSTR("TH:"), row++);
            oled_write_val_auto(sel == 2, "    0", row++);
            oled_write_auto_P(PSTR("TG_L:"), row++);
            oled_write_val_auto(sel == 3, "    0", row++);
#endif
            snprintf(line, sizeof(line), " %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_rightmost(line, row++);
        } break;

        case 2: { // 3: Scroll Param（指定体裁）
            uint8_t sel = g_ui_sel_idx[page];
            uint8_t row = 0;
            oled_write_auto_P(PSTR("Scrl"), row++);
            oled_write_auto_P(PSTR(" conf"), row++);
            uint8_t os = keyball_os_idx();
            uint8_t inv = kbpf.scroll_invert[os] ? 1 : 0;
            uint8_t preset = kbpf.scroll_preset[os];
            oled_write_auto_P(PSTR("Inv:"), row++);
            snprintf(line, sizeof(line), "   %u", (unsigned)inv);
            oled_write_val_auto(sel == 0, line, row++);
            oled_write_auto_P(PSTR("PST:"), row++);
            {
                const char *pl = (preset == 2) ? " mac" : (preset == 1) ? " fine" : " norm";
                oled_write_val_auto(sel == 1, pl, row++);
            }
            snprintf(line, sizeof(line), " %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_rightmost(line, row++);
        } break;

        case 3: { // 4: Scroll Snap（指定体裁）
            uint8_t sel = g_ui_sel_idx[page];
            uint8_t row = 0;
            oled_write_auto_P(PSTR("SSNP"), row++);
            oled_write_auto_P(PSTR(" conf"), row++);
#if KEYBALL_SCROLLSNAP_ENABLE == 2
            keyball_scrollsnap_mode_t m = keyball_get_scrollsnap_mode();
            const char *ml = (m == KEYBALL_SCROLLSNAP_MODE_VERTICAL) ? " VRT" :
                             (m == KEYBALL_SCROLLSNAP_MODE_HORIZONTAL) ? " HOR" : " FRE";
            oled_write_auto_P(PSTR("Mode:"), row++);
            oled_write_val_auto(sel == 0, ml, row++);
            oled_write_auto_P(PSTR("Thr:"), row++);
            snprintf(line, sizeof(line), "    %u", (unsigned)KEYBALL_SCROLLSNAP_TENSION_THRESHOLD);
            oled_write_auto(line, row++);
            oled_write_auto_P(PSTR("Rst:"), row++);
            snprintf(line, sizeof(line), "  %u", (unsigned)KEYBALL_SCROLLSNAP_RESET_TIMER);
            oled_write_auto(line, row++);
#else
            oled_write_auto_P(PSTR("Mode:"), row++);
            oled_write_val_auto(sel == 0, " OFF", row++);
            oled_write_auto_P(PSTR("Thr:"), row++);
            oled_write_auto_P(PSTR("    -"), row++);
            oled_write_auto_P(PSTR("Rst:"), row++);
            oled_write_auto_P(PSTR("  -"), row++);
#endif
            snprintf(line, sizeof(line), " %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_rightmost(line, row++);
        } break;

        case 4: { // 5: Scroll Raw Monitor（指定体裁）
            uint8_t row = 0;
            oled_write_auto_P(PSTR("Scrl"), row++);
            oled_write_auto_P(PSTR(" moni"), row++);
            int16_t sx, sy, h, v;
            keyball_scroll_get_dbg(&sx, &sy, &h, &v);
            oled_write_auto_P(PSTR("sx:"), row++);
            snprintf(line, sizeof(line), "   %d", (int)sx);
            oled_write_auto(line, row++);
            oled_write_auto_P(PSTR("sy:"), row++);
            snprintf(line, sizeof(line), "   %d", (int)sy);
            oled_write_auto(line, row++);
            oled_write_auto_P(PSTR("H:"), row++);
            snprintf(line, sizeof(line), "   %d", (int)h);
            oled_write_auto(line, row++);
            oled_write_auto_P(PSTR("V:"), row++);
            snprintf(line, sizeof(line), "   %d", (int)v);
            oled_write_auto(line, row++);
            snprintf(line, sizeof(line), " %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_rightmost(line, row++);
        } break;

        case 5: { // 6: Swipe Config（指定体裁）
            uint8_t sel = g_ui_sel_idx[page];
            kb_swipe_params_t p = keyball_swipe_get_params();
            uint8_t row = 0;
            oled_write_auto_P(PSTR("Swipe"), row++);
            oled_write_auto_P(PSTR(" conf"), row++);
            oled_write_auto_P(PSTR("St:"), row++);
            snprintf(line, sizeof(line), " %u", (unsigned)p.step);
            oled_write_val_auto(sel == 0, line, row++);
            oled_write_auto_P(PSTR("Dz:"), row++);
            snprintf(line, sizeof(line), "   %u", (unsigned)p.deadzone);
            oled_write_val_auto(sel == 1, line, row++);
            oled_write_auto_P(PSTR("Rt:"), row++);
            snprintf(line, sizeof(line), "  %u", (unsigned)p.reset_ms);
            oled_write_val_auto(sel == 2, line, row++);
            oled_write_auto_P(PSTR("Frz:"), row++);
            snprintf(line, sizeof(line), "   %u", p.freeze ? 1u : 0u);
            oled_write_val_auto(sel == 3, line, row++);
            snprintf(line, sizeof(line), " %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_rightmost(line, row++);
        } break;

        case 6: { // 7: Swipe Monitor（指定体裁）
            uint32_t ar, al, ad, au;
            keyball_swipe_get_accum(&ar, &al, &ad, &au);
            uint8_t row = 0;
            oled_write_auto_P(PSTR("Swipe"), row++);
            oled_write_auto_P(PSTR(" moni"), row++);
            snprintf(line, sizeof(line), "Ac: %u", keyball_swipe_is_active() ? 1u : 0u);
            oled_write_auto(line, row++);
            snprintf(line, sizeof(line), "Md: %u", (unsigned)keyball_swipe_mode_tag());
            oled_write_auto(line, row++);
            oled_write_auto_P(PSTR("Dir:"), row++);
            {
                const char *d = (keyball_swipe_direction() == KB_SWIPE_UP) ? "  Up" :
                                 (keyball_swipe_direction() == KB_SWIPE_DOWN) ? "  Dn" :
                                 (keyball_swipe_direction() == KB_SWIPE_LEFT) ? "  Lt" :
                                 (keyball_swipe_direction() == KB_SWIPE_RIGHT) ? "  Rt" : "  Non";
                oled_write_auto_P(d, row++);
            }
            snprintf(line, sizeof(line), "U:  %u", clip0_9999(au));
            oled_write_auto(line, row++);
            snprintf(line, sizeof(line), "D:  %u", clip0_9999(ad));
            oled_write_auto(line, row++);
            snprintf(line, sizeof(line), "R:  %u", clip0_9999(ar));
            oled_write_auto(line, row++);
            snprintf(line, sizeof(line), "L:  %u", clip0_9999(al));
            oled_write_auto(line, row++);
            snprintf(line, sizeof(line), " %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_rightmost(line, row++);
        } break;

        case 7: { // 8: RGB Light
#ifdef RGBLIGHT_ENABLE
            uint8_t sel = g_ui_sel_idx[page];
            uint8_t row = 0;
            oled_write_auto_P(PSTR("RGB"), row++);
            oled_write_auto_P(PSTR(" conf"), row++);
            oled_write_auto_P(PSTR("light"), row++);
            {
                const char *onoff = rgblight_is_enabled() ? "  on" : " off";
                oled_write_val_auto(sel == 0, onoff, row++);
            }
            oled_write_auto_P(PSTR("HUE"), row++);
            {
                char v[8];
                snprintf(v, sizeof(v), " %03u", (unsigned)rgblight_get_hue());
                oled_write_val_auto(sel == 1, v, row++);
            }
            oled_write_auto_P(PSTR("SAT"), row++);
            {
                char v[8];
                snprintf(v, sizeof(v), " %03u", (unsigned)rgblight_get_sat());
                oled_write_val_auto(sel == 2, v, row++);
            }
            oled_write_auto_P(PSTR("VAL"), row++);
            {
                char v[8];
                snprintf(v, sizeof(v), " %03u", (unsigned)rgblight_get_val());
                oled_write_val_auto(sel == 3, v, row++);
            }
            oled_write_auto_P(PSTR("Mode"), row++);
            {
                char v[8];
                snprintf(v, sizeof(v), "  %02u", (unsigned)rgblight_get_mode());
                oled_write_val_auto(sel == 4, v, row++);
            }
#else
            uint8_t row = 0;
            oled_write_auto_P(PSTR("RGB"), row++);
            oled_write_auto_P(PSTR(" conf"), row++);
            oled_write_auto_P(PSTR(""), row++);
            oled_write_auto_P(PSTR("  (RGB disabled)"), row++);
#endif
            snprintf(line, sizeof(line), "  %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_auto(line, row++);
        } break;

        case 8: { // 9: Default layer conf
            uint8_t sel = g_ui_sel_idx[page];
            uint8_t row = 0;
            oled_write_auto_P(PSTR("layer"), row++);
            oled_write_auto_P(PSTR(" conf"), row++);
            oled_write_auto_P(PSTR("def:"), row++);
            {
                char b[8];
                snprintf(b, sizeof b, "   %u", (unsigned)kbpf.default_layer);
                oled_write_val_auto(sel == 0, b, row++);
            }
            snprintf(line, sizeof(line), "  %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_auto(line, row++);
        } break;

        case 9: { // 10: Send monitor
            uint8_t row = 0;
            // ヘッダ
            oled_write_auto_P(PSTR("Send"), row++);
            oled_write_auto_P(PSTR(" moni"), row++);
            // レイヤ
            oled_write_auto_P(PSTR("Lay:"), row++);
            {
                char b[8];
                snprintf(b, sizeof b, "   %u", (unsigned)keyball.last_layer);
                oled_write_auto(b, row++);
            }
            // キーコード
            oled_write_auto_P(PSTR("kc:"), row++);
            {
                char b[8];
                snprintf(b, sizeof b, "%04X", (unsigned)keyball.last_kc & 0xFFFFu);
                oled_write_auto(b, row++);
            }
            // row/col
            oled_write_auto_P(PSTR("(r,c)"), row++);
            {
                char b[12];
                snprintf(b, sizeof b, " %u, %u", (unsigned)keyball.last_pos.row, (unsigned)keyball.last_pos.col);
                oled_write_auto(b, row++);
            }
            // Mod状態（リアルタイム）
            oled_write_auto_P(PSTR("scgaC"), row++);
            {
                char b[8];
                uint8_t m = get_mods();
                led_t leds = host_keyboard_led_state();
                b[0] = (m & MOD_MASK_SHIFT) ? '+' : '-';
                b[1] = (m & MOD_MASK_CTRL)  ? '+' : '-';
                b[2] = (m & MOD_MASK_GUI)   ? '+' : '-';
                b[3] = (m & MOD_MASK_ALT)   ? '+' : '-';
                b[4] = leds.caps_lock       ? '+' : '-';
                b[5] = '\0';
                oled_write_auto(b, row++);
            }
            // ページ
            oled_write_auto_P(PSTR("page"), row++);
            snprintf(line, sizeof(line), " %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
            oled_write_rightmost(line, row++);
        } break;
    }

    g_dbg_in_render = false;
}

// Backward-compatible alias
void keyball_oled_render_debug(void) { keyball_oled_render_setting(); }

// ---------------------------------------------------------------------------
// Simple info renderers shared by keymaps

void oled_render_info_layer(void) {
    // Effective layer = highest bit of (layer_state | default_layer_state)
    uint8_t eff = get_highest_layer(layer_state | default_layer_state);
    oled_write_P("Lay: ", false);
    char b[8];
    snprintf(b, sizeof b, "    %u", (unsigned)eff);
    oled_write_P(b, false);
}

void oled_render_info_layer_default(void) {
    // Default layer (from kbpf or default_layer_state)
    uint8_t def = kbpf.default_layer;
    oled_write_P("DefL:", false);
    char b[8];
    snprintf(b, sizeof b, "    %u", (unsigned)def);
    oled_write_P(b, false);
}

void oled_render_info_ball(void) {
    oled_write_P("ball:", false);
    char b[12];
    snprintf(b, sizeof b, "x%4d", (int)keyball.last_mouse.x);
    oled_write_P(b, false);
    snprintf(b, sizeof b, "y%4d", (int)keyball.last_mouse.y);
    oled_write_P(b, false);
    snprintf(b, sizeof b, "h%4d", (int)keyball.last_mouse.h);
    oled_write_P(b, false);
    snprintf(b, sizeof b, "v%4d", (int)keyball.last_mouse.v);
    oled_write_P(b, false);
}

void oled_render_info_keycode(void) {
    oled_write_ln("kc:", false);
    char b[8];
    snprintf(b, sizeof b, " %04X", (unsigned)keyball.last_kc & 0xFFFFu);
    oled_write_P(b, false);
}

void oled_render_info_mods(void) {
    oled_write_ln("Mods:", false);
    oled_write_P("scgaC", false);
    char s[6];
    uint8_t m = get_mods();
    led_t leds = host_keyboard_led_state();
    s[0] = (m & MOD_MASK_SHIFT) ? '+' : '-';
    s[1] = (m & MOD_MASK_CTRL)  ? '+' : '-';
    s[2] = (m & MOD_MASK_GUI)   ? '+' : '-';
    s[3] = (m & MOD_MASK_ALT)   ? '+' : '-';
    s[4] = leds.caps_lock       ? '+' : '-';
    s[5] = '\0';
    oled_write_P(s, false);
}

void oled_render_info_cpi(void) {
    oled_write_ln("cpi:", false);
    char b[8];
    snprintf(b, sizeof b, " %u", (unsigned)keyball_get_cpi());
    oled_write_P(b, false);
}

void oled_render_info_scroll_step(void) {
    oled_write_ln("scr:", false);
    char b[8];
    snprintf(b, sizeof b, "    %u", (unsigned)keyball_get_scroll_div());
    oled_write_P(b, false);
    // invert flag (per OS)
    oled_write_P("inv:", false);
    uint8_t inv = kbpf.scroll_invert[keyball_os_idx()] ? 1u : 0u;
    oled_write_ln(inv ? "+" : "-", false);
}

void oled_render_info_swipe_tag(void) {
    oled_write_P("SW_t:", false);
    char b[8];
    snprintf(b, sizeof b, "    %u", (unsigned)keyball_swipe_mode_tag());
    oled_write_P(b, false);
}

void oled_render_info_key_pos(void) {
    oled_write_ln("r,c:", false);
    char b[12];
    snprintf(b, sizeof b, " %u, %u", (unsigned)keyball.last_pos.row, (unsigned)keyball.last_pos.col);
    oled_write_P(b, false);
}

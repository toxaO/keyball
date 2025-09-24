#include <stdint.h>
#include <stdbool.h>
#include "quantum.h"
#include "timer.h"
#include "oled_driver.h"

#include "keyball_oled.h"
#include "keyball.h"
#include <stdio.h>
#include <stdarg.h>
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
        case 0: return 5; // CPI, Glo, Th1, Th2, Dz
        case 1: return 4; // AML: en, TO, TH, TG_L
        case 2: return 7; // Scroll: ST, Dz, Inv, S_On, S_Ly, PST, H_Ga
        case 3: return 3; // SSNP: Mode, Thr, Rst を編集可能
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

// 5桁固定の簡易描画ユーティリティ（snprintf を使い幅で揃える）
static inline uint8_t kb_line_width(void) { return 5; }

static void oled_writef(uint8_t row, const char* fmt, ...) {
    char tmp[32];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    char out[6];
    // 左詰め・最大5文字
    snprintf(out, sizeof(out), "%-5.5s", tmp);
    oled_set_cursor(0, row);
    oled_write(out, false);
}

// 空行を n 行分スキップ
#define row_skip(r, n) do { (r) += (uint8_t)(n); } while (0)

static void oled_writef_sel(uint8_t row, bool selected, const char* fmt, ...) {
    char tmp[32];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    char out[6];
    out[0] = selected ? '>' : ' ';
    // 選択フラグ分を引いて残り4桁で詰める
    snprintf(out + 1, sizeof(out) - 1, "%-4.4s", tmp);
    oled_set_cursor(0, row);
    oled_write(out, false);
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

    // ページ送り / 値調整（Shift+←/→で値、非Shiftでページ）
    if (keycode == KC_LEFT || keycode == KC_RIGHT) {
        // 非Shift: ページ移動（内部でデバウンスするためここでは呼ばない）
        if ((get_mods() & MOD_MASK_SHIFT) == 0) {
            if (keycode == KC_LEFT) {
                keyball_oled_prev_page();
            } else {
                keyball_oled_next_page();
            }
            return true; // ホストへは送らない
        }
        // Shift併用: 値調整
        if (!ui_op_ready()) return true; // 入力は消費
        int dir = (keycode == KC_RIGHT) ? +1 : -1; // S+R: 増加, S+L: 減少

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
                } else if (sel == 3) {
                    // Th2 ±1 (th1+1 .. 63)
                    int32_t th1 = (int32_t)kbpf.move_th1[os];
                    int32_t th2 = (int32_t)kbpf.move_th2[os] + dir;
                    if (th2 < th1 + 1) th2 = th1 + 1;
                    if (th2 > 63) th2 = 63;
                    kbpf.move_th2[os] = (uint8_t)th2;
                    g_move_th2        = (int16_t)th2;
                } else if (sel == 4) {
                    // Dz: Move deadzone 0..9（UIは1桁表示。内部は0..32対応）
                    int32_t v = (int32_t)kbpf.move_deadzone + dir;
                    if (v < 0) v = 0;
                    if (v > 9) v = 9;
                    kbpf.move_deadzone = (uint8_t)v;
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
                int32_t th = (int32_t)kbpf.aml_threshold + dir * 50;
                    if (th < 50) th = 50;
                    if (th > 1000) th = 1000;
                    kbpf.aml_threshold = (uint16_t)th;
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
                    // ST: 1..7（内部でクランプ）
                    int32_t nv = (int32_t)keyball_get_scroll_div() + dir;
                    if (nv < 1) nv = 1; // 下限のガード（set側でもクランプされる）
                    keyball_set_scroll_div((uint8_t)nv);
                } else if (sel == 1) {
                    // Scroll Dz: 0..32
                    int32_t v = (int32_t)kbpf.scroll_deadzone + dir;
                    if (v < 0) v = 0;
                    if (v > 9) v = 9;
                    kbpf.scroll_deadzone = (uint8_t)v;
                } else if (sel == 2) {
                    // Inv: 0/1 toggle
                    uint8_t v = kbpf.scroll_invert[os] ? 1 : 0;
                    v = (dir > 0) ? 1 : 0;
                    kbpf.scroll_invert[os] = v;
                } else if (sel == 3) {
                    // Scroll layer enable: 0/1
                    bool en = kbpf.scroll_layer_enable ? true : false;
                    en = (dir > 0);
                    kbpf.scroll_layer_enable = en ? 1 : 0;
                    if (en) {
                        keyball_refresh_scroll_layer();
                    } else {
                        keyball_set_scroll_mode(false);
                    }
                } else if (sel == 4) {
                    // Scroll layer index: 0..31
                    int32_t layer = (int32_t)kbpf.scroll_layer_index + dir;
                    if (layer < 0) layer = 0;
                    if (layer > 31) layer = 31;
                    kbpf.scroll_layer_index = (uint8_t)layer;
                    keyball_refresh_scroll_layer();
                } else if (sel == 5) {
                    // PST: 0..2 cycle（norm/fine/mac）
                    int32_t p = (int32_t)kbpf.scroll_preset[os] + dir;
                    if (p < 0) p = 0;
                    if (p > 2) p = 2;
                    kbpf.scroll_preset[os] = (uint8_t)p;
                } else if (sel == 6) {
                    // H_Ga: 1..100 (%), 1刻み
                    int32_t v = (int32_t)kbpf.scroll_hor_gain_pct + dir * 1;
                    if (v < 1) v = 1;
                    if (v > 100) v = 100;
                    kbpf.scroll_hor_gain_pct = (uint8_t)v;
                }
            } break;

            case 3: { // Scroll Snap
#if KEYBALL_SCROLLSNAP_ENABLE == 2
                uint8_t sel = g_ui_sel_idx[page];
                if (sel == 0) {
                    keyball_scrollsnap_mode_t m = keyball_get_scrollsnap_mode();
                    int32_t nm = (int32_t)m + dir;
                    if (nm < 0) nm = 0;
                    if (nm > 2) nm = 2;
                    keyball_set_scrollsnap_mode((keyball_scrollsnap_mode_t)nm);
                    kbpf.scrollsnap_mode = (uint8_t)nm;
                } else if (sel == 1) {
                    int32_t v = (int32_t)kbpf.scrollsnap_thr + dir * 10; // ±10
                    if (v < 0) v = 0;
                    if (v > 400) v = 400;
                    kbpf.scrollsnap_thr = (uint16_t)v;
                } else if (sel == 2) {
                    int32_t v = (int32_t)kbpf.scrollsnap_rst_ms + dir * 100; // ±100ms
                    if (v < 100) v = 100;
                    if (v > 5000) v = 5000;
                    kbpf.scrollsnap_rst_ms = (uint16_t)v;
                }
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

    if (is_keyboard_master()) {
        g_oled_vertical = true;
        return OLED_ROTATION_270;
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
                oled_writef(row++, "mouse"); // row 1
                oled_writef(row++, " conf"); // row 2
                row_skip(row, 1);            // row 3
                oled_writef(row++, "cpi:");  // row 4
                oled_writef_sel(row++, sel == 0, "%u", (unsigned)keyball_get_cpi()); // row 5
                row_skip(row, 1);            // row 6
#ifdef KEYBALL_MOVE_SHAPING_ENABLE
                {
                    uint8_t  g   = kbpf.move_gain_lo_fp[keyball_os_idx()];
                    uint16_t pct = (uint16_t)((g * 100u + 127u) / 255u); // 四捨五入
                    oled_writef(row++, "Glo:"); // row 7
                    oled_writef_sel(row++, sel == 1, " %u%%", (unsigned)pct); // row 8
                }
                row_skip(row, 1); // row 9
                oled_writef(row++, "Th:"); // row 10
                oled_writef_sel(row++, sel == 2, "1:%2u", (unsigned)kbpf.move_th1[keyball_os_idx()]); // row 11
                oled_writef_sel(row++, sel == 3, "2:%2u", (unsigned)kbpf.move_th2[keyball_os_idx()]); // row 12
                row_skip(row, 1); // row 13
                // Move deadzone（0..9 表示）
                oled_writef_sel(row++, sel == 4, "DZ:%1u", (unsigned)kbpf.move_deadzone);
#else
                oled_writef(row++, "Glo:");
                oled_writef_sel(row++, sel == 1, "OFF");
                row_skip(row, 1);
                oled_writef(row++, "Th1:");
                oled_writef_sel(row++, sel == 2, "-");
                row_skip(row, 1);
                oled_writef(row++, "Th2:");
                oled_writef_sel(row++, sel == 3, "-");
                row_skip(row, 1);
#endif
            // 追加: ポインタ移動量
            // {
            //     char b[12];
            //     snprintf(b, sizeof b, "x:%3d", (int)keyball.last_mouse.x);
            //     oled_writef(row++, "%s", b); // row 13
            //     snprintf(b, sizeof b, "y:%3d", (int)keyball.last_mouse.y);
            //     oled_writef(row++, "%s", b); // row 14 or 16 depending
            // }
            row_skip(row, 1); // 調整: 末尾の空行を1行に
            // ページ表示
            oled_writef(row++, "%2u/%2u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count()); // row 16
        } break;

        case 1: { // 2: Auto Mouse Layer（指定体裁）
            uint8_t sel = g_ui_sel_idx[page];
            uint8_t row = 0;
            oled_writef(row++, "AML");
            oled_writef(row++, " conf");
            row_skip(row, 1); // row 12
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
            oled_writef(row++, "en:");
            oled_writef_sel(row++, sel == 0, " %3u", get_auto_mouse_enable() ? 1u : 0u);
            row_skip(row, 1); // row 12
            oled_writef(row++, "TO:");
            {
                uint32_t to = (uint32_t)get_auto_mouse_timeout();
                if (to >= 60000u) {
                    oled_writef_sel(row++, sel == 1, "HOLD");
                } else {
                    oled_writef_sel(row++, sel == 1, "%u", (unsigned)to);
                }
            }
            row_skip(row, 1); // row 12
            oled_writef(row++, "TH:");
            {
                oled_writef_sel(row++, sel == 2, " %4u", (unsigned)kbpf.aml_threshold);
            }
            row_skip(row, 1); // row 12
            oled_writef(row++, "TG_L:");
            oled_writef_sel(row++, sel == 3, " %3u", (unsigned)get_auto_mouse_layer());
            row_skip(row, 1); // row 12
#else
            oled_writef(row++, "en:");
            oled_writef_sel(row++, sel == 0, "0");
            row_skip(row, 1); // row 12
            oled_writef(row++, "TO:");
            oled_writef_sel(row++, sel == 1, "0");
            row_skip(row, 1); // row 12
            oled_writef(row++, "TH:");
            oled_writef_sel(row++, sel == 2, "0");
            row_skip(row, 1); // row 12
            oled_writef(row++, "TG_L:");
            oled_writef_sel(row++, sel == 3, "0");
            row_skip(row, 1); // row 12
#endif
            oled_writef(row++, " %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
        } break;

        case 2: { // 3: Scroll Param（指定体裁）
            uint8_t sel = g_ui_sel_idx[page];
            uint8_t row = 0;
            oled_writef(row++, "Scrl");
            oled_writef(row++, " conf");
            row_skip(row, 1); // row 12
            uint8_t os = keyball_os_idx();
            uint8_t inv = kbpf.scroll_invert[os] ? 1 : 0;
            uint8_t preset = kbpf.scroll_preset[os];
            // ST（スクロールステップ）
            oled_writef_sel(row++, sel == 0, "ST:%1u", (unsigned)keyball_get_scroll_div());
            // Scroll deadzone
            oled_writef_sel(row++, sel == 1, "Dz:%1u", (unsigned)kbpf.scroll_deadzone);
            // inverse
            oled_writef_sel(row++, sel == 2, "Iv:%1u", (unsigned)inv);
            // Scroll layer auto togglet
            oled_writef(row++, "ScLy:");
            oled_writef_sel(row++, sel == 3, " %3s", (unsigned)(kbpf.scroll_layer_enable ? "on" : "off"));
            oled_writef(row++, "LNo.:");
            oled_writef_sel(row++, sel == 4, " %02u", (unsigned)kbpf.scroll_layer_index);
            {
                const char *pl = (preset == 2) ? "m" : (preset == 1) ? "f" : "n";
                oled_writef_sel(row++, sel == 5, "Md:%s", pl);
            }
            oled_writef(row++, "H_Ga:");
            {
                uint8_t pct = kbpf.scroll_hor_gain_pct; // 1..100
                // ちょうど4桁で収まる "%3u%%" を使う（例: "100%", " 99%", "  1%"）
                oled_writef_sel(row++, sel == 6, "%3u%%", (unsigned)pct);
            }
            row_skip(row, 2);
            oled_writef(row++, " %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
        } break;

        case 3: { // 4: Scroll Snap（指定体裁）
            uint8_t sel = g_ui_sel_idx[page];
            uint8_t row = 0;
            oled_writef(row++, "SSNP");
            oled_writef(row++, " conf");
            row_skip(row, 1); // row 12
#if KEYBALL_SCROLLSNAP_ENABLE == 2
            keyball_scrollsnap_mode_t m = keyball_get_scrollsnap_mode();
            const char *ml = (m == KEYBALL_SCROLLSNAP_MODE_VERTICAL) ? " VRT" :
                             (m == KEYBALL_SCROLLSNAP_MODE_HORIZONTAL) ? " HOR" : " FRE";
            oled_writef(row++, "Mode:");
            oled_writef_sel(row++, sel == 0, "%s", ml);
            row_skip(row, 1); // row 12
            oled_writef(row++, "Thr:");
            oled_writef_sel(row++, sel == 1, " %3u", (unsigned)kbpf.scrollsnap_thr);
            row_skip(row, 1); // row 12
            oled_writef(row++, "Rst:");
            oled_writef_sel(row++, sel == 2, "%4u", (unsigned)kbpf.scrollsnap_rst_ms);
            row_skip(row, 1); // row 12
#else
            oled_writef(row++, "Mode:");
            oled_writef_sel(row++, sel == 0, "OFF");
            row_skip(row, 1); // row 12
            oled_writef(row++, "Thr:");
            oled_writef(row++, " -");
            row_skip(row, 1); // row 12
            oled_writef(row++, "Rst:");
            oled_writef(row++, " -");
            row_skip(row, 1); // row 12
#endif
            row_skip(row, 3); // row 12
            oled_writef(row++, " %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
        } break;

        case 4: { // 5: Scroll Raw Monitor（指定体裁）
            uint8_t row = 0;
            oled_writef(row++, "Scrl");
            oled_writef(row++, " moni");
            row_skip(row, 1); // row 12
            int16_t sx, sy, h, v;
            int32_t ah, av;
            int8_t  t;
            keyball_scroll_get_dbg(&sx, &sy, &h, &v);
            keyball_scroll_get_dbg_inner(&ah, &av, &t);
            oled_writef(row++, "x:%3d", (int)sx);
            oled_writef(row++, "y:%3d", (int)sy);
            oled_writef(row++, "h:%3d", (int)h);
            oled_writef(row++, "v:%3d", (int)v);
            oled_writef(row++, "ah:");oled_writef(row++, " %4d", (int)ah);
            oled_writef(row++, "av:");oled_writef(row++, " %4d", (int)av);
            oled_writef(row++, "t:%3d", (int)t);
            row_skip(row, 3); // row 12
            oled_writef(row++, " %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
        } break;

        case 5: { // 6: Swipe Config（指定体裁）
            uint8_t sel = g_ui_sel_idx[page];
            kb_swipe_params_t p = keyball_swipe_get_params();
            uint8_t row = 0;
            oled_writef(row++, "Swipe");
            oled_writef(row++, " conf");
            row_skip(row, 1); // row 12
            oled_writef(row++, "St:");
            oled_writef_sel(row++, sel == 0, " %u", (unsigned)p.step);
            row_skip(row, 1); // row 12
            oled_writef(row++, "Dz:");
            oled_writef_sel(row++, sel == 1, " %3u", (unsigned)p.deadzone);
            row_skip(row, 1); // row 12
            oled_writef(row++, "Rt:");
            oled_writef_sel(row++, sel == 2, " %3u", (unsigned)p.reset_ms);
            row_skip(row, 1); // row 12
            oled_writef(row++, "Frz:");
            oled_writef_sel(row++, sel == 3, " %3u", p.freeze ? 1u : 0u);
            row_skip(row, 1); // row 12
            oled_writef(row++, " %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
        } break;

        case 6: { // 7: Swipe Monitor（指定体裁）
            uint32_t ar, al, ad, au;
            keyball_swipe_get_accum(&ar, &al, &ad, &au);
            uint8_t row = 0;
            oled_writef(row++, "Swipe");
            oled_writef(row++, " moni");
            row_skip(row, 1); // row 12
            snprintf(line, sizeof(line), "Ac: %u", keyball_swipe_is_active() ? 1u : 0u);
            oled_writef(row++, "%s", line);
            row_skip(row, 1); // row 12
            snprintf(line, sizeof(line), "Md: %u", (unsigned)keyball_swipe_mode_tag());
            oled_writef(row++, "%s", line);
            row_skip(row, 1); // row 12
            oled_writef(row++, "Dir:");
            {
                const char *d = (keyball_swipe_direction() == KB_SWIPE_UP) ? "Up" :
                                 (keyball_swipe_direction() == KB_SWIPE_DOWN) ? "Dn" :
                                 (keyball_swipe_direction() == KB_SWIPE_LEFT) ? "Lt" :
                                 (keyball_swipe_direction() == KB_SWIPE_RIGHT) ? "Rt" : "Non";
                oled_writef(row++, "  %s", d);
            }
            row_skip(row, 1); // row 12
            oled_writef(row++, "U:%3u", clip0_9999(au));
            oled_writef(row++, "D:%3u", clip0_9999(ad));
            oled_writef(row++, "R:%3u", clip0_9999(ar));
            oled_writef(row++, "L:%3u", clip0_9999(al));
            row_skip(row, 1); // row 12
            oled_writef(row++, " %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
        } break;

        case 7: { // 8: RGB Light
#ifdef RGBLIGHT_ENABLE
            uint8_t sel = g_ui_sel_idx[page];
            uint8_t row = 0;
            oled_writef(row++, "RGB");
            oled_writef(row++, " conf");
            row_skip(row, 1); // row 12
            oled_writef(row++, "light");
            oled_writef_sel(row++, sel == 0, "%s", rgblight_is_enabled() ? "  on" : " off");
            row_skip(row, 1); // row 12
            oled_writef(row++, "HUE");
            {
                char v[8];
                snprintf(v, sizeof(v), " %03u", (unsigned)rgblight_get_hue());
                oled_writef_sel(row++, sel == 1, "%s", v);
            }
            oled_writef(row++, "SAT");
            {
                char v[8];
                snprintf(v, sizeof(v), " %03u", (unsigned)rgblight_get_sat());
                oled_writef_sel(row++, sel == 2, "%s", v);
            }
            oled_writef(row++, "VAL");
            {
                char v[8];
                snprintf(v, sizeof(v), " %03u", (unsigned)rgblight_get_val());
                oled_writef_sel(row++, sel == 3, "%s", v);
            }
            row_skip(row, 1); // row 12
            oled_writef(row++, "Mode");
            {
                char v[8];
                snprintf(v, sizeof(v), "  %02u", (unsigned)rgblight_get_mode());
                oled_writef_sel(row++, sel == 4, "%s", v);
            }
#else
            uint8_t row = 0;
            oled_writef(row++, "RGB");
            row_skip(row, 1); // row 12
            oled_writef(row++, " conf");
            oled_writef(row++, "");
            row_skip(row, 1); // row 12
            oled_writef(row++, "(dis)");
            row_skip(row, 1); // row 12
#endif
            oled_writef(row++, " %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
        } break;

        case 8: { // 9: Default layer conf
            uint8_t sel = g_ui_sel_idx[page];
            uint8_t row = 0;
            oled_writef(row++, "layer");
            oled_writef(row++, " conf");
            row_skip(row, 1); // row 12
            oled_writef(row++, "def:");
            {
                char b[8];
                snprintf(b, sizeof b, "   %u", (unsigned)kbpf.default_layer);
                oled_writef_sel(row++, sel == 0, "%s", b);
            }
            row_skip(row, 10); // row 12
            oled_writef(row++, " %u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
        } break;

        case 9: { // 10: Send monitor
            uint8_t row = 0;
            // ヘッダ
            oled_writef(row++, "Send");
            oled_writef(row++, " moni");
            row_skip(row, 1); // row 12
            // レイヤ
            oled_writef(row++, "Lay:");
            {
                char b[8];
                snprintf(b, sizeof b, "   %u", (unsigned)keyball.last_layer);
                oled_writef(row++, "%s", b);
            }
            row_skip(row, 1); // row 12
            // キーコード
            oled_writef(row++, "kc:");
            {
                char b[8];
                snprintf(b, sizeof b, "%04X", (unsigned)keyball.last_kc & 0xFFFFu);
                oled_writef(row++, " %s", b);
            }
            row_skip(row, 1); // row 12
            // row/col
            oled_writef(row++, "(r,c)");
            {
                char b[12];
                snprintf(b, sizeof b, " %u, %u", (unsigned)keyball.last_pos.row, (unsigned)keyball.last_pos.col);
                oled_writef(row++, "%s", b);
            }
            row_skip(row, 1); // row 12
            // Mod状態（リアルタイム）
            oled_writef(row++, "scgaC");
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
                oled_writef(row++, "%s", b);
            }
            row_skip(row, 1); // row 12
            // ページ
            oled_writef(row++, "%u/%u", (unsigned)(page + 1), (unsigned)keyball_oled_get_page_count());
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
    oled_write_P("Mods:", false);
    oled_write_P("scgaC", false);

    char active[6];
    uint8_t mods = get_mods();
    led_t leds = host_keyboard_led_state();
    active[0] = (mods & MOD_MASK_SHIFT) ? '+' : '-';
    active[1] = (mods & MOD_MASK_CTRL)  ? '+' : '-';
    active[2] = (mods & MOD_MASK_GUI)   ? '+' : '-';
    active[3] = (mods & MOD_MASK_ALT)   ? '+' : '-';
    active[4] = leds.caps_lock          ? '+' : '-';
    active[5] = '\0';
    oled_write_P(active, false);

    uint8_t osm = get_oneshot_mods();
    uint8_t osl = get_oneshot_locked_mods();
    char oneshot[5];
    oneshot[0] = (osm & MOD_MASK_SHIFT) ? '+' : '-';
    oneshot[1] = (osm & MOD_MASK_CTRL)  ? '+' : '-';
    oneshot[2] = (osm & MOD_MASK_GUI)   ? '+' : '-';
    oneshot[3] = (osm & MOD_MASK_ALT)   ? '+' : '-';
    oneshot[4] = '\0';
    oled_write_P(" OSM:", false);
    oled_write_P(oneshot, false);

    char locked[5];
    locked[0] = (osl & MOD_MASK_SHIFT) ? '+' : '-';
    locked[1] = (osl & MOD_MASK_CTRL)  ? '+' : '-';
    locked[2] = (osl & MOD_MASK_GUI)   ? '+' : '-';
    locked[3] = (osl & MOD_MASK_ALT)   ? '+' : '-';
    locked[4] = '\0';
    oled_write_P(" LCK:", false);
    oled_write_P(locked, false);
}

void oled_render_info_cpi(void) {
    oled_write_ln("cpi:", false);
    char b[8];
    snprintf(b, sizeof b, " %u", (unsigned)keyball_get_cpi());
    oled_write_P(b, false);
}

void oled_render_info_scroll_step(void) {
    oled_write_P("scr:", false);
    char b[8];
    snprintf(b, sizeof b, "%u", (unsigned)keyball_get_scroll_div());
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

// ---------------------------------------------------------------------------
// Backward-compatible wrappers (keyball_oled_* -> oled_render_info_*)

void keyball_oled_render_keyinfo(void) {
    oled_render_info_keycode();
    oled_render_info_mods();
}

void keyball_oled_render_ballinfo(void) {
    oled_render_info_ball();
}

void keyball_oled_render_layerinfo(void) {
    oled_render_info_layer();
    oled_render_info_layer_default();
}

void keyball_oled_render_ballsubinfo(void) {
    oled_render_info_key_pos();
}

#include "pointing.h"
#include "quantum.h"
#include <string.h>

static const char* pointer_kind_to_string(hk_pointer_kind kind) {
    switch (kind) {
        case POINTER_KIND_NONE:
            return "NONE \xB1";
        case POINTER_KIND_PIMORONI_TRACKBALL:
            return "PIM  \xB1";
        case POINTER_KIND_TRACKPOINT:
            return "TP   \xB1";
        case POINTER_KIND_CIRQUE35:
            return "CR35 \xB1";
        case POINTER_KIND_CIRQUE40:
            return "CR40 \xB1";
        case POINTER_KIND_TPS43:
            return "TPS43\xB1";
        default:
            return "?????\xB1";
    }
}

static const char BL = '\xB0'; // Blank indicator character
static const char LFSTR_ON[] PROGMEM = "\xB2\xB3";
static const char LFSTR_OFF[] PROGMEM = "\xB4\xB5";

static const char *format_3d(int8_t d) {
    static char buf[10] = {0};
    char        lead   = ' ';
    if (d < 0) {
        d    = -d;
        lead = '-';
    }
    sprintf(buf, "%c%2d", lead, d);
    return buf;
}

static const char *format_2d(int8_t d) {
    static char buf[10] = {0};
    if (d > 99) {
        d = 99;
    }
    sprintf(buf, "%02d",d);
    return buf;
}

static char to_1x(uint8_t x) {
    x &= 0x0f;
    return x < 10 ? x + '0' : x + 'a' - 10;
}

static const char* format_multiplier(float f) {
    static char buf[10] = {0};
    if (f > 10 || f < -10) {
        sprintf(buf, "err");
    } else {
        sprintf(buf, "%2.1f", f);
    }
    return buf;
}

void hk_oled_render_pointer_state(void) {
    // Output example:
    //
    //  TPS43: -12  34   0   0
    //  CUR D: 1.0/5  ON LK:VT

    // 1st line, pointing device kind, mouse x, y, h, and v.
    oled_write_P(pointer_kind_to_string(g_hk_state.main.pointer_kind), false);

    oled_write(format_3d(g_hk_state.display.last_mouse.x), false);
    oled_write(format_3d(g_hk_state.display.last_mouse.y), false);
    oled_write(format_3d(g_hk_state.display.last_mouse.h), false);
    oled_write_ln(format_3d(g_hk_state.display.last_mouse.v), false);

    // 2nd line, cursor mode, default multiplier, drag scroll mode, scroll lock mode, and scroll buffer size.
    if (g_hk_state.setting_default_scale) {
        oled_write_P(PSTR("CUR D\xB1"), false);
        oled_write(format_multiplier(g_hk_state.main.pointer_default_multiplier), false);
    } else if (g_hk_state.setting_sniping_scale) {
        oled_write_P(PSTR("CUR S\xB1"), false);
        oled_write(format_multiplier(g_hk_state.main.pointer_sniping_multiplier), false);
    } else {
        switch (g_hk_state.main.cursor_mode)
        {
            case CURSOR_MODE_DEFAULT:
                oled_write_P(PSTR("CUR D\xB1"), false);
                oled_write(format_multiplier(g_hk_state.main.pointer_default_multiplier), false);
                break;
            case CURSOR_MODE_SNIPING:
                oled_write_P(PSTR("CUR S\xB1"), false);
                oled_write(format_multiplier(g_hk_state.main.pointer_sniping_multiplier), false);
                break;
            default:
                oled_write_P(PSTR("CUR ?\xB1"), false);
                break;
        }
    }

    oled_write_char('/', false);
    // scroll buffer size:
    oled_write(format_2d(g_hk_state.main.pointer_scroll_buffer_size), false);
    oled_write_char(' ', false);

    // drag scroll mode: on/off
    if (g_hk_state.main.drag_scroll) {
        oled_write_P(LFSTR_ON, false);
    } else {
        oled_write_P(LFSTR_OFF, false);
    }

    // scroll lock mode: "VT" (vertical), "HN" (horiozntal), and "NO" (free)
    switch (g_hk_state.main.scroll_lock) {
        case SCROLL_LOCK_VERTICAL:
            oled_write_ln_P(PSTR(" L:VT"), false);
            break;
        case SCROLL_LOCK_HORIZONTAL:
            oled_write_ln_P(PSTR(" L:HO"), false);
            break;
        default:
            oled_write_ln_P(PSTR(" L:NO"), false);
            break;
    }
}

void hk_oled_render_keyinfo(void) {
    // Format: `Key :  R{row}  C{col} K{kc} {name}{name}{name}`
    //
    // Where `kc` is lower 8 bit of keycode.
    // Where `name`s are readable labels for pressing keys, valid between 4 and 56.
    //
    // `row`, `col`, and `kc` indicates the last processed key,
    // but `name`s indicate unreleased keys in best effort.
    //
    // For example:
    //
    //     Key  :  R2  C3 K06 abc
    //     TPS43:   0   0   0   0

    // "Key" Label.
    oled_write_P(PSTR("Key  \xB1"), false);

    // Row and column.
    oled_write_char('\xB8', false);
    oled_write_char(to_1x(g_hk_state.display.last_pos.row), false);
    oled_write_char('\xB9', false);
    oled_write_char(to_1x(g_hk_state.display.last_pos.col), false);

    // Keycode.
    oled_write_P(PSTR("\xBA\xBB"), false);
    oled_write_char(to_1x(g_hk_state.display.last_kc >> 4), false);
    oled_write_char(to_1x(g_hk_state.display.last_kc), false);

    // Keys currently pressed.
    oled_write_P(PSTR(" "), false);
    oled_write(g_hk_state.display.pressing_keys, false);
}

void hk_oled_render_layerinfo(void) {
    // Format: `Layer:{layer state}`
    //
    // Output example:
    //
    //     Layer:-23------------
    //
    oled_write_P(PSTR("Layer\xB1"), false);
    for (uint8_t i = 1; i < 8; i++) {
        oled_write_char((layer_state_is(i) ? to_1x(i) : BL), false);
    }
    oled_write_char(' ', false);

#    ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
    oled_write_P(PSTR("\xC2\xC3"), false);
    if (get_auto_mouse_enable()) {
        oled_write_P(LFSTR_ON, false);
    } else {
        oled_write_P(LFSTR_OFF, false);
    }

    oled_write(format_3d(get_auto_mouse_timeout()), false);
    oled_write_char('0', false);
#    else
    oled_write_P(PSTR("\xC2\xC3\xB4\xB5 --"), false);
#    endif
}

bool oled_task_user(void) {
    if (g_hk_state.init) {
        hk_oled_render_keyinfo();
        hk_oled_render_pointer_state();
        hk_oled_render_layerinfo();
    }
    return true;
}

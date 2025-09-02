#include "hk_debug.h"

#include "print.h"

const char* hk_scroll_mode_to_string(hk_scroll_lock mode) {
    switch (mode) {
        case SCROLL_LOCK_OFF:
            return "off";
        case SCROLL_LOCK_HORIZONTAL:
            return "horizontal";
        case SCROLL_LOCK_VERTICAL:
            return "vertical";
        default:
            return "unknown";
    }
}

const char* hk_cursor_mode_to_string(hk_cursor_mode mode) {
    switch (mode) {
        case CURSOR_MODE_DEFAULT:
            return "default";
        case CURSOR_MODE_SNIPING:
            return "sniping";
        default:
            return "unknown";
    }
}

const char* hk_pointer_kind_to_string(hk_pointer_kind kind) {
    switch (kind) {
        case POINTER_KIND_NONE:
            return "none";
        case POINTER_KIND_PIMORONI_TRACKBALL:
            return "pimoroni_trackball";
        case POINTER_KIND_TRACKPOINT:
            return "trackpoint";
        case POINTER_KIND_CIRQUE35:
            return "cirque35";
        case POINTER_KIND_CIRQUE40:
            return "cirque40";
        case POINTER_KIND_TPS43:
            return "tps43";
        default:
            return "unknown";
    }
}

void debug_hk_mouse_report(const char* prefix, const report_mouse_t* report) {
    #    ifdef CONSOLE_ENABLE
        printf("debug_hk_mouse_report (%s): { x=%d y=%d v=%d h=%d }\n",
                prefix,
                report->x, report->y, report->v, report->h
        );
    #    endif // CONSOLE_ENABLE
}

void debug_hk_state_to_console(hk_state_t* state) {
    #    ifdef CONSOLE_ENABLE
        printf("debug_hk: state = {\n"
                "\tis_main_side=%u\n"
                "\tsetting_default_scale=%u\n"
                "\tsetting_sniping_scale=%u\n"
                "\tsetting_scroll_buffer=%u\n"
                "\tmain=\n"
                "\t{\n"
                "\t\tpointer_kind=%s\n"
                "\t\tcursor_mode=%s\n"
                "\t\tdrag_scroll=%u\n"
                "\t\tscroll_lock=%s\n"
                "\t\tpointer_default_multiplier=%.2f\n"
                "\t\tpointer_sniping_multiplier=%.2f\n"
                "\t\tpointer_scroll_buffer_size=%u\n"
                "\t}\n"
                "\tperipheral=\n"
                "\t{\n"
                "\t\tpointer_kind=%s\n"
                "\t\tcursor_mode=%s\n"
                "\t\tdrag_scroll=%u\n"
                "\t\tscroll_lock=%s\n"
                "\t\tpointer_default_multiplier=%.2f\n"
                "\t\tpointer_sniping_multiplier=%.2f\n"
                "\t\tpointer_scroll_buffer_size=%u\n"
                "\t}\n"
                "}\n",
                state->is_main_side, state->setting_default_scale, state->setting_sniping_scale, state->setting_scroll_buffer,
                hk_pointer_kind_to_string(state->main.pointer_kind), hk_cursor_mode_to_string(state->main.cursor_mode), state->main.drag_scroll, hk_scroll_mode_to_string(state->main.scroll_lock),
                state->main.pointer_default_multiplier, state->main.pointer_sniping_multiplier, state->main.pointer_scroll_buffer_size,
                hk_pointer_kind_to_string(state->peripheral.pointer_kind), hk_cursor_mode_to_string(state->peripheral.cursor_mode), state->peripheral.drag_scroll, hk_scroll_mode_to_string(state->peripheral.scroll_lock),
                state->peripheral.pointer_default_multiplier, state->peripheral.pointer_sniping_multiplier, state->peripheral.pointer_scroll_buffer_size
        );
    #    endif // CONSOLE_ENABLE
}

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "holykeebs.h"

#include "eeconfig.h"
#include "pointing_device.h"
#include "report.h"
#include "color.h"

#include "pointing.h"
#include "pimoroni.h"
#include "trackpoint.h"
#include "hk_debug.h"
#include "eeprom_config.h"

#ifdef HK_SPLIT_SYNC_STATE
#include "rpc.h"
#include "transactions.h"
#endif

#define _CONSTRAIN(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
#define CONSTRAIN_XY(val)      (mouse_xy_report_t) _CONSTRAIN(val, MOUSE_REPORT_XY_MIN, MOUSE_REPORT_XY_MAX)
#define CONSTRAIN_HV(val)      (mouse_hv_report_t) _CONSTRAIN(val, MOUSE_REPORT_HV_MIN, MOUSE_REPORT_HV_MAX)

static const char BL = '\xB0'; // Blank indicator character

hk_state_t g_hk_state = {0};
hk_eeprom_config_t hk_eeprom_config;

static void deserialize_eeconfig_to_state(const hk_eeprom_config_t* config) {
    g_hk_state.main.cursor_mode = config->pointing.main_cursor_mode;
    g_hk_state.main.drag_scroll = config->pointing.main_drag_scroll;
    g_hk_state.main.scroll_lock = config->pointing.main_scroll_lock;
    g_hk_state.main.pointer_default_multiplier = config->pointing.main_default_multiplier / 100.0;
    g_hk_state.main.pointer_sniping_multiplier = config->pointing.main_sniping_multiplier / 100.0;
    g_hk_state.main.pointer_scroll_buffer_size = config->pointing.main_scroll_buffer_size;

    g_hk_state.peripheral.cursor_mode = config->pointing.peripheral_cursor_mode;
    g_hk_state.peripheral.drag_scroll = config->pointing.peripheral_drag_scroll;
    g_hk_state.peripheral.scroll_lock = config->pointing.peripheral_scroll_lock;
    g_hk_state.peripheral.pointer_default_multiplier = config->pointing.peripheral_default_multiplier / 100.0;
    g_hk_state.peripheral.pointer_sniping_multiplier = config->pointing.peripheral_sniping_multiplier / 100.0;
    g_hk_state.peripheral.pointer_scroll_buffer_size = config->pointing.peripheral_scroll_buffer_size;
}

static void serialize_state_to_eeconfig(hk_eeprom_config_t* config) {
    config->pointing.main_cursor_mode = g_hk_state.main.cursor_mode;
    config->pointing.main_drag_scroll = g_hk_state.main.drag_scroll;
    config->pointing.main_scroll_lock = g_hk_state.main.scroll_lock;
    config->pointing.main_default_multiplier = (int16_t)(g_hk_state.main.pointer_default_multiplier * 100);
    config->pointing.main_sniping_multiplier = (int16_t)(g_hk_state.main.pointer_sniping_multiplier * 100);
    config->pointing.main_scroll_buffer_size = g_hk_state.main.pointer_scroll_buffer_size;

    config->pointing.peripheral_cursor_mode = g_hk_state.peripheral.cursor_mode;
    config->pointing.peripheral_drag_scroll = g_hk_state.peripheral.drag_scroll;
    config->pointing.peripheral_scroll_lock = g_hk_state.peripheral.scroll_lock;
    config->pointing.peripheral_default_multiplier = (int16_t)(g_hk_state.peripheral.pointer_default_multiplier * 100);
    config->pointing.peripheral_sniping_multiplier = (int16_t)(g_hk_state.peripheral.pointer_sniping_multiplier * 100);
    config->pointing.peripheral_scroll_buffer_size = g_hk_state.peripheral.pointer_scroll_buffer_size;
}

static void write_eeconfig(void) {
    serialize_state_to_eeconfig(&hk_eeprom_config);
    eeconfig_update_user_datablock(&hk_eeprom_config, 0, sizeof(hk_eeprom_config_t));

    printf("write_eeconfig: eeprom data written\n");
}

static void hk_configure_tps43_common(hk_pointer_state_t* state) {
    state->pointer_default_multiplier = 1.25;
    state->pointer_sniping_multiplier = 1.0;
    state->pointer_scroll_buffer_size = 5;
}

static void hk_configure_pimoroni_common(hk_pointer_state_t* state) {
    state->pointer_default_multiplier = 1.5;
    state->pointer_sniping_multiplier = 1.0;
    state->pointer_scroll_buffer_size = 1;
}

static void hk_configure_trackpoint_common(hk_pointer_state_t* state) {
    state->pointer_default_multiplier = 2.0;
    state->pointer_sniping_multiplier = 1.0;
    state->pointer_scroll_buffer_size = 5;
}

static void hk_configure_cirque_common(hk_pointer_state_t* state) {
    state->pointer_default_multiplier = 1.0;
    state->pointer_sniping_multiplier = 1.0;
}

static hk_state_t init_state(void) {
    printf("init_state\n");
    hk_state_t state = {
        .init = true,
        .dirty = false,
        .is_main_side = is_keyboard_master(),
        .setting_default_scale = false,
        .setting_sniping_scale = false,
        .setting_scroll_buffer = false,
        .main = {
            .pointer_kind = POINTER_KIND_NONE,
            .cursor_mode = CURSOR_MODE_DEFAULT,
            .drag_scroll = false,
            .scroll_lock = SCROLL_LOCK_OFF,
            .pointer_default_multiplier = 0,
            .pointer_sniping_multiplier = 0,
            .pointer_scroll_buffer_size = 0,
        },
        .peripheral = {
            .pointer_kind = POINTER_KIND_NONE,
            .cursor_mode = CURSOR_MODE_DEFAULT,
            .drag_scroll = false,
            .scroll_lock = SCROLL_LOCK_OFF,
            .pointer_default_multiplier = 0,
            .pointer_sniping_multiplier = 0,
            .pointer_scroll_buffer_size = 0,
        },
        .display = {
            .last_kc = KC_NO,
            .last_pos = {0, 0},
            .last_mouse = {0, 0, 0, 0, 0},
            .pressing_keys = { BL, BL, BL, BL, BL, BL, 0 },
        },
    };

    if (!state.is_main_side) {
        return state;
    }

    #ifdef HK_POINTING_DEVICE_RIGHT_PIMORONI
        state.main.pointer_kind = POINTER_KIND_PIMORONI_TRACKBALL;
    #elif defined(HK_POINTING_DEVICE_RIGHT_TRACKPOINT)
        state.main.pointer_kind = POINTER_KIND_TRACKPOINT;
    #elif defined(HK_POINTING_DEVICE_RIGHT_CIRQUE35)
        state.main.pointer_kind = POINTER_KIND_CIRQUE35;
    #elif defined(HK_POINTING_DEVICE_RIGHT_CIRQUE40)
        state.main.pointer_kind = POINTER_KIND_CIRQUE40;
    #elif defined(HK_POINTING_DEVICE_RIGHT_TPS43)
        state.main.pointer_kind = POINTER_KIND_TPS43;
    #endif

    #ifdef HK_POINTING_DEVICE_LEFT_PIMORONI
        state.peripheral.pointer_kind = POINTER_KIND_PIMORONI_TRACKBALL;
    #elif defined(HK_POINTING_DEVICE_LEFT_TRACKPOINT)
        state.peripheral.pointer_kind = POINTER_KIND_TRACKPOINT;
    #elif defined(HK_POINTING_DEVICE_LEFT_CIRQUE35)
        state.peripheral.pointer_kind = POINTER_KIND_CIRQUE35;
    #elif defined(HK_POINTING_DEVICE_LEFT_CIRQUE40)
        state.peripheral.pointer_kind = POINTER_KIND_CIRQUE40;
    #elif defined(HK_POINTING_DEVICE_LEFT_TPS43)
        state.peripheral.pointer_kind = POINTER_KIND_TPS43;
    #endif

    if (is_keyboard_left()) {
        hk_pointer_kind temp = state.main.pointer_kind;
        state.main.pointer_kind = state.peripheral.pointer_kind;
        state.peripheral.pointer_kind = temp;
    }

    switch (state.main.pointer_kind) {
        case POINTER_KIND_TRACKPOINT:
            hk_configure_trackpoint_common(&state.main);
            break;
        case POINTER_KIND_CIRQUE35:
        case POINTER_KIND_CIRQUE40:
            hk_configure_cirque_common(&state.main);
            break;
        case POINTER_KIND_TPS43:
            hk_configure_tps43_common(&state.main);
            break;
        case POINTER_KIND_PIMORONI_TRACKBALL:
            hk_configure_pimoroni_common(&state.main);
            break;
        default:
            printf("init_state: unknown main pointer kind\n");
            break;
    }

    // Overrides the defaults for the case where the desired value is already known by the user. This only gets set
    // if there's nothing saved in eeprom.
    if (state.main.pointer_kind) {
        #ifdef HK_MAIN_DEFAULT_POINTER_DEFAULT_MULTIPLIER
            state.main.pointer_default_multiplier = HK_MAIN_DEFAULT_POINTER_DEFAULT_MULTIPLIER;
        #endif
        #ifdef HK_MAIN_DEFAULT_POINTER_SNIPING_MULTIPLIER
            state.main.pointer_sniping_multiplier = HK_MAIN_DEFAULT_POINTER_SNIPING_MULTIPLIER;
        #endif
        #ifdef HK_MAIN_DEFAULT_POINTER_SCROLL_BUFFER_SIZE
            state.main.pointer_scroll_buffer_size = HK_MAIN_DEFAULT_POINTER_SCROLL_BUFFER_SIZE;
        #endif
    }

    if (state.peripheral.pointer_kind != POINTER_KIND_NONE) {
        switch (state.peripheral.pointer_kind) {
            case POINTER_KIND_TRACKPOINT:
                hk_configure_trackpoint_common(&state.peripheral);
                break;
            case POINTER_KIND_CIRQUE35:
            case POINTER_KIND_CIRQUE40:
                hk_configure_cirque_common(&state.peripheral);
                break;
            case POINTER_KIND_TPS43:
                hk_configure_tps43_common(&state.peripheral);
                break;
            case POINTER_KIND_PIMORONI_TRACKBALL:
                hk_configure_pimoroni_common(&state.peripheral);
                state.peripheral.drag_scroll = true;
                break;
            default:
                printf("init_state: unknown peripheral pointer kind\n");
                break;
        }

        // Overrides the defaults for the case where the desired value is already known by the user. This only gets set
        // if there's nothing saved in eeprom.
        #ifdef HK_PERIPHERAL_DEFAULT_POINTER_DEFAULT_MULTIPLIER
            state.peripheral.pointer_default_multiplier = HK_PERIPHERAL_DEFAULT_POINTER_DEFAULT_MULTIPLIER;
        #endif
        #ifdef HK_PERIPHERAL_DEFAULT_POINTER_SNIPING_MULTIPLIER
            state.peripheral.pointer_sniping_multiplier = HK_PERIPHERAL_DEFAULT_POINTER_SNIPING_MULTIPLIER;
        #endif
        #ifdef HK_PERIPHERAL_DEFAULT_POINTER_SCROLL_BUFFER_SIZE
            state.peripheral.pointer_scroll_buffer_size = HK_PERIPHERAL_DEFAULT_POINTER_SCROLL_BUFFER_SIZE;
        #endif
    }

    return state;
}

static bool has_shift_mod(void) {
#        ifdef NO_ACTION_ONESHOT
    return mod_config(get_mods()) & MOD_MASK_SHIFT;
#        else
    return mod_config(get_mods() | get_oneshot_mods()) & MOD_MASK_SHIFT;
#        endif // NO_ACTION_ONESHOT
}

__attribute__((weak)) report_mouse_t pointing_device_task_keymap(report_mouse_t mouse_report) {
    return mouse_report;
}

__attribute__((weak)) report_mouse_t pointing_device_task_combined_keymap(report_mouse_t mouse_report) {
    return mouse_report;
}

__attribute__((weak)) bool process_record_keymap(uint16_t keycode, keyrecord_t* record) {
    return true;
}

// Perform scroll related functionality: drag scrolling, scroll lock.
void hk_process_scroll(const hk_pointer_state_t* pointer_state, report_mouse_t* mouse_report) {
    if (pointer_state->drag_scroll) {
        mouse_report->h = mouse_report->x;
        mouse_report->v = mouse_report->y;
        mouse_report->x = 0;
        mouse_report->y = 0;
    }

    if (pointer_state->pointer_scroll_buffer_size > 0) {
        static int16_t scroll_buffer_h = 0;
        static int16_t scroll_buffer_v = 0;

        scroll_buffer_h += mouse_report->h;
        scroll_buffer_v += mouse_report->v;
        mouse_report->h = 0;
        mouse_report->v = 0;

        bool output_horizontal = pointer_state->scroll_lock == SCROLL_LOCK_HORIZONTAL || pointer_state->scroll_lock == SCROLL_LOCK_OFF;
        bool output_vertical = pointer_state->scroll_lock == SCROLL_LOCK_VERTICAL || pointer_state->scroll_lock == SCROLL_LOCK_OFF;

        if (output_horizontal && abs(scroll_buffer_h) > pointer_state->pointer_scroll_buffer_size) {
            mouse_report->h = scroll_buffer_h > 0 ? 1 : -1;
            scroll_buffer_h = 0;
        }

        if (output_vertical && abs(scroll_buffer_v) > pointer_state->pointer_scroll_buffer_size) {
            mouse_report->v = scroll_buffer_v > 0 ? 1 : -1;
            scroll_buffer_v = 0;
        }
    }
}

static hk_cursor_mode hk_get_cursor_mode(bool side_peripheral) {
    return side_peripheral ? g_hk_state.peripheral.cursor_mode : g_hk_state.main.cursor_mode;
}

static hk_cursor_mode hk_get_dragscroll(bool side_peripheral) {
    return side_peripheral ? g_hk_state.peripheral.drag_scroll : g_hk_state.main.drag_scroll;
}

static void hk_set_cursor_mode(hk_cursor_mode target_mode, bool enabled, bool side_peripheral) {
    hk_pointer_state_t* state = side_peripheral ? &g_hk_state.peripheral : &g_hk_state.main;
    if (enabled) {
        state->cursor_mode = target_mode;
    } else {
        state->cursor_mode = CURSOR_MODE_DEFAULT;
    }

    g_hk_state.dirty = true;
}

static void hk_set_dragscroll(bool enabled, bool side_peripheral) {
    hk_pointer_state_t* state = side_peripheral ? &g_hk_state.peripheral : &g_hk_state.main;
    state->drag_scroll = enabled;
    g_hk_state.dirty = true;
}

static float scale_movement(const hk_pointer_state_t* state, int32_t amount) {
    float multiplier = 1;
    switch (state->cursor_mode) {
        case CURSOR_MODE_DEFAULT:
            multiplier = state->pointer_default_multiplier;
            break;
        case CURSOR_MODE_SNIPING:
            multiplier = state->pointer_sniping_multiplier;
            break;
    }

    return amount * multiplier;
}

static float hk_pointer_scale_step(const hk_pointer_state_t* state) {
    switch (state->pointer_kind) {
        case POINTER_KIND_PIMORONI_TRACKBALL:
            return .1;
        case POINTER_KIND_TRACKPOINT:
            return .1;
        case POINTER_KIND_CIRQUE35:
            return .1;
        case POINTER_KIND_CIRQUE40:
            return .1;
        case POINTER_KIND_TPS43:
            return .1;
        default:
            // Should never happen
            return 0;
    }
}

static void hk_cycle_pointer_default_multiplier(bool forward, bool side_peripheral) {
    hk_pointer_state_t* state = side_peripheral ? &g_hk_state.peripheral : &g_hk_state.main;
    float step = hk_pointer_scale_step(state);
    float new_value = forward ? state->pointer_default_multiplier + step : state->pointer_default_multiplier - step;
    if (new_value > 0) {
        state->pointer_default_multiplier = new_value;
        g_hk_state.dirty = true;
    }
}

static void hk_cycle_pointer_sniping_multiplier(bool forward, bool side_peripheral) {
    hk_pointer_state_t* state = side_peripheral ? &g_hk_state.peripheral : &g_hk_state.main;
    float step = hk_pointer_scale_step(state);
    float new_value = forward ? state->pointer_sniping_multiplier + step : state->pointer_sniping_multiplier - step;
    if (new_value > 0) {
        state->pointer_sniping_multiplier = new_value;
        g_hk_state.dirty = true;
    }
}

static void hk_cycle_pointer_scroll_buffer(bool forward, bool side_peripheral) {
    hk_pointer_state_t* state = side_peripheral ? &g_hk_state.peripheral : &g_hk_state.main;
    uint8_t new_value = forward ? state->pointer_scroll_buffer_size + 1 : state->pointer_scroll_buffer_size - 1;
    if (new_value >= 0) {
        state->pointer_scroll_buffer_size = new_value;
        g_hk_state.dirty = true;
    }
}

static void hk_cycle_scroll_mode(bool side_peripheral) {
    hk_pointer_state_t* state = side_peripheral ? &g_hk_state.peripheral : &g_hk_state.main;
    hk_scroll_lock new_mode = state->scroll_lock + 1;
    if (new_mode > SCROLL_LOCK_VERTICAL) {
        new_mode = SCROLL_LOCK_OFF;
    }
    state->scroll_lock = new_mode;
    g_hk_state.dirty = true;
}

void hk_process_mouse_report(const hk_pointer_state_t* pointer_state, report_mouse_t* mouse_report) {
    #ifdef ENABLE_DRIFT_DETECTION
        #ifndef POINTING_DEVICE_CONFIGURATION_TRACKPOINT
            #error "cannot use ENABLE_DRIFT_DETECTION without a trackpoint"
        #endif

        drift_detection(&mouse_report);
    #endif

    #ifdef ENABLE_PIMORONI_ADAPTIVE_MOTION
        #ifndef POINTING_DEVICE_CONFIGURATION_TRACKBALL
            #error "cannot use ENABLE_PIMORONI_ADAPTIVE_MOTION without a pimoroni trackball"
        #endif

        pimoroni_adaptive_motion(&mouse_report);
    #endif

    // rounding carry to recycle dropped floats from int mouse reports, to smoothen low speed movements (credit
    // @ankostis)
    static float rounding_carry_x = 0;
    static float rounding_carry_y = 0;

    // Reset carry when pointer swaps direction, to follow user's hand.
    if (mouse_report->x * rounding_carry_x < 0) rounding_carry_x = 0;
    if (mouse_report->y * rounding_carry_y < 0) rounding_carry_y = 0;

    // First, scale the mouse movement.
    const report_mouse_t mouse_report_copy = *mouse_report;
    const float new_x = scale_movement(pointer_state, mouse_report->x);
    const float new_y = scale_movement(pointer_state, mouse_report->y);

    // Accumulate any difference from next integer (quantization).
    rounding_carry_x = new_x - (int)new_x;
    rounding_carry_y = new_y - (int)new_y;

    // Clamp values.
    const mouse_xy_report_t x = CONSTRAIN_XY(new_x);
    const mouse_xy_report_t y = CONSTRAIN_XY(new_y);
    bool debug_mouse_report = false;
    if (x != 0 || y != 0 || mouse_report->v != 0 || mouse_report->h != 0) {
        debug_mouse_report = true;
    }
    mouse_report->x = x;
    mouse_report->y = y;

    hk_process_scroll(pointer_state, mouse_report);

    if (debug_mouse_report) {
        debug_hk_mouse_report("before", &mouse_report_copy);
        debug_hk_mouse_report(" after", mouse_report);
    }
    g_hk_state.dirty = true;
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    if (!g_hk_state.is_main_side) {
        return pointing_device_task_keymap(mouse_report);
    }

    // We get here only on the main side.
    hk_process_mouse_report(&g_hk_state.main, &mouse_report);
    g_hk_state.display.last_mouse = mouse_report;

    return pointing_device_task_keymap(mouse_report);
}

#if defined(SPLIT_POINTING_ENABLE) && defined(POINTING_DEVICE_COMBINED)
report_mouse_t pointing_device_task_combined_user(report_mouse_t left_report, report_mouse_t right_report) {
    if (!g_hk_state.is_main_side) {
        report_mouse_t report = pointing_device_combine_reports(left_report, right_report);
        return pointing_device_task_combined_keymap(report);
    }

    // We get here only on the main side. Use is_keyboard_left to know which report is main and which is peripheral.
    hk_process_mouse_report(&g_hk_state.main, is_keyboard_left() ? &left_report : &right_report);
    hk_process_mouse_report(&g_hk_state.peripheral, is_keyboard_left() ? &right_report : &left_report);

    report_mouse_t report = pointing_device_combine_reports(left_report, right_report);
    g_hk_state.display.last_mouse = report;
    return pointing_device_task_combined_keymap(report);
}
#endif

// clang-format off
const char PROGMEM code_to_name[] = {
    'a', 'b', 'c', 'd', 'e', 'f',  'g', 'h', 'i',  'j',
    'k', 'l', 'm', 'n', 'o', 'p',  'q', 'r', 's',  't',
    'u', 'v', 'w', 'x', 'y', 'z',  '1', '2', '3',  '4',
    '5', '6', '7', '8', '9', '0',  'R', 'E', 'B',  'T',
    '_', '-', '=', '[', ']', '\\', '#', ';', '\'', '`',
    ',', '.', '/',
};
// clang-format on

static void pressing_keys_update(uint16_t keycode, keyrecord_t *record) {
    // Process only valid keycodes.
    if (keycode >= 4 && keycode < 57) {
        char value = pgm_read_byte(code_to_name + keycode - 4);
        char where = BL;
        if (!record->event.pressed) {
            // Swap `value` and `where` when releasing.
            where = value;
            value = BL;
        }
        // Rewrite the last `where` of pressing_keys to `value` .
        for (int i = 0; i < HK_OLED_MAX_PRESSING_KEYCODES; i++) {
            if (g_hk_state.display.pressing_keys[i] == where) {
                g_hk_state.display.pressing_keys[i] = value;
                break;
            }
        }
    }
    g_hk_state.dirty = true;
}

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
    if (!g_hk_state.is_main_side) {
        return process_record_keymap(keycode, record);
    }

    g_hk_state.dirty = true;
    g_hk_state.display.last_kc = keycode;
    g_hk_state.display.last_pos = record->event.key;
    pressing_keys_update(keycode, record);

    if (!process_record_keymap(keycode, record)) {
        return false;
    }

    bool propagate_event = true;
    bool state_changed = false;

    switch (keycode) {
        case HK_SAVE_SETTINGS:
            if (record->event.pressed) {
                write_eeconfig();
            }
            break;
        case HK_RESET_SETTINGS:
            if (record->event.pressed) {
                g_hk_state = init_state();
                write_eeconfig();
            }
            break;
        case HK_DUMP_SETTINGS:
            if (record->event.pressed) {
                debug_hk_state_to_console(&g_hk_state);
            }
            break;
        case KC_UP:
        case KC_DOWN:
            if (!g_hk_state.setting_default_scale && !g_hk_state.setting_sniping_scale && !g_hk_state.setting_scroll_buffer) {
                break;
            }
            if (record->event.pressed) {
                if (g_hk_state.setting_default_scale) {
                    hk_cycle_pointer_default_multiplier(/*forward=*/keycode == KC_UP, /*side_peripheral=*/has_shift_mod());
                }
                else if (g_hk_state.setting_sniping_scale) {
                    hk_cycle_pointer_sniping_multiplier(/*forward=*/keycode == KC_UP, /*side_peripheral=*/has_shift_mod());
                }
                else if (g_hk_state.setting_scroll_buffer) {
                    hk_cycle_pointer_scroll_buffer(/*forward=*/keycode == KC_UP, /*side_peripheral=*/has_shift_mod());
                }
                propagate_event = false;
                state_changed = true;
            }
            break;
        case HK_POINTER_SET_DEFAULT_SCALER:
            g_hk_state.setting_default_scale = record->event.pressed;
            // state_changed = true;
            break;
        case HK_POINTER_SET_SNIPING_SCALER:
            g_hk_state.setting_sniping_scale = record->event.pressed;
            // state_changed = true;
            break;
        case HK_POINTER_SET_SCROLL_BUFFER:
            g_hk_state.setting_scroll_buffer = record->event.pressed;
            // state_changed = true;
            break;
        case HK_SNIPING_MODE:
            hk_set_cursor_mode(/*mode=*/CURSOR_MODE_SNIPING, /*enabled=*/record->event.pressed, /*side_peripheral=*/has_shift_mod());
            state_changed = true;
            break;
        case HK_SNIPING_MODE_TOGGLE:
            if (record->event.pressed) {
                bool is_on = hk_get_cursor_mode(/*side_peripheral=*/has_shift_mod()) == CURSOR_MODE_SNIPING;
                hk_set_cursor_mode(/*mode=*/CURSOR_MODE_SNIPING, /*enabled=*/!is_on, /*side_peripheral=*/has_shift_mod());
                state_changed = true;
            }
            break;
        case HK_DRAGSCROLL_MODE:
            hk_set_dragscroll(/*enabled=*/record->event.pressed, /*side_peripheral=*/has_shift_mod());
            state_changed = true;
            break;
        case HK_DRAGSCROLL_MODE_TOGGLE:
            if (record->event.pressed) {
                bool is_on = hk_get_dragscroll(/*side_peripheral=*/has_shift_mod());
                hk_set_dragscroll(/*enabled=*/!is_on, /*side_peripheral=*/has_shift_mod());
                state_changed = true;
            }
            break;
        case HK_CYCLE_SCROLL_LOCK:
            if (record->event.pressed) {
                hk_cycle_scroll_mode(/*side_peripheral=*/has_shift_mod());
                state_changed = true;
            }
            break;
    }
    if (state_changed) {
        debug_hk_state_to_console(&g_hk_state);
        // write_eeconfig();
    }
    return propagate_event;
}

__attribute__((weak)) void keyboard_post_init_keymap(void) {}

void housekeeping_task_user(void) {
#ifdef HK_SPLIT_SYNC_STATE
    if (is_keyboard_master()) {
        static uint32_t last_sync = 0;
        if (timer_elapsed32(last_sync) > 100 && g_hk_state.dirty) {
            if (transaction_rpc_send(HK_SYNC_STATE, sizeof(g_hk_state), &g_hk_state)) {
                g_hk_state.dirty = false;
                last_sync = timer_read32();
            } else {
                printf("housekeeping_task_user: failed to send HK_SYNC_STATE rpc\n");
            }
        }
    }
#endif


#if defined(HK_PIMORONI_TRACKBALL_RGB_RAINBOW) && defined(POINTING_DEVICE_DRIVER_pimoroni_trackball)
    bool run_animation = false;

    // With two trackballs, always run the animation.
    #if defined(HK_POINTING_DEVICE_LEFT_PIMORONI) && defined(HK_POINTING_DEVICE_RIGHT_PIMORONI)
        run_animation = true;
    #elif defined(HK_POINTING_DEVICE_LEFT_PIMORONI)
        run_animation = is_keyboard_left();
    #elif defined(HK_POINTING_DEVICE_RIGHT_PIMORONI)
        run_animation = !is_keyboard_left();
    #else
        #error "HK_PIMORONI_TRACKBALL_RGB_RAINBOW requires a pimoroni on either sides."
    #endif

    if (run_animation) {
        static uint32_t timer = 0;
        static HSV color = { .h = 0, .s = 255, .v = 255 };

        if (timer_elapsed32(timer) < 400)
            return;

        timer = timer_read32();

        // increase hue -> change color
        color.h++;

        RGB rgb = hsv_to_rgb(color);
        pimoroni_trackball_set_rgbw(rgb.r, rgb.g, rgb.b, 0);
    }
#endif
}

void keyboard_post_init_user(void) {
    if (!is_keyboard_master()) {
        #ifdef HK_SPLIT_SYNC_STATE
            transaction_register_rpc(HK_SYNC_STATE, hk_rpc_sync_state);
        #endif

        keyboard_post_init_keymap();
        return;
    }

    memset(&hk_eeprom_config, 0, sizeof(hk_eeprom_config_t));
    eeconfig_read_user_datablock(&hk_eeprom_config, 0, sizeof(hk_eeprom_config_t));
    printf("keyboard_post_init_user: reading eeprom, check: %u\n", hk_eeprom_config.check);
    if (!eeconfig_is_user_datablock_valid() || !hk_eeprom_config.check) {
        printf("keyboard_post_init_user: eeprom data not found, initializing\n");
        eeconfig_init_user();
    } else {
        g_hk_state = init_state();
        deserialize_eeconfig_to_state(&hk_eeprom_config);
        debug_hk_state_to_console(&g_hk_state);
    }

    keyboard_post_init_keymap();
}

__attribute__((weak)) void eeconfig_init_keymap(void) {}
void                       eeconfig_init_user(void) {
    g_hk_state = init_state();
    debug_hk_state_to_console(&g_hk_state);

    memset(&hk_eeprom_config, 0, sizeof(hk_eeprom_config_t));
    hk_eeprom_config.check = true;
    serialize_state_to_eeconfig(&hk_eeprom_config);

    eeconfig_init_keymap();
    eeconfig_update_user_datablock(&hk_eeprom_config, 0, sizeof(hk_eeprom_config_t));

    printf("eeconfig_init_user: eeprom data written\n");
}

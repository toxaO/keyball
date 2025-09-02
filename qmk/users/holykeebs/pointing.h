#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "quantum.h"

typedef enum {
    POINTER_KIND_NONE,
    POINTER_KIND_PIMORONI_TRACKBALL,
    POINTER_KIND_TRACKPOINT,
    POINTER_KIND_CIRQUE35,
    POINTER_KIND_CIRQUE40,
    POINTER_KIND_TPS43,
} hk_pointer_kind;

typedef enum {
    SCROLL_LOCK_OFF,
    SCROLL_LOCK_HORIZONTAL,
    SCROLL_LOCK_VERTICAL,
} hk_scroll_lock;

typedef enum {
    CURSOR_MODE_DEFAULT,
    CURSOR_MODE_SNIPING,
} hk_cursor_mode;

typedef struct PACKED {
    hk_pointer_kind pointer_kind : 4;
    hk_cursor_mode cursor_mode : 2;
    hk_scroll_lock scroll_lock : 2;
    bool drag_scroll : 1;
    float pointer_default_multiplier;
    float pointer_sniping_multiplier;
    uint8_t pointer_scroll_buffer_size;
} hk_pointer_state_t;

#define HK_OLED_MAX_PRESSING_KEYCODES 6

typedef struct PACKED {
    uint16_t       last_kc;
    keypos_t       last_pos;
    report_mouse_t last_mouse;

    // Buffer to indicate pressing keys.
    char pressing_keys[HK_OLED_MAX_PRESSING_KEYCODES + 1];
} hk_display_state_t;

typedef struct PACKED {
    bool init : 1;
    bool dirty : 1;

    bool is_main_side : 1;
    bool setting_default_scale : 1;
    bool setting_sniping_scale : 1;
    bool setting_scroll_buffer : 1;

    hk_pointer_state_t main;
    hk_pointer_state_t peripheral;

    hk_display_state_t display;
} hk_state_t;

#ifdef HK_SPLIT_SYNC_STATE
    _Static_assert(sizeof(hk_state_t) <= RPC_M2S_BUFFER_SIZE, "State too big to fit in split sync buffer size.");
#endif

extern hk_state_t g_hk_state;

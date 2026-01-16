#include "keyball_led.h"

#ifdef RGBLIGHT_ENABLE

#    include "quantum.h"
#    include "rgblight.h"
#    include "color.h"

#    ifdef SPLIT_KEYBOARD
#        include "transactions.h"
#        include "split_util.h"
#    endif

#    ifndef HSV_BLUE
#        define HSV_BLUE 170, 255, 255
#    endif
#    ifndef HSV_OFF
#        define HSV_OFF 0, 0, 0
#    endif

#    ifndef KEYBALL_LED_MONITOR_COLOR_HSV
#        define KEYBALL_LED_MONITOR_COLOR_HSV HSV_BLUE
#    endif

#    define LED_MONITOR_FLAG_ACTIVE (1u << 0)

#    ifdef SPLIT_KEYBOARD
enum {
    KEYBALL_LED_SYNC_CMD_SET_AT = 0,
    KEYBALL_LED_SYNC_CMD_SET_RANGE,
};
#    endif

static uint8_t monitor_index = 0;
static int16_t last_index     = -1;

static struct {
    bool    active;
    bool    restore_pending;
    bool    prev_enabled;
    uint8_t prev_mode;
    uint8_t prev_hue;
    uint8_t prev_sat;
    uint8_t prev_val;
} monitor_state = {0};

static uint8_t keyball_led_normalize_index(uint8_t index);
static uint8_t keyball_led_clamp_count(uint8_t start, uint8_t count);
static void keyball_led_apply_hsv_at(uint8_t hue, uint8_t sat, uint8_t val, uint8_t index);
static void keyball_led_apply_hsv_range(uint8_t hue, uint8_t sat, uint8_t val, uint8_t index, uint8_t count);

#    ifdef SPLIT_KEYBOARD
typedef struct {
    uint8_t index;
    uint8_t flags;
} keyball_led_monitor_packet_t;

typedef struct {
    uint8_t cmd;
    uint8_t index;
    uint8_t count;
    uint8_t hue;
    uint8_t sat;
    uint8_t val;
} keyball_led_sync_packet_t;
#    endif

static inline bool keyball_led_monitor_has_leds(void) {
    return RGBLIGHT_LED_COUNT > 0;
}

#    ifdef SPLIT_KEYBOARD
static inline void keyball_led_monitor_split_counts(uint8_t *left, uint8_t *right) {
#        ifdef RGBLED_SPLIT
    static const uint8_t split_counts[] = RGBLED_SPLIT;
    *left  = split_counts[0];
    *right = (sizeof(split_counts) / sizeof(split_counts[0]) > 1) ? split_counts[1] : 0;
#        else
    *left  = RGBLIGHT_LED_COUNT / 2;
    *right = RGBLIGHT_LED_COUNT - *left;
#        endif
}
#    endif

static inline void keyball_led_monitor_get_local_range(uint8_t *start, uint8_t *end) {
    uint8_t clip_start = rgblight_ranges.clipping_start_pos;
    uint8_t clip_count = rgblight_ranges.clipping_num_leds;
    if (clip_count > 0 && clip_start < RGBLIGHT_LED_COUNT) {
        uint8_t clip_end = clip_start + clip_count;
        if (clip_end > RGBLIGHT_LED_COUNT) {
            clip_end = RGBLIGHT_LED_COUNT;
        }
        if (clip_count < RGBLIGHT_LED_COUNT) {
            *start = clip_start;
            *end   = clip_end;
            return;
        }
    }

#    ifdef SPLIT_KEYBOARD
    uint8_t left_leds = 0, right_leds = 0;
    keyball_led_monitor_split_counts(&left_leds, &right_leds);
    if (is_keyboard_left()) {
        *start = 0;
        *end   = left_leds;
    } else {
        *start = left_leds;
        *end   = left_leds + right_leds;
    }
#    else
    *start = 0;
    *end   = RGBLIGHT_LED_COUNT;
#    endif
}

static inline bool keyball_led_monitor_index_is_local(uint8_t index) {
    uint8_t start, end;
    keyball_led_monitor_get_local_range(&start, &end);
    return index >= start && index < end;
}

static void keyball_led_monitor_clear(void) {
    if (!keyball_led_monitor_has_leds()) {
        return;
    }
    uint8_t start, end;
    keyball_led_monitor_get_local_range(&start, &end);
    if (end > start) {
        rgblight_setrgb_range(0, 0, 0, start, end);
    }
    last_index = -1;
}

static void keyball_led_monitor_apply(void) {
    if (!monitor_state.active || !keyball_led_monitor_has_leds()) {
        return;
    }

    if (last_index >= 0 && last_index < RGBLIGHT_LED_COUNT && last_index != monitor_index) {
        if (keyball_led_monitor_index_is_local((uint8_t)last_index)) {
            rgblight_sethsv_at(HSV_OFF, (uint8_t)last_index);
        }
        last_index = -1;
    }

    if (!keyball_led_monitor_index_is_local(monitor_index)) {
        last_index = -1;
        return;
    }

    rgblight_sethsv_at(KEYBALL_LED_MONITOR_COLOR_HSV, monitor_index);
    last_index = monitor_index;
}

#    ifdef SPLIT_KEYBOARD
static void keyball_led_monitor_sync(void) {
    if (!is_keyboard_master()) {
        return;
    }
    keyball_led_monitor_packet_t packet = {
        .index = monitor_index,
        .flags = monitor_state.active ? LED_MONITOR_FLAG_ACTIVE : 0,
    };
    transaction_rpc_send(KEYBALL_LED_MONITOR_SYNC, sizeof(packet), &packet);
}
#    endif

static void keyball_led_monitor_set_active(bool active, bool from_remote) {
    if (monitor_state.active == active) {
#    ifdef SPLIT_KEYBOARD
        if (!from_remote) {
            keyball_led_monitor_sync();
        }
#    endif
        return;
    }

    if (active) {
        monitor_state.prev_enabled = rgblight_is_enabled();
        monitor_state.prev_mode    = rgblight_get_mode();
        monitor_state.prev_hue     = rgblight_get_hue();
        monitor_state.prev_sat     = rgblight_get_sat();
        monitor_state.prev_val     = rgblight_get_val();
        monitor_state.restore_pending = true;

        if (!monitor_state.prev_enabled) {
            rgblight_enable_noeeprom();
        }
        keyball_led_monitor_clear();
        monitor_state.active = true;
        keyball_led_monitor_apply();
    } else {
        if (monitor_state.restore_pending) {
            if (!monitor_state.prev_enabled) {
                rgblight_disable_noeeprom();
            } else {
                rgblight_mode_noeeprom(monitor_state.prev_mode);
                rgblight_sethsv_noeeprom(monitor_state.prev_hue, monitor_state.prev_sat, monitor_state.prev_val);
            }
        }
        monitor_state.restore_pending = false;
        monitor_state.active          = false;
        last_index                    = -1;
    }

#    ifdef SPLIT_KEYBOARD
    if (!from_remote) {
        keyball_led_monitor_sync();
    }
#    else
    (void)from_remote;
#    endif
}

#    ifdef SPLIT_KEYBOARD
static void keyball_led_monitor_sync_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    (void)out_buflen;
    (void)out_data;
    if (in_buflen < sizeof(keyball_led_monitor_packet_t)) {
        return;
    }
    const keyball_led_monitor_packet_t *packet = (const keyball_led_monitor_packet_t *)in_data;
    if (!keyball_led_monitor_has_leds()) {
        return;
    }
    uint8_t count = RGBLIGHT_LED_COUNT ? RGBLIGHT_LED_COUNT : 1;
    monitor_index = packet->index % count;
    bool want_active = (packet->flags & LED_MONITOR_FLAG_ACTIVE) != 0;
    keyball_led_monitor_set_active(want_active, true);
    if (monitor_state.active) {
        keyball_led_monitor_apply();
    }
}

static void keyball_led_sync_send(uint8_t cmd, uint8_t index, uint8_t count, uint8_t hue, uint8_t sat, uint8_t val) {
    if (!is_keyboard_master()) {
        return;
    }
    keyball_led_sync_packet_t packet = {
        .cmd   = cmd,
        .index = index,
        .count = count,
        .hue   = hue,
        .sat   = sat,
        .val   = val,
    };
    transaction_rpc_send(KEYBALL_LED_SYNC, sizeof(packet), &packet);
}

static void keyball_led_sync_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    (void)out_buflen;
    (void)out_data;
    if (in_buflen < sizeof(keyball_led_sync_packet_t)) {
        return;
    }
    const keyball_led_sync_packet_t *packet = (const keyball_led_sync_packet_t *)in_data;
    switch (packet->cmd) {
        case KEYBALL_LED_SYNC_CMD_SET_AT:
            keyball_led_apply_hsv_at(packet->hue, packet->sat, packet->val, packet->index);
            break;
        case KEYBALL_LED_SYNC_CMD_SET_RANGE:
            keyball_led_apply_hsv_range(packet->hue, packet->sat, packet->val, packet->index, packet->count);
            break;
        default:
            break;
    }
}
#    endif

static uint8_t keyball_led_normalize_index(uint8_t index) {
    if (!RGBLIGHT_LED_COUNT) {
        return 0;
    }
    return index % RGBLIGHT_LED_COUNT;
}

static uint8_t keyball_led_clamp_count(uint8_t start, uint8_t count) {
    if (!RGBLIGHT_LED_COUNT || count == 0) {
        return 0;
    }
    uint8_t max = RGBLIGHT_LED_COUNT - start;
    if (count > max) {
        return max;
    }
    return count;
}

static void keyball_led_apply_hsv_at(uint8_t hue, uint8_t sat, uint8_t val, uint8_t index) {
    if (!keyball_led_monitor_has_leds()) {
        return;
    }
    uint8_t idx = keyball_led_normalize_index(index);
    rgblight_sethsv_at(hue, sat, val, idx);
}

static void keyball_led_apply_hsv_range(uint8_t hue, uint8_t sat, uint8_t val, uint8_t index, uint8_t count) {
    if (!keyball_led_monitor_has_leds()) {
        return;
    }
    uint8_t idx = keyball_led_normalize_index(index);
    uint8_t len = keyball_led_clamp_count(idx, count);
    if (len == 0) {
        return;
    }
    rgblight_sethsv_range(hue, sat, val, idx, len);
}

void keyball_led_monitor_init(void) {
#    ifdef SPLIT_KEYBOARD
    transaction_register_rpc(KEYBALL_LED_MONITOR_SYNC, keyball_led_monitor_sync_handler);
    transaction_register_rpc(KEYBALL_LED_SYNC, keyball_led_sync_handler);
#    endif
    monitor_state.active          = false;
    monitor_state.restore_pending = false;
    monitor_index                 = 0;
    last_index                    = -1;
}

void keyball_led_monitor_on(void) {
    if (!keyball_led_monitor_has_leds()) {
        return;
    }
    keyball_led_monitor_set_active(true, false);
}

void keyball_led_monitor_off(void) {
    keyball_led_monitor_set_active(false, false);
}

void keyball_led_monitor_step(int8_t delta) {
    if (!monitor_state.active || !keyball_led_monitor_has_leds()) {
        return;
    }

    int16_t total = RGBLIGHT_LED_COUNT;
    if (total <= 0) {
        return;
    }

    int16_t next = (int16_t)monitor_index + delta;
    while (next < 0) {
        next += total;
    }
    while (next >= total) {
        next -= total;
    }

    monitor_index = (uint8_t)next;
    keyball_led_monitor_apply();
#    ifdef SPLIT_KEYBOARD
    keyball_led_monitor_sync();
#    endif
}

uint8_t keyball_led_monitor_get_index(void) {
    return monitor_index;
}

void keyball_led_set_hsv_at(uint8_t hue, uint8_t sat, uint8_t val, uint8_t index) {
    keyball_led_apply_hsv_at(hue, sat, val, index);
#    ifdef SPLIT_KEYBOARD
    keyball_led_sync_send(KEYBALL_LED_SYNC_CMD_SET_AT, index, 1, hue, sat, val);
#    endif
}

void keyball_led_set_hsv_range(uint8_t hue, uint8_t sat, uint8_t val, uint8_t index, uint8_t count) {
    keyball_led_apply_hsv_range(hue, sat, val, index, count);
#    ifdef SPLIT_KEYBOARD
    keyball_led_sync_send(KEYBALL_LED_SYNC_CMD_SET_RANGE, index, count, hue, sat, val);
#    endif
}

#else  // RGBLIGHT_ENABLE

void keyball_led_monitor_init(void) {}
void keyball_led_monitor_on(void) {}
void keyball_led_monitor_off(void) {}
void keyball_led_monitor_step(int8_t delta) {
    (void)delta;
}
uint8_t keyball_led_monitor_get_index(void) {
    return 0;
}

void keyball_led_set_hsv_at(uint8_t hue, uint8_t sat, uint8_t val, uint8_t index) {
    (void)hue;
    (void)sat;
    (void)val;
    (void)index;
}

void keyball_led_set_hsv_range(uint8_t hue, uint8_t sat, uint8_t val, uint8_t index, uint8_t count) {
    (void)hue;
    (void)sat;
    (void)val;
    (void)index;
    (void)count;
}

#endif // RGBLIGHT_ENABLE

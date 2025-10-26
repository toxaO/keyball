#include "led_user.h"

#ifdef RGBLIGHT_ENABLE

#include "quantum.h"
#include "rgblight.h"
#include "color.h"
#include <stdbool.h>

#ifdef SPLIT_KEYBOARD
#    include "transactions.h"
#endif

#ifndef HSV_BLUE
#    define HSV_BLUE 170, 255, 255
#endif
#ifndef HSV_OFF
#    define HSV_OFF 0, 0, 0
#endif

#define LED_MONITOR_FLAG_ACTIVE (1u << 0)

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

#ifdef SPLIT_KEYBOARD
typedef struct {
    uint8_t index;
    uint8_t flags;
} led_monitor_packet_t;
#endif

static inline bool has_leds(void) {
    return RGBLIGHT_LED_COUNT > 0;
}

static void clear_all_leds(void) {
    if (!has_leds()) {
        return;
    }
    rgblight_setrgb_range(0, 0, 0, 0, RGBLIGHT_LED_COUNT);
    last_index = -1;
}

static void apply_index(void) {
    if (!monitor_state.active || !has_leds()) {
        return;
    }

    if (last_index >= 0 && last_index < RGBLIGHT_LED_COUNT && last_index != monitor_index) {
        rgblight_sethsv_at(HSV_OFF, (uint8_t)last_index);
    }

    rgblight_sethsv_at(HSV_BLUE, monitor_index);
    last_index = monitor_index;
}

#ifdef SPLIT_KEYBOARD
static void sync_state(void) {
    if (!is_keyboard_master()) {
        return;
    }
    led_monitor_packet_t packet = {
        .index = monitor_index,
        .flags = monitor_state.active ? LED_MONITOR_FLAG_ACTIVE : 0,
    };
    transaction_rpc_send(TOXAO_LED_MONITOR_SYNC, sizeof(packet), &packet);
}
#endif

static void set_active(bool active, bool from_remote) {
    if (monitor_state.active == active) {
#ifdef SPLIT_KEYBOARD
        if (!from_remote) {
            sync_state();
        }
#endif
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
        clear_all_leds();
        monitor_state.active = true;
        apply_index();
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

#ifdef SPLIT_KEYBOARD
    if (!from_remote) {
        sync_state();
    }
#else
    (void)from_remote;
#endif
}

#ifdef SPLIT_KEYBOARD
static void led_monitor_sync_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    (void)out_buflen;
    (void)out_data;
    if (in_buflen < sizeof(led_monitor_packet_t)) {
        return;
    }

    const led_monitor_packet_t *packet = (const led_monitor_packet_t *)in_data;
    if (!has_leds()) {
        return;
    }

    uint8_t count = RGBLIGHT_LED_COUNT ? RGBLIGHT_LED_COUNT : 1;
    monitor_index = packet->index % count;
    bool want_active = (packet->flags & LED_MONITOR_FLAG_ACTIVE) != 0;
    set_active(want_active, true);
    if (monitor_state.active) {
        apply_index();
    }
}
#endif

void keyball_led_monitor_init(void) {
#ifdef SPLIT_KEYBOARD
    transaction_register_rpc(TOXAO_LED_MONITOR_SYNC, led_monitor_sync_handler);
#endif
    monitor_state.active          = false;
    monitor_state.restore_pending = false;
    last_index                    = -1;
}

void keyball_led_monitor_on(void) {
    if (!has_leds()) {
        return;
    }
    set_active(true, false);
}

void keyball_led_monitor_off(void) {
    set_active(false, false);
}

void keyball_led_monitor_step(int8_t delta) {
    if (!monitor_state.active || !has_leds()) {
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
    apply_index();
#ifdef SPLIT_KEYBOARD
    sync_state();
#endif
}

uint8_t keyball_led_monitor_get_index(void) {
    return monitor_index;
}

#else

void keyball_led_monitor_init(void) {}
void keyball_led_monitor_on(void) {}
void keyball_led_monitor_off(void) {}
void keyball_led_monitor_step(int8_t delta) {
    (void)delta;
}
uint8_t keyball_led_monitor_get_index(void) { return 0; }

#endif // RGBLIGHT_ENABLE

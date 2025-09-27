#pragma once

#include <stdbool.h>
#include <stdint.h>

// 名称を Debug から Setting へ移行（互換のため DEBUG も同値で残す）
typedef enum {
    KB_OLED_MODE_NORMAL  = 0,
    KB_OLED_MODE_SETTING = 1,
    KB_OLED_MODE_DEBUG   = 1, // backward-compatible alias
} kb_oled_mode_t;

void        keyball_oled_mode_toggle(void);
void        keyball_oled_set_mode(kb_oled_mode_t m);
kb_oled_mode_t keyball_oled_get_mode(void);

// Setting mode helpers (旧 dbg_* をエイリアスとして残す)
void        keyball_oled_setting_toggle(void);
void        keyball_oled_setting_show(bool on);
bool        keyball_oled_setting_enabled(void);
// Backward-compatible aliases
#define     keyball_oled_dbg_toggle   keyball_oled_setting_toggle
#define     keyball_oled_dbg_show     keyball_oled_setting_show
#define     keyball_oled_dbg_enabled  keyball_oled_setting_enabled

void        keyball_oled_next_page(void);
void        keyball_oled_prev_page(void);
uint8_t     keyball_oled_get_page(void);
uint8_t     keyball_oled_get_page_count(void);

void        keyball_oled_render_setting(void);
// Backward-compatible alias
void        keyball_oled_render_debug(void);

// Simple info renderers
void keyball_oled_render_ballinfo(void);
void keyball_oled_render_keyinfo(void);
void keyball_oled_render_layerinfo(void);
void keyball_oled_render_ballsubinfo(void);

// New info render helpers (keyboard-level)
void oled_render_info_layer(void);
void oled_render_info_layer_default(void);
void oled_render_info_ball(void);
void oled_render_info_keycode(void);
void oled_render_info_mods(void);
void oled_render_info_mods_oneshot(void);
void oled_render_info_mods_lock(void);
void oled_render_info_cpi(void);
void oled_render_info_scroll_step(void);
void oled_render_info_swipe_tag(void);
void oled_render_info_key_pos(void);

// UI navigation for debug pages
bool keyball_oled_handle_ui_key(uint16_t keycode, keyrecord_t *record);

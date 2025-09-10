#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "report.h"

// === Swipe hook (KB-level detect -> user-level action) ===
typedef enum {
    KB_SWIPE_NONE = 0,
    KB_SWIPE_UP,
    KB_SWIPE_DOWN,
    KB_SWIPE_RIGHT,
    KB_SWIPE_LEFT,
} kb_swipe_dir_t;

// ユーザー定義のモードタグ（KBは中身を解釈しない）
typedef uint8_t kb_swipe_tag_t;

// user → KB：スワイプセッション開始/終了
void            keyball_swipe_begin(kb_swipe_tag_t mode_tag);
void            keyball_swipe_end(void);

// user が参照したい状態
bool            keyball_swipe_is_active(void);          // 押下中？
kb_swipe_tag_t  keyball_swipe_mode_tag(void);           // begin() で渡されたタグ
kb_swipe_dir_t  keyball_swipe_direction(void);          // 現在の方向（未実装の間は常に NONE）
bool            keyball_swipe_fired_since_begin(void);  // セッション開始以降に1回でも発火したか
bool            keyball_swipe_consume_fired(void);      // ↑を取得して false に戻す

// KB → user：発火イベント（弱シンボル；実装は user 側。未定義なら呼ばない）
__attribute__((weak)) void keyball_on_swipe_fire(kb_swipe_tag_t mode_tag, kb_swipe_dir_t dir);

// ==== Swipe runtime params ====
typedef struct {
    uint16_t step;     // 発火しきい値（counts）
    uint8_t  deadzone; // デッドゾーン（counts）
    uint8_t  reset_ms; // 停止後リセット遅延(ms)
    bool     freeze;   // スワイプ中ポインタ凍結
} kb_swipe_params_t;

// 取得・設定
kb_swipe_params_t keyball_swipe_get_params(void);
void keyball_swipe_set_step(uint16_t v);
void keyball_swipe_set_deadzone(uint8_t v);
void keyball_swipe_set_reset_ms(uint8_t v);
void keyball_swipe_set_freeze(bool on);
void keyball_swipe_toggle_freeze(void);

// ==== Swipe params persistence ====
bool keyball_swipe_cfg_load(void);   // 起動時に呼ぶ: true=読めた, false=初期化
void keyball_swipe_cfg_save(void);   // 現在の params を保存
void keyball_swipe_cfg_reset(void);  // 既定値に戻して保存（=工場出荷）

// Apply swipe motion to mouse report
void keyball_swipe_apply(report_mouse_t *report, report_mouse_t *output, bool is_left);

// Render swipe debug to OLED
void keyball_swipe_render_debug(void);


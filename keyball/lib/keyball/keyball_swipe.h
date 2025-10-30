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

// スワイプモードタグ（ユーザが独自に使っても良い）
typedef uint8_t kb_swipe_tag_t;
// 代表タグ（APP/VOL/BRO/TAB/WIN）: キーボードレベルのSWキーで使用
enum {
    KBS_TAG_APP = 1,
    KBS_TAG_VOL,
    KBS_TAG_BRO,
    KBS_TAG_TAB,
    KBS_TAG_WIN,
    KBS_TAG_ARR, // Arrow proxy (SW_ARR)
    KBS_TAG_EX1, // Extension swipe key #1 (SW_EX1)
    KBS_TAG_EX2, // Extension swipe key #2 (SW_EX2)
};

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
// セッション終了時のクリーンアップ用フック（任意実装）
__attribute__((weak)) void keyball_on_swipe_end(kb_swipe_tag_t mode_tag);
// タップ（発火なしで離した）時のフォールバック操作（任意実装）
__attribute__((weak)) void keyball_on_swipe_tap(kb_swipe_tag_t mode_tag);

// 方向別クールタイム（ms）を返すフック（未実装なら 0=無制限）
// - 例: VOL タグの左右のみ 500ms にする、PAD_A は全方向 300ms など。
__attribute__((weak)) uint16_t keyball_swipe_get_cooldown_ms(kb_swipe_tag_t tag, kb_swipe_dir_t dir);

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

// Debug helpers
void keyball_swipe_get_accum(uint32_t *r, uint32_t *l, uint32_t *d, uint32_t *u);

// 外部から1ステップ分の発火を指示（スワイプ有効時のみ有効）
void keyball_swipe_fire_once(kb_swipe_dir_t dir);

#ifdef HAPTIC_ENABLE
void keyball_swipe_haptic_pulse(void);
void keyball_swipe_haptic_reset_sequence(void);
void keyball_swipe_haptic_prepare_repeat(void);
#endif

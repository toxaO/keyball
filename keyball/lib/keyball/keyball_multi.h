#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "keyball_swipe.h"

// ユーザーオーバーライド用：
// 各 MULTI_* キー押下時に呼び出されるフック（弱シンボル）
// - mode_tag: スワイプ開始中ならそのタグ（KBS_TAG_*）、未開始なら 0
__attribute__((weak)) void keyball_on_multi_a(kb_swipe_tag_t mode_tag);
__attribute__((weak)) void keyball_on_multi_b(kb_swipe_tag_t mode_tag);
__attribute__((weak)) void keyball_on_multi_c(kb_swipe_tag_t mode_tag);
__attribute__((weak)) void keyball_on_multi_d(kb_swipe_tag_t mode_tag);

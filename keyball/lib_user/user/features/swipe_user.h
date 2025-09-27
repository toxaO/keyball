#pragma once
#include "lib/keyball/keyball_swipe.h"

extern bool canceller;  // 既存のあなたの変数を流用

// ユーザー独自タグ例: _Pad レイヤのフリック用
#define KBS_TAG_PAD_A     100

// FLICK_* 系（各ベースキーごとにタグを分ける）
#define KBS_TAG_FLICK_A   101
#define KBS_TAG_FLICK_D   102
#define KBS_TAG_FLICK_G   103
#define KBS_TAG_FLICK_J   104
#define KBS_TAG_FLICK_M   105
#define KBS_TAG_FLICK_P   106
#define KBS_TAG_FLICK_T   107
#define KBS_TAG_FLICK_W   108

static inline bool kb_is_flick_tag(kb_swipe_tag_t tag) {
    return ((tag >= KBS_TAG_FLICK_A && tag <= KBS_TAG_FLICK_W) || tag == KBS_TAG_PAD_A);
}

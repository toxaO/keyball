#pragma once

// user定義のモードタグ定数（KBは解釈しない）
enum {
    KBS_TAG_APP = 1,
    KBS_TAG_VOL,
    KBS_TAG_BRO,
    KBS_TAG_TAB,
    KBS_TAG_WIN,
};

extern bool canceller;  // 既存のあなたの変数を流用

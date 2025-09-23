#include "layer_user.h"
#include "quantum.h"
#include "../keycode_user.h"
#include "lib/keyball/keyball.h"

/*
#HSV_COLOR_CODE
------------------------------
HSV_AZURE 青み
HSV_BLACK/HSV_OFF
HSV_BLUE
HSV_CHARTREUSE 淡い黄緑
HSV_CORAL ピンクみ
HSV_CYAN
HSV_GOLD
HSV_GOLDENROD 黄土
HSV_GREEN
HSV_MAGENTA
HSV_ORANGE
HSV_PINK
HSV_PURPLE
HSV_RED
HSV_SPRINGGREEN 鮮やかな黄緑
HSV_TEAL 青緑
HSV_TURQUOISE ターコイズ
HSV_WHITE
HSV_YELLOW
------------------------------
*/

// 選択中のRGBアニメーションモード（設定レイヤで更新し、_Def に適用）
static uint8_t g_setting_rgb_mode = 0;
static bool    g_setting_mode_inited = false;
static uint8_t g_prev_highest_layer = 0xFFu;

// layer state setting ------------------------------
layer_state_t layer_state_set_user(layer_state_t state) {

    // 設定レイヤから離れたタイミングで、その時点のRGBモードを保持
    {
        uint8_t highest = get_highest_layer(state);
        if (!g_setting_mode_inited) {
            g_setting_rgb_mode   = rgblight_get_mode();
            g_setting_mode_inited = true;
        }
        g_prev_highest_layer = highest;
    }

    //LED------------------------------
    uint8_t layer = biton32(state);
    switch (layer) {
        case _Def:
            rgblight_mode_noeeprom(g_setting_rgb_mode);
            break;
        case _Scr:
            rgblight_mode_noeeprom(RGBLIGHT_MODE_SNAKE + 2);
            break;
    }

    return state;
}

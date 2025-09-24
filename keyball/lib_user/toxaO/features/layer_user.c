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

// _Scr 入退でRGBモードを一時切替するための保存領域
#ifdef RGBLIGHT_ENABLE
static uint8_t g_saved_rgb_mode = 0;
static bool    g_in_scr          = false;
#endif

// layer state setting ------------------------------
layer_state_t layer_state_set_user(layer_state_t state) {
    uint8_t highest = get_highest_layer(state);

#ifdef RGBLIGHT_ENABLE
    // _Scr に入ったら現在モードを保存し、SNAKEへ一時切替。
    // _Scr から離れたら保存しておいたモードへ戻す。
    if (highest == _Scr) {
        if (!g_in_scr) {
            g_saved_rgb_mode = rgblight_get_mode();
            g_in_scr = true;
        }
        rgblight_mode_noeeprom(RGBLIGHT_MODE_SNAKE + 2);
    } else {
        if (g_in_scr) {
            rgblight_mode_noeeprom(g_saved_rgb_mode);
            g_in_scr = false;
        }
    }
#endif

    return state;
}

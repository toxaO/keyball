#include "layer.h"
#include "quantum.h"
#include "my_keycode.h"
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

// layer state setting ------------------------------
layer_state_t layer_state_set_user(layer_state_t state) {

    // Auto enable scroll mode when the highest layer is 3
    keyball_set_scroll_mode(get_highest_layer(state) == _Scr);
    if (get_highest_layer(state) == _Set) {
      keyball_oled_set_mode(KB_OLED_MODE_DEBUG);
    };

    //LED------------------------------
    uint8_t layer = biton32(state);
    switch (layer) {
        case _Def:
            rgblight_mode_noeeprom(RGBLIGHT_MODE_BREATHING + 0);
            keyball_oled_set_mode(KB_OLED_MODE_NORMAL);
            break;
        case _Sym:
            rgblight_mode_noeeprom(RGBLIGHT_MODE_BREATHING + 3);
            break;
        case _mCur:
            rgblight_mode_noeeprom(RGBLIGHT_MODE_BREATHING + 3);
            break;
        case _wCur:
            rgblight_mode_noeeprom(RGBLIGHT_MODE_BREATHING + 3);
            break;
        case _mMou:
            rgblight_mode_noeeprom(RGBLIGHT_MODE_BREATHING + 2);
            break;
        case _wMou:
            rgblight_mode_noeeprom(RGBLIGHT_MODE_BREATHING + 2);
            break;
        case _NumP:
            rgblight_mode_noeeprom(RGBLIGHT_MODE_BREATHING + 0);
            break;
        case _Scr:
            rgblight_mode_noeeprom(RGBLIGHT_MODE_SNAKE + 2);
            break;
        case _Set:
            rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
            keyball_oled_set_mode(KB_OLED_MODE_DEBUG);
            break;
    }

    return state;
}

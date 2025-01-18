// layer state setting ------------------------------
layer_state_t layer_state_set_user(layer_state_t state) {

    // Auto enable scroll mode when the highest layer is 3
    keyball_set_scroll_mode(get_highest_layer(state) == 7);

    //LED------------------------------
    uint8_t layer = biton32(state);
    switch (layer) {
        case _Def:
            rgblight_sethsv(HSV_CYAN);
            rgblight_mode(RGBLIGHT_MODE_BREATHING + 0);
            break;
        case _Sym:
            rgblight_sethsv(HSV_PURPLE);
            rgblight_mode(RGBLIGHT_MODE_BREATHING + 3);
            break;
        case _mCur:
            rgblight_sethsv(HSV_GREEN);
            rgblight_mode(RGBLIGHT_MODE_BREATHING + 3);
            break;
        case _wCur:
            rgblight_sethsv(HSV_GREEN);
            rgblight_mode(RGBLIGHT_MODE_BREATHING + 3);
            break;
        case _mMou:
            rgblight_sethsv(HSV_BLUE);
            rgblight_mode(RGBLIGHT_MODE_BREATHING + 2);
            break;
        case _wMou:
            rgblight_sethsv(HSV_BLUE);
            rgblight_mode(RGBLIGHT_MODE_BREATHING + 2);
            break;
        case _NumP:
            rgblight_sethsv(HSV_ORANGE);
            rgblight_mode(RGBLIGHT_MODE_BREATHING + 0);
            break;
        case _Scr:
            rgblight_sethsv(HSV_BLUE);
            rgblight_mode(RGBLIGHT_MODE_SNAKE + 2);
            break;
    }

    return state;
}

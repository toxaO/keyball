#include "quantum.h"
#include "../features/oled_user.h"
#include "../keycode_user.h"
#include "lib/keyball/keyball.h"

void oled_render_layer_state(void) {
  oled_write_ln(PSTR("Lay:"), false);
  switch (get_highest_layer(layer_state | default_layer_state)) {
    case _Def:
      oled_write_P(PSTR("  Def"), false);
      break;
    case _NumP:
      oled_write_P(PSTR(" NumP"), false);
      break;
    case _Sym:
      oled_write_P(PSTR("  Sym /Num"), false);
      break;
    case _mCur:
      oled_write_P(PSTR("  Cur/  Fn"), false);
      break;
    case _wCur:
      oled_write_P(PSTR("  Cur/  Fn"), false);
      break;
    case _mMou:
      oled_write_P(PSTR("Mouse"), false);
      break;
    case _wMou:
      oled_write_P(PSTR("Mouse"), false);
      break;
    case _Scr:
      oled_write_P(PSTR(" Scrl"), false);
      break;
    case _Pad:
      oled_write_P(PSTR("  Pad"), false);
      break;
    case _Set:
      oled_write_P(PSTR("  Set"), false);
      break;
  }
}


// OLEDの実装
void oledkit_render_info_user(void) {
  if (keyball_oled_get_mode() == KB_OLED_MODE_SETTING) {
    // 設定モード（旧デバッグ）：設定ページを描く
    keyball_oled_render_setting();
  } else {
    // 通常モード：レイヤー中心（最小限）
    // oled_render_layer_state();  // ← 既存のあなたの関数

    // oled_render_info_layer();
    // oled_render_info_layer_default();
    oled_render_info_ball();
    oled_render_info_keycode();
    oled_render_info_mods();
    oled_render_info_cpi();
    oled_render_info_scroll_step();
    // oled_render_info_swipe_tag();
    // oled_render_info_key_pos();
  }
}

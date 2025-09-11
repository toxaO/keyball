#include "quantum.h"
#include "oled.h"
#include "my_keycode.h"
#include "lib/keyball/keyball.h"

void oled_render_layer_state(void) {
  oled_write_P(PSTR("Layer: "), false);
  switch (get_highest_layer(layer_state)) {
    case _Def:
      oled_write_ln_P(PSTR("Default   "), false);
      break;
    case _NumP:
      oled_write_ln_P(PSTR("NumPad  "), false);
      break;
    case _Sym:
      oled_write_ln_P(PSTR("Sym / Num "), false);
      break;
    case _mCur:
      oled_write_ln_P(PSTR("Cur / Func"), false);
      break;
    case _wCur:
      oled_write_ln_P(PSTR("Cur / Func"), false);
      break;
    case _mMou:
      oled_write_ln_P(PSTR("Mouse     "), false);
      break;
    case _wMou:
      oled_write_ln_P(PSTR("Mouse     "), false);
      break;
    case _Scr:
      oled_write_ln_P(PSTR("Scroll    "), false);
      break;
    case _Pad:
      oled_write_ln_P(PSTR("Pad       "), false);
      break;
    case _Set:
      oled_write_ln_P(PSTR("Setting   "), false);
      break;
  }
}


// OLEDの実装
void oledkit_render_info_user(void) {
  if (keyball_oled_get_mode() == KB_OLED_MODE_DEBUG) {
    // デバッグモード：デバッグページだけ描く
    keyball_oled_render_debug();
  } else {
    // 通常モード：レイヤー中心（最小限）
    oled_render_layer_state();  // ← 既存のあなたの関数
                                // 必要なら 1〜2 行だけサブ情報を足す例：
                                // keyball_oled_render_ballinfo();      // CPI/DIVを見たい時だけ
                                // keyball_oled_render_ballsubinfo();   // Move shapingの簡易表示
  }
}

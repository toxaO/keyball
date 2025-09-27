#include "quantum.h"
#include "../features/oled_user.h"
#include "../keycode_user.h"
#include "lib/keyball/keyball.h"


// OLEDの実装
void oledkit_render_info_user(void) {
  if (keyball_oled_get_mode() == KB_OLED_MODE_SETTING) {
    // 設定モード（旧デバッグ）：設定ページを描く
    keyball_oled_render_setting();

  } else {
    // 通常モード：レイヤー中心（最小限）
    // 順番は好きに入れ替えてください

    oled_render_info_layer(); // 現在のレイヤー表示
    // oled_render_info_layer_default(); // 現在のデフォルトレイヤー
    // oled_render_info_ball(); // トラックボールの現在値
    // oled_render_info_keycode(); // 送信キーコード
    oled_render_info_mods(); // modifier keyの状態 順番にShift, Ctrl, Gui, alt
    oled_render_info_mods_oneshot(); // one shot modifier keyの状態 順番にShift, Ctrl, Gui, alt
    oled_render_info_mods_lock(); // modifier keyのlock状態 順番にShift, Ctrl, Gui, alt, Caps
    // oled_render_info_cpi(); // ポインターの速度
    // oled_render_info_scroll_step(); // スクロール速度
    // oled_render_info_swipe_tag(); // スワイプ状態
    // oled_render_info_key_pos(); // 押したキーの位置
  }
}

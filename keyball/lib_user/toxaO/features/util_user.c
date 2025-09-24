#include "quantum.h"
#include "util_user.h"
#include "os_detection.h"
#include "lib/keyball/keyball.h"
#include "../keycode_user.h"

int host_os;

void keyboard_post_init_user(void) {
    host_os = detected_host_os();
    // Auto Mouse Layer 初期デフォルト（EEPROM未設定時のみ）
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
    if (kbpf.aml_layer == 0xFFu) {
      set_auto_mouse_layer(3);
      // ここではレイヤの既定値のみを設定し、有効化はしない。
      // 有効/無効は永続化済み設定（kbpf.aml_enable）に従い、
      // 利用者が `AML_TO` などで明示的に切り替える前提とする。
    }
#endif
}

#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
// スワイプ始動キーをマウスキーとして扱うことで、
// AMLの「マウスキー押下中はレイヤ保持」ロジックに乗せる
bool is_mouse_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case APP_SW:
    case VOL_SW:
    case BRO_SW:
    case TAB_SW:
    case WIN_SW:
      return true; // マウスキーとして扱う
    default:
      return false;
  }
}
#endif

// // 自前の絶対数を返す関数。 Functions that return absolute numbers.
// int16_t my_abs(int16_t num) {
//   if (num < 0) {
//     num = -num;
//   }
//   return num;
// }


void tap_code16_with_oneshot(uint16_t keycode) {
  uint8_t osm = get_oneshot_mods();
  if (osm) {
    add_weak_mods(osm);
    send_keyboard_report();
  }
  tap_code16(keycode);
  if (osm) {
    del_weak_mods(osm);
    send_keyboard_report();
    clear_oneshot_mods();
  }
}

void tap_code16_os(
    uint16_t win,
    uint16_t mac,
    uint16_t ios,
    uint16_t linux,
    uint16_t unsure) {
  switch (host_os) {
    case OS_WINDOWS:
      tap_code16_with_oneshot(win);
      break;
    case OS_MACOS:
      tap_code16_with_oneshot(mac);
      break;
    case OS_IOS:
      tap_code16_with_oneshot(ios);
      break;
    case OS_LINUX:
      tap_code16_with_oneshot(linux);
      break;
    case OS_UNSURE:
      tap_code16_with_oneshot(unsure);
      break;
  }
}

// void reset_eeprom(void) {
//   eeconfig_init();
// }

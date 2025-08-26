#include "quantum.h"
#include "util.h"
#include "os_detection.h"
#include "lib/keyball/keyball.h"

int host_os;

// 右手だけ光らせるレイヤ（例：右手の先頭から4灯をシアン）
const rgblight_segment_t PROGMEM right_only_layer[] =
    RGBLIGHT_LAYER_SEGMENTS({ LEFT_LEDS + 0, 14, HSV_CYAN });

// （必要なら他のレイヤも足せる）
const rgblight_segment_t* const PROGMEM my_rgb_layers[] =
    RGBLIGHT_LAYERS_LIST(
        right_only_layer          // レイヤID 0
    );

void keyboard_post_init_user(void) {
    host_os = detected_host_os();
    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
    rgblight_layers = my_rgb_layers;
}

// 自前の絶対数を返す関数。 Functions that return absolute numbers.
int16_t my_abs(int16_t num) {
  if (num < 0) {
    num = -num;
  }
  return num;
}


void tap_code16_os(
    uint16_t win,
    uint16_t mac,
    uint16_t ios,
    uint16_t linux,
    uint16_t unsure) {
  switch (host_os) {
    case OS_WINDOWS:
      tap_code16(win);
      break;
    case OS_MACOS:
      tap_code16(mac);
      break;
    case OS_IOS:
      tap_code16(ios);
      break;
    case OS_LINUX:
      tap_code16(linux);
      break;
    case OS_UNSURE:
      tap_code16(unsure);
      break;
  }
}

void reset_eeprom(void) {
  eeconfig_init();
}

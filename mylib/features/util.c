#include "quantum.h"
#include "util.h"
#include "os_detection.h"
#include "lib/keyball/keyball.h"

int host_os;

void keyboard_post_init_user(void) {
    host_os = detected_host_os();
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

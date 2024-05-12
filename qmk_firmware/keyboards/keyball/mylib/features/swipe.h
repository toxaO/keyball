/* Copyright 2023 kamidai (@d_kamiichi)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// 自前の絶対数を返す関数。 Functions that return absolute numbers.
int16_t my_abs(int16_t num) {
  if (num < 0) {
    num = -num;
  }
  return num;
}

// 自前の符号を返す関数。 Function to return the sign.
int16_t mmouse_move_y_sign(int16_t num) {
  if (num < 0) {
    return -1;
  }
  return 1;
}


// swipe implement
int16_t current_keycode;
const int16_t SWIPE_THRESHOLD = 10;
bool is_swiped = false;
bool canceller = false;
enum { NORMAL = 0, HIGH, VERY_HIGH } repeat_speed = NORMAL;
/* enum repeat_speed swipe_speed; */

// スワイプジェスチャーで何が起こるかを実際に処理する関数
// 上、下、左、右、スワイプなしの5つのオプションがあります
void process_swipe_gesture(int16_t x, int16_t y) {
  // APP_SWIPE
  // desktop control
  if (current_keycode == APP_SWIPE) {
    if (my_abs(x) < my_abs(y)) {
      if (y < 0) { // swipe up
        if (canceller) {
            tap_code(KC_ESC);
            canceller = false;
        } else {
            tap_code16(G(KC_SPACE)); // spotlight
            canceller = true;
        }
      } else { // swipe down
        if (canceller) {
            tap_code(KC_ESC);
            canceller = false;
        } else {
            tap_code16(C(KC_DOWN)); // app control
            canceller = true;
        }
      }
    }
    if (my_abs(x) > my_abs(y)) {
      if (x < 0) { // swipe left
        tap_code16(C(KC_RIGHT)); // right desktop
      } else { // swipe right
        tap_code16(C(KC_LEFT)); // left desktop
      }
    }
  }

  // VOL_SWIPE
  if (current_keycode == VOL_SWIPE) {
    if (my_abs(x) < my_abs(y)) {
      if (y < 0) { // swipe up
        tap_code(KC_VOLU);
        repeat_speed = VERY_HIGH;
      } else { // swipe down
        tap_code(KC_VOLD);
        repeat_speed = VERY_HIGH;
      }
    }
    if (my_abs(x) > my_abs(y)) {
      if (x < 0) { // swipe left
        tap_code(KC_MNXT);
      } else { // swipe right
        tap_code(KC_MPRV);
      }
    }
  }

  // BROWSE_SWIPE
  // browse control
  if (current_keycode == BROWSE_SWIPE) {
    if (my_abs(x) < my_abs(y)) {
      if (y < 0) { // swipe up
        tap_code16(G(KC_C)); // copy
      } else { // swipe down
        tap_code16(G(KC_V)); // paste
      }
    }
    if (my_abs(x) > my_abs(y)) {
      if (x < 0) { // swipe left
        tap_code16(G(KC_LEFT)); // browse back
      } else { // swipe right
        tap_code16(G(KC_RIGHT)); // browse forward
      }
    }
  }

  // TAB_SWIPE
  // TAB
  if (current_keycode == TAB_SWIPE) {
    if (my_abs(x) < my_abs(y)) {
      if (y < 0) { // swipe up
        tap_code16(S(G(KC_T))); // new tab
      } else { // swipe down
        tap_code16(G(KC_W)); // tab close
      }
    }
    if (my_abs(x) > my_abs(y)) {
      if (x < 0) { // swipe left
        tap_code16(S(C(KC_TAB))); // next tab
        repeat_speed = HIGH;
      } else { // swipe right
        tap_code16(C(KC_TAB)); // previous tab
        repeat_speed = HIGH;
      }
    }
  }

  // MAG_SWIPE
  // MEGNET
  if (current_keycode == MAG_SWIPE) {
    if (my_abs(x) < my_abs(y)) {
      if (y < 0) { // swipe up
        tap_code16(C(A(KC_UP)));
      } else { // swipe down
        tap_code16(C(A(KC_DOWN)));
      }
    }
    if (my_abs(x) > my_abs(y)) {
      if (x < 0) { // swipe left
        tap_code16(C(A(KC_LEFT)));
      } else { // swipe right
        tap_code16(C(A(KC_RIGHT)));
      }
    }
  }
}


// swipe mode
enum ball_state {
  NONE = 0,
  SWIPE,      // スワイプモードが有効になりスワイプ入力が取れる。 Swipe mode is enabled to take swipe input.
  SWIPING     // スワイプ中。 swiping.
};

enum ball_state state;  // 現在のクリック入力受付の状態 Current click input reception status
uint16_t click_timer;   // タイマー。状態に応じて時間で判定する。 Timer. Time to determine the state of the system.
uint16_t swipe_timer; // スワイプキーがTAPPING_TERMにあるかを判定する (≒ mod_tap)

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
  int16_t current_x = mouse_report.x;
  int16_t current_y = mouse_report.y;

  if (current_x != 0 || current_y != 0) {
    switch (state) {
      case SWIPE:
        click_timer = timer_read();

        if (my_abs(current_x) >= SWIPE_THRESHOLD || my_abs(current_y) >= SWIPE_THRESHOLD) {
          rgblight_sethsv(HSV_PINK);
          process_swipe_gesture(current_x, current_y);
          is_swiped = true;
          state = SWIPING;
        }
        break;

      default:
        break;
    }

  } else {
    switch (state) {
      case SWIPE:
        rgblight_sethsv(HSV_SPRINGGREEN);
        break;

      case SWIPING:
        switch (repeat_speed) {
          case NORMAL:
            if (timer_elapsed(click_timer) > 300) {
              state = SWIPE;
            }
            break;

          case HIGH:
            if (timer_elapsed(click_timer) > 150) {
              state = SWIPE;
            }
            break;

          case VERY_HIGH:
            if (timer_elapsed(click_timer) > 80) {
              state = SWIPE;
            }
            break;
        }
        break;

      default:
        break;
    }
  }

  if (state == SWIPE || state == SWIPING) {
      mouse_report.x = 0;
      mouse_report.y = 0;
  }

  return mouse_report;
}

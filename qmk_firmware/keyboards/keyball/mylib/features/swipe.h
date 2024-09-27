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
/* int16_t swipemode; */
const int16_t SWIPE_THRESHOLD = 10;
const int16_t DIRECTION_THRESHOLD = 10;
bool is_swiped = false;
bool canceller = false;
enum { NORMAL = 0, HIGH, VERY_HIGH } repeat_speed = NORMAL;
enum { NO_SW = 0, APP_SW, VOL_SW, BRO_SW, TAB_SW, WIN_SW } swipemode = NO_SW;

// スワイプジェスチャーで何が起こるかを実際に処理する関数
// 上、下、左、右、スワイプなしの5つのオプションがあります
// スワイプなしに関してはmacrokey.hに記載する
void process_swipe_gesture(int16_t x, int16_t y) {
  // APP_SWIPE
  // desktop control
  if (swipemode == APP_SW) {
    if ((my_abs(x) < my_abs(y)) && (DIRECTION_THRESHOLD < my_abs(y))) {
      if (y < 0) { // swipe up
        if (canceller) {
            tap_code(KC_ESC);
            canceller = false;
        } else {
            if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
              tap_code16(G(KC_SPACE)); // spotlight
            } else {
              tap_code16(KC_LGUI); // windows key
            }
            canceller = true;
        }
      } else { // swipe down
        if (canceller) {
            tap_code(KC_ESC);
            canceller = false;
        } else {
            if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
              tap_code16(C(KC_DOWN)); // spotlight
            } else {
              tap_code16(G(KC_TAB)); // windows key
            }
            canceller = true;
        }
      }
    }
    // ここでelseを使ってしまうとポインターが動いていない時にも反応してしまう
    if (( my_abs(x) > my_abs(y)) && (DIRECTION_THRESHOLD < my_abs(x))) {
      if (x < 0) { // swipe left
        if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
          tap_code16(m_R_DESK); // spotlight
        } else {
          tap_code16(w_R_DESK); // windows key
        }
      } else { // swipe right
        if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
          tap_code16(m_L_DESK); // spotlight
        } else {
          tap_code16(w_L_DESK); // windows key
        }
      }
    }
  }

  // VOL_SWIPE
  if (swipemode == VOL_SW) {
    if ((my_abs(x) < my_abs(y)) && (DIRECTION_THRESHOLD < my_abs(y))) {
      if (y < 0) { // swipe up
        tap_code(KC_VOLU);
        repeat_speed = VERY_HIGH;
      } else { // swipe down
        tap_code(KC_VOLD);
        repeat_speed = VERY_HIGH;
      }
    }
    if (( my_abs(x) > my_abs(y)) && (DIRECTION_THRESHOLD < my_abs(x))) {
      if (x < 0) { // swipe left
        tap_code(KC_MNXT);
      } else { // swipe right
        tap_code(KC_MPRV);
      }
    }
  }

  // BROWSE_SWIPE
  // browse control
  if (swipemode == BRO_SW) {
    if ((my_abs(x) < my_abs(y)) && (DIRECTION_THRESHOLD < my_abs(y))) {
      if (y < 0) { // swipe up
        if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
          tap_code16(G(KC_C)); // copy
        } else {
          tap_code16(C(KC_C)); // windows key
        }
      } else { // swipe down
        if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
          tap_code16(G(KC_V)); // copy
        } else {
          tap_code16(C(KC_V)); // windows key
        }
      }
    }
    if (( my_abs(x) > my_abs(y)) && (DIRECTION_THRESHOLD < my_abs(x))) {
      if (x < 0) { // swipe left
        if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
          tap_code16(G(KC_LEFT)); // browse back
        } else {
          tap_code16(KC_WBAK); // windows key
        }
      } else { // swipe right
        if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
          tap_code16(G(KC_RIGHT)); // browse back
        } else {
          tap_code16(KC_WFWD); // windows key
        }
      }
    }
  }

  // TAB_SWIPE
  // TAB
  if (swipemode == TAB_SW) {
    if ((my_abs(x) < my_abs(y)) && (DIRECTION_THRESHOLD < my_abs(y))) {
      if (y < 0) { // swipe up
        if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
          tap_code16(m_NEW_TAB); // browse back
        } else {
          tap_code16(w_NEW_TAB); // windows key
        }
      } else { // swipe down
        if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
          tap_code16(m_CLOSE); // browse back
        } else {
          tap_code16(w_CLOSE); // windows key
        }
      }
    }
    if (( my_abs(x) > my_abs(y)) && (DIRECTION_THRESHOLD < my_abs(x))) {
      if (x < 0) { // swipe left
        tap_code16(S(C(KC_TAB))); // next tab
        repeat_speed = HIGH;
      } else { // swipe right
        tap_code16(C(KC_TAB)); // previous tab
        repeat_speed = HIGH;
      }
    }
  }

  // WIN_SWIPE
  // MEGNET
  if (swipemode == WIN_SW) {
    if ((my_abs(x) < my_abs(y)) && (DIRECTION_THRESHOLD < my_abs(y))) {
      if (y < 0) { // swipe up
        if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
          tap_code16(MGN_U); // browse back
        } else {
          tap_code16(G(KC_UP)); // windows key
        }
      } else { // swipe down
        if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
          tap_code16(MGN_D); // browse back
        } else {
          tap_code16(G(KC_DOWN)); // windows key
        }
      }
    }
    if (( my_abs(x) > my_abs(y)) && (DIRECTION_THRESHOLD < my_abs(x))) {
      if (x < 0) { // swipe left
        if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
          tap_code16(MGN_L); // browse back
        } else {
          tap_code16(G(KC_LEFT)); // windows key
        }
      } else { // swipe right
        if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS){
          tap_code16(MGN_R); // browse back
        } else {
          tap_code16(G(KC_RIGHT)); // windows key
        }
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

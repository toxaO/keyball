
/* // swipe implement */
/* #include <stdint.h> */
/* #include "util.h" */
/* #include "swipe.h" */
/* #include "quantum.h" */
/* #include "my_keycode.h" */
/* #include "os_detection.h" */

/* const int16_t SWIPE_THRESHOLD = 7; */
/* bool is_swiped = false; */
/* bool canceller = false; */

/* /1* typedef enum { NORMAL = 0, HIGH, VERY_HIGH } RepeatSpeed; *1/ */
/* /1* typedef enum { NO_SW = 0, APP_SW, VOL_SW, BRO_SW, TAB_SW, WIN_SW } SwipeMode; *1/ */
/* /1* typedef enum { SW_NO = 0, SW_UP, SW_DOWN, SW_RIGHT, SW_LEFT } SwipeDirection; *1/ */
/* /1* // swipe mode *1/ */
/* /1* typedef enum { *1/ */
/* /1*   NONE = 0, *1/ */
/* /1*   SWIPE,      // スワイプモードが有効になりスワイプ入力が取れる。 Swipe mode is enabled to take swipe input. *1/ */
/* /1*   SWIPING     // スワイプ動作中。 swiping. *1/ */
/* /1* } SwipeState; *1/ */


/* RepeatSpeed repeat_speed = NORMAL; */
/* SwipeMode swipe_mode = NO_SW; */
/* SwipeState state = NONE; */

/* // スワイプの方向を判断する関数 */
/* SwipeDirection get_swipe_dirction(int16_t x, int16_t y) { */

/*     int16_t abs_x = my_abs(x); */
/*     int16_t abs_y = my_abs(y); */

/*     if ( (abs_y / abs_x) < 0.83 || 1.2 < (abs_y / abs_x) ) { */
/*         if (abs_x > abs_y) { */
/*             // x方向のスワイプ */
/*             return (x < 0) ? SW_LEFT : SW_RIGHT; */
/*         } else { */
/*             // y方向のスワイプ */
/*             return (y < 0) ? SW_UP : SW_DOWN; */
/*         } */
/*     } */

/*     // 閾値を超えない場合 */
/*     return SW_NO; */
/* } */

/* // スワイプジェスチャーで何が起こるかを実際に処理する関数 */
/* // 上、下、左、右、スワイプなしの5つのオプションがあります */
/* // スワイプなしに関してはmacrokey.hに記載する */
/* void process_swipe_gesture(int16_t x, int16_t y) { */
/*   // APP_SWIPE */
/*   // desktop control */
/*   switch (swipe_mode) { */

/*     case APP_SW: */
/*       switch (get_swipe_dirction( x, y)) { */
/*         case SW_UP: */
/*           if (canceller) { */
/*             tap_code(KC_ESC); */
/*             canceller = false; */
/*           } else { */
/*             tap_code16_os(w_Ueli, G(KC_SPACE), G(KC_SPACE), KC_NO, KC_NO); */
/*             canceller = true; */
/*           } */
/*           break; */

/*         case SW_DOWN: */
/*           if (canceller) { */
/*             tap_code(KC_ESC); */
/*             canceller = false; */
/*           } else { */
/*             tap_code16_os(G(KC_TAB), C(KC_DOWN), C(KC_DOWN), KC_NO, KC_NO); */
/*             canceller = true; */
/*           } */
/*           break; */

/*         case SW_LEFT: */
/*           tap_code16_os(w_R_DESK, m_R_DESK, m_R_DESK, KC_NO, KC_NO); */
/*           break; */

/*         case SW_RIGHT: */
/*           tap_code16_os(w_L_DESK, m_L_DESK, m_L_DESK, KC_NO, KC_NO); */
/*           break; */

/*         default: */
/*           break; */

/*       } */
/*       break; */

/*     case VOL_SW: */
/*       switch (get_swipe_dirction(x, y)) { */
/*         case SW_UP: */
/*           tap_code(KC_VOLU); */
/*           repeat_speed = VERY_HIGH; */
/*           break; */

/*         case SW_DOWN: */
/*           tap_code(KC_VOLD); */
/*           repeat_speed = VERY_HIGH; */
/*           break; */

/*         case SW_LEFT: */
/*           tap_code(KC_MNXT); */
/*           break; */

/*         case SW_RIGHT: */
/*           tap_code(KC_MPRV); */
/*           break; */

/*         default: */
/*           break; */
/*       } */
/*       break; */

/*     case BRO_SW: */
/*       switch (get_swipe_dirction(x, y)) { */
/*         case SW_UP: */
/*           tap_code16_os(C(KC_C), G(KC_C), G(KC_C), KC_NO, KC_NO); */
/*           break; */

/*         case SW_DOWN: */
/*           tap_code16_os(C(KC_V), G(KC_V), G(KC_V), KC_NO, KC_NO); */
/*           break; */

/*         case SW_LEFT: */
/*           tap_code16_os(KC_WBAK, G(KC_LEFT), G(KC_LEFT), KC_NO, KC_NO); */
/*           break; */

/*         case SW_RIGHT: */
/*           tap_code16_os(KC_WFWD, G(KC_RIGHT), G(KC_RIGHT), KC_NO, KC_NO); */
/*           break; */

/*         default: */
/*           break; */
/*       } */
/*       break; */

/*     case TAB_SW: */
/*       switch (get_swipe_dirction(x, y)) { */
/*         case SW_UP: */
/*           tap_code16_os(S(C(KC_T)), S(G(KC_T)), S(G(KC_T)), KC_NO, KC_NO); */
/*           break; */

/*         case SW_DOWN: */
/*           tap_code16_os(w_CLOSE, m_CLOSE, m_CLOSE, KC_NO, KC_NO); */
/*           break; */

/*         case SW_LEFT: */
/*           tap_code16(S(C(KC_TAB))); // next tab */
/*           repeat_speed = HIGH; */
/*           break; */

/*         case SW_RIGHT: */
/*           tap_code16(C(KC_TAB)); // previous tab */
/*           repeat_speed = HIGH; */
/*           break; */

/*         default: */
/*           break; */
/*       } */
/*       break; */

/*     case WIN_SW: */
/*       switch (get_swipe_dirction(x, y)) { */
/*         case SW_UP: */
/*           switch (host_os) { */
/*             case OS_MACOS: */
/*             case OS_IOS: */
/*               tap_code16(MGN_U); // browse back */
/*               break; */

/*             case OS_WINDOWS: */
/*               register_code(KC_LGUI); */
/*               tap_code(KC_UP); */
/*               break; */
/*           } */
/*           break; */

/*         case SW_DOWN: */
/*           switch (host_os) { */
/*             case OS_MACOS: */
/*             case OS_IOS: */
/*               tap_code16(MGN_D); // browse back */
/*               break; */

/*             case OS_WINDOWS: */
/*               register_code(KC_LGUI); */
/*               tap_code(KC_DOWN); */
/*               break; */
/*           } */
/*           break; */

/*         case SW_LEFT: */
/*           switch (host_os) { */
/*             case OS_MACOS: */
/*             case OS_IOS: */
/*               tap_code16(MGN_L); // browse back */
/*               break; */

/*             case OS_WINDOWS: */
/*               register_code(KC_LGUI); */
/*               tap_code(KC_LEFT); */
/*               break; */
/*           } */
/*           break; */

/*         case SW_RIGHT: */
/*           switch (host_os) { */
/*             case OS_MACOS: */
/*             case OS_IOS: */
/*               tap_code16(MGN_R); // browse back */
/*               break; */

/*             case OS_WINDOWS: */
/*               register_code(KC_LGUI); */
/*               tap_code(KC_RIGHT); */
/*               break; */
/*           } */
/*           break; */

/*         default: */
/*           break; */
/*       } */
/*       break; */

/*     default: */
/*       break; */
/*   } */
/* } */

/* uint16_t click_timer;   // タイマー。状態に応じて時間で判定する。 Timer. Time to determine the state of the system. */
/* uint16_t swipe_timer; // スワイプキーがTAPPING_TERMにあるかを判定する (≒ mod_tap) */

/* report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) { */
/*   int16_t current_x = mouse_report.x; */
/*   int16_t current_y = mouse_report.y; */

/*   if (current_x != 0 || current_y != 0) { */
/*     switch (state) { */
/*       case SWIPE: */
/*         click_timer = timer_read(); */

/*         if (my_abs(current_x) >= SWIPE_THRESHOLD || my_abs(current_y) >= SWIPE_THRESHOLD) { */
/*           process_swipe_gesture(current_x, current_y); */
/*           is_swiped = true; */
/*           state = SWIPING; */
/*         } */
/*         break; */

/*       default: */
/*         break; */
/*     } */

/*   } else { */
/*     // swipe中はmouse.reportの値を0にしている（下記のコード参照） */
/*     switch (state) { */
/*       case SWIPING: */
/*         switch (repeat_speed) { */
/*           case NORMAL: */
/*             if (timer_elapsed(click_timer) > 300) { */
/*               state = SWIPE; */
/*             } */
/*             break; */

/*           case HIGH: */
/*             if (timer_elapsed(click_timer) > 120) { */
/*               state = SWIPE; */
/*             } */
/*             break; */

/*           case VERY_HIGH: */
/*             if (timer_elapsed(click_timer) > 80) { */
/*               state = SWIPE; */
/*             } */
/*             break; */
/*         } */
/*         break; */

/*       default: */
/*         break; */
/*     } */
/*   } */

/*   if (state == SWIPE || state == SWIPING) { */
/*       mouse_report.x = 0; */
/*       mouse_report.y = 0; */
/*   } */

/*   return mouse_report; */
/* } */

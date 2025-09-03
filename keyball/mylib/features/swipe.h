#ifndef MYLIB_SWIPE_H
#define MYLIB_SWIPE_H

#include <stdint.h>
#include "util.h"

extern bool canceller;
extern bool is_swiped;

extern uint16_t click_timer;   // タイマー。状態に応じて時間で判定する。 Timer. Time to determine the state of the system.
extern uint16_t swipe_timer; // スワイプキーがTAPPING_TERMにあるかを判定する (≒ mod_tap)

typedef enum { NO_SW = 0, APP_SW, VOL_SW, BRO_SW, TAB_SW, WIN_SW } SwipeMode;
typedef enum { SW_NO = 0, SW_UP, SW_DOWN, SW_RIGHT, SW_LEFT } SwipeDirection;
typedef enum { NONE = 0, SWIPE, SWIPING } SwipeState;

extern SwipeMode swipe_mode;
extern SwipeState state;

#endif

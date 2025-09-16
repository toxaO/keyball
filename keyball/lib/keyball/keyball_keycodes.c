#include "keyball_keycodes.h"
#include "keyball.h"
#include "keyball_move.h"
#include "keyball_oled.h"
#include "keyball_scroll.h"
#include "os_detection.h"
#include "keyball_swipe.h"
#include "keyball_multi.h"
#include "timer.h"
#if defined(DYNAMIC_KEYMAP_ENABLE)
#    include "dynamic_keymap.h"
#endif

#define _CONSTRAIN(amt, low, high)                                             \
  ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// (removed) CPI adjustment helper no longer used

static uint16_t g_swipe_keydown_ms = 0;

bool keyball_process_keycode(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
  case SCRL_MO:
    keyball_set_scroll_mode(record->event.pressed);
    return true;
  }

  if (record->event.pressed) {
    switch (keycode) {
    // Configuration
  case KBC_RST:
      kbpf_defaults();
      keyball_set_cpi(kbpf.cpi[keyball_os_idx()]);
      keyball_set_scroll_div(kbpf.scroll_step[keyball_os_idx()]);
      g_move_gain_lo_fp = kbpf.move_gain_lo_fp[keyball_os_idx()];
      g_move_th1 = kbpf.move_th1[keyball_os_idx()];
      keyball_swipe_set_step(kbpf.swipe_step);
      keyball_swipe_set_deadzone(kbpf.swipe_deadzone);
      keyball_swipe_set_reset_ms(kbpf.swipe_reset_ms);
      keyball_swipe_set_freeze(kbpf.swipe_freeze);
      keyball_swipe_end();
      kbpf_write();
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
      set_auto_mouse_enable(kbpf.aml_enable ? true : false);
      set_auto_mouse_timeout(kbpf.aml_timeout);
      if (kbpf.aml_layer != 0xFFu) set_auto_mouse_layer(kbpf.aml_layer);
#endif
      break;

    case KBC_SAVE:
      kbpf.move_gain_lo_fp[keyball_os_idx()] =
          (uint8_t)_CONSTRAIN(g_move_gain_lo_fp, 1, 255);
      kbpf.move_th1[keyball_os_idx()] =
          (uint8_t)_CONSTRAIN(g_move_th1, 0, kbpf.move_th2[keyball_os_idx()] - 1);
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
      kbpf.aml_enable  = get_auto_mouse_enable() ? 1 : 0;
      kbpf.aml_timeout = get_auto_mouse_timeout();
      kbpf.aml_layer   = get_auto_mouse_layer();
#endif
#if KEYBALL_SCROLLSNAP_ENABLE == 2
      kbpf.scrollsnap_mode = (uint8_t)keyball_get_scrollsnap_mode();
#endif
      kbpf_write(); // OSごとの全データ+グローバルを一括保存
      dprintf("KBPF saved (ver=%u) AML(en=%u,tg=%u,to=%u) SSNP=%u\n",
              kbpf.version,
              (unsigned)kbpf.aml_enable,
              (unsigned)kbpf.aml_layer,
              (unsigned)kbpf.aml_timeout,
              (unsigned)kbpf.scrollsnap_mode);
      break;

    // Scroll control (keep only mode toggles)
    case SCRL_TO:
      keyball_set_scroll_mode(!keyball_get_scroll_mode());
      break;
    // Setting view controls
    case STG_TOG:
      keyball_oled_mode_toggle();
      return false;
    case STG_NP:
      keyball_oled_next_page();
      return false;
    case STG_PP:
      keyball_oled_prev_page();
      return false;

    // Swipe action keys: begin on press
    case APP_SW: g_swipe_keydown_ms = timer_read(); keyball_swipe_begin(1); return false; // KBS_TAG_APP (=1)
    case VOL_SW: g_swipe_keydown_ms = timer_read(); keyball_swipe_begin(2); return false; // KBS_TAG_VOL (=2)
    case BRO_SW: g_swipe_keydown_ms = timer_read(); keyball_swipe_begin(3); return false; // KBS_TAG_BRO (=3)
    case TAB_SW: g_swipe_keydown_ms = timer_read(); keyball_swipe_begin(4); return false; // KBS_TAG_TAB (=4)
    case WIN_SW: g_swipe_keydown_ms = timer_read(); keyball_swipe_begin(5); return false; // KBS_TAG_WIN (=5)
    case SW_ARR: g_swipe_keydown_ms = timer_read(); keyball_swipe_begin(KBS_TAG_ARR); return false; // Arrow proxy swipe

    // Arrow keys as swipe-direction proxies while swipe mode held
    case KC_LEFT:
    case KC_RIGHT:
    case KC_UP:
    case KC_DOWN:
      if (keyball_swipe_is_active()) {
        kb_swipe_dir_t dir = (keycode == KC_LEFT) ? KB_SWIPE_LEFT
                             : (keycode == KC_RIGHT) ? KB_SWIPE_RIGHT
                             : (keycode == KC_UP) ? KB_SWIPE_UP
                                                  : KB_SWIPE_DOWN;
        keyball_swipe_fire_once(dir);
        return false; // consume
      }
      return true;

    // MULTI keys: A,B,C,D
    case MULTI_A: {
      kb_swipe_tag_t tag = keyball_swipe_is_active() ? keyball_swipe_mode_tag() : 0;
      if (keyball_on_multi_a) keyball_on_multi_a(tag); else tap_code16(KC_F1);
      return false;
    }
    case MULTI_B: {
      kb_swipe_tag_t tag = keyball_swipe_is_active() ? keyball_swipe_mode_tag() : 0;
      if (keyball_on_multi_b) keyball_on_multi_b(tag); else tap_code16(KC_F2);
      return false;
    }
    case MULTI_C: {
      kb_swipe_tag_t tag = keyball_swipe_is_active() ? keyball_swipe_mode_tag() : 0;
      if (keyball_on_multi_c) keyball_on_multi_c(tag); else tap_code16(KC_F3);
      return false;
    }
    case MULTI_D: {
      kb_swipe_tag_t tag = keyball_swipe_is_active() ? keyball_swipe_mode_tag() : 0;
      if (keyball_on_multi_d) keyball_on_multi_d(tag); else tap_code16(KC_F4);
      return false;
    }

#if KEYBALL_SCROLLSNAP_ENABLE == 2
    // Scroll snap (explicit keycodes)
    case SSNP_HOR:
      keyball_set_scrollsnap_mode(KEYBALL_SCROLLSNAP_MODE_HORIZONTAL);
      kbpf.scrollsnap_mode = KEYBALL_SCROLLSNAP_MODE_HORIZONTAL;
      dprintf("SSNP: mode=HOR\n");
      return false;
    case SSNP_VRT:
      keyball_set_scrollsnap_mode(KEYBALL_SCROLLSNAP_MODE_VERTICAL);
      kbpf.scrollsnap_mode = KEYBALL_SCROLLSNAP_MODE_VERTICAL;
      dprintf("SSNP: mode=VRT\n");
      return false;
    case SSNP_FRE:
      keyball_set_scrollsnap_mode(KEYBALL_SCROLLSNAP_MODE_FREE);
      kbpf.scrollsnap_mode = KEYBALL_SCROLLSNAP_MODE_FREE;
      dprintf("SSNP: mode=FRE\n");
      return false;
#endif

    default:
      return true;
    }
    return false;
  } else {
    // release: end swipe and fallback tap if nothing fired
    switch (keycode) {
      case APP_SW:
      case VOL_SW:
      case BRO_SW:
      case TAB_SW:
      case WIN_SW:
      case SW_ARR: {
        bool fired = keyball_swipe_fired_since_begin();
        uint16_t elapsed = timer_elapsed(g_swipe_keydown_ms);
        keyball_swipe_end();
        if (!fired && elapsed < TAPPING_TERM) {
          // User-level override first
          kb_swipe_tag_t tag = 0;
          if (keycode == APP_SW) tag = KBS_TAG_APP;
          else if (keycode == VOL_SW) tag = KBS_TAG_VOL;
          else if (keycode == BRO_SW) tag = KBS_TAG_BRO;
          else if (keycode == TAB_SW) tag = KBS_TAG_TAB;
          else if (keycode == WIN_SW) tag = KBS_TAG_WIN;
          else if (keycode == SW_ARR) tag = KBS_TAG_ARR;
          if (keyball_on_swipe_tap && tag != 0) {
            keyball_on_swipe_tap(tag);
          } else {
            // Generic fallback
            uint16_t kc = KC_NO;
            if (keycode == APP_SW) kc = KC_F1;
            else if (keycode == VOL_SW) kc = KC_F2;
            else if (keycode == BRO_SW) kc = KC_F3;
            else if (keycode == TAB_SW) kc = KC_F4;
            else if (keycode == WIN_SW) kc = KC_F5;
            else if (keycode == SW_ARR) kc = KC_NO; // 明示: 何も送らない
            if (kc != KC_NO) tap_code16(kc);
          }
        }
        return false;
      }
    }
  }

  return true;
}

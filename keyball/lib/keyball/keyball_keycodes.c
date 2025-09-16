#include "keyball_keycodes.h"
#include "keyball.h"
#include "keyball_move.h"
#include "keyball_oled.h"
#include "keyball_scroll.h"
#include "os_detection.h"
#include "keyball_swipe.h"
#if defined(DYNAMIC_KEYMAP_ENABLE)
#    include "dynamic_keymap.h"
#endif

#define _CONSTRAIN(amt, low, high)                                             \
  ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// (removed) CPI adjustment helper no longer used

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

    default:
      return true;
    }
    return false;
  }

  return true;
}

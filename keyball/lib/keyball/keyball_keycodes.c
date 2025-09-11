#include "keyball.h"
#include "keyball_keycodes.h"
#include "keyball_move.h"
#include "keyball_scroll.h"
#include "keyball_swipe.h"
#include "keyball_oled.h"

#define _CONSTRAIN(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

static void add_cpi(int16_t delta) {
  int16_t v = (int16_t)keyball_get_cpi() + delta;
  if (v < 1) v = 1;
  keyball_set_cpi((uint16_t)v);
}

bool keyball_process_keycode(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case SCRL_MO:
      keyball_set_scroll_mode(record->event.pressed);
      return true;
  }

  if (record->event.pressed) {
    switch (keycode) {
      case KBC_RST:
        kbpf_defaults();
        keyball_set_cpi(kbpf.cpi[keyball_os_idx()]);
        keyball_set_scroll_div(kbpf.sdiv[keyball_os_idx()]);
        g_move_gain_lo_fp   = kbpf.mv_gain_lo_fp[keyball_os_idx()];
        g_move_th1          = kbpf.mv_th1[keyball_os_idx()];
        g_scroll_deadzone   = kbpf.sc_dz;
        g_scroll_hysteresis = kbpf.sc_hyst;
        keyball_swipe_set_step(kbpf.step);
        keyball_swipe_set_deadzone(kbpf.deadzone);
        keyball_swipe_set_reset_ms(kbpf.sw_rst_ms);
        keyball_swipe_set_freeze(kbpf.freeze);
        keyball_swipe_end();
        kbpf_write();
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
        set_auto_mouse_enable(false);
        set_auto_mouse_timeout(AUTO_MOUSE_TIME);
#endif
        break;

      case KBC_SAVE:
        kbpf.mv_gain_lo_fp[keyball_os_idx()] = (uint8_t)_CONSTRAIN(g_move_gain_lo_fp, 1, 255);
        kbpf.mv_th1[keyball_os_idx()]        = (uint8_t)_CONSTRAIN(g_move_th1, 0, kbpf.mv_th2[keyball_os_idx()] - 1);
        kbpf.sc_dz   = g_scroll_deadzone;
        kbpf.sc_hyst = g_scroll_hysteresis;
        kbpf_write();  // OSごとの全データを一括保存
        dprintf("KB profiles saved (magic=0x%08lX ver=%u)\n",
                (unsigned long)kbpf.magic, kbpf.version);
        break;

      case SCRL_DVI:
        keyball_set_scroll_div(keyball_get_scroll_div() + 1);
        break;
      case SCRL_DVD:
        keyball_set_scroll_div(keyball_get_scroll_div() - 1);
        break;

      case SCRL_INV: {
        uint8_t i = keyball_os_idx();
        kbpf.inv[i] = !kbpf.inv[i];
        dprintf("invert toggle OS=%u -> %u\n", i, kbpf.inv[i]);
      } break;

      case SCRL_TO:
        keyball_set_scroll_mode(!keyball_get_scroll_mode());
        break;

      case CPI_I100:
        add_cpi(100);
        break;
      case CPI_D100:
        add_cpi(-100);
        break;

#if KEYBALL_SCROLLSNAP_ENABLE == 2
      case SSNP_HOR:
        keyball_set_scrollsnap_mode(KEYBALL_SCROLLSNAP_MODE_HORIZONTAL);
        break;
      case SSNP_VRT:
        keyball_set_scrollsnap_mode(KEYBALL_SCROLLSNAP_MODE_VERTICAL);
        break;
      case SSNP_FRE:
        keyball_set_scrollsnap_mode(KEYBALL_SCROLLSNAP_MODE_FREE);
        break;
#endif

#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
      case AML_TO:
        set_auto_mouse_enable(!get_auto_mouse_enable());
        break;
      case AML_I50: {
        uint16_t v = get_auto_mouse_timeout() + 50;
        set_auto_mouse_timeout(MIN(v, AML_TIMEOUT_MAX));
      } break;
      case AML_D50: {
        uint16_t v = get_auto_mouse_timeout() - 50;
        set_auto_mouse_timeout(MAX(v, AML_TIMEOUT_MIN));
      } break;
#endif

      case MVGL:
        g_move_gain_lo_fp = _CONSTRAIN(g_move_gain_lo_fp + ((get_mods() & MOD_MASK_SHIFT) ? -8 : 8), 16, 255);
        kbpf.mv_gain_lo_fp[keyball_os_idx()] = (uint8_t)_CONSTRAIN(g_move_gain_lo_fp, 1, 255);
        dprintf("move: gain_lo=%ld/256\n", (long)g_move_gain_lo_fp);
        break;
      case MVTH1: {
        int8_t delta = (get_mods() & MOD_MASK_SHIFT) ? -1 : 1;
        g_move_th1 = _CONSTRAIN(g_move_th1 + delta, 0, kbpf.mv_th2[keyball_os_idx()] - 1);
        kbpf.mv_th1[keyball_os_idx()] = (uint8_t)_CONSTRAIN(g_move_th1, 0, kbpf.mv_th2[keyball_os_idx()] - 1);
        dprintf("move: th1=%d\n", g_move_th1);
        return false;
      }
      case SW_ST: {
        kb_swipe_params_t p = keyball_swipe_get_params();
        int step = p.step + ((get_mods() & MOD_MASK_SHIFT) ? -10 : 10);
        if (step < 10) step = 10;
        keyball_swipe_set_step((uint16_t)step);
        return false;
      }
      case SW_DZ: {
        kb_swipe_params_t p = keyball_swipe_get_params();
        int dz = p.deadzone + ((get_mods() & MOD_MASK_SHIFT) ? -1 : 1);
        if (dz < 0) dz = 0;
        keyball_swipe_set_deadzone((uint8_t)dz);
        return false;
      }
      case SW_FRZ:
        keyball_swipe_toggle_freeze();
        return false;
      case SW_RT: {
        kb_swipe_params_t p = keyball_swipe_get_params();
        int v = (int)p.reset_ms + ((get_mods() & MOD_MASK_SHIFT) ? -10 : 10);
        if (v < 0) v = 0;
        keyball_swipe_set_reset_ms((uint16_t)v);
        return false;
      }
      case SCRL_DZ: {
        int8_t delta = (get_mods() & MOD_MASK_SHIFT) ? -1 : 1;
        g_scroll_deadzone = _CONSTRAIN(g_scroll_deadzone + delta, 0, 32);
        kbpf.sc_dz = g_scroll_deadzone;
        dprintf("scroll: deadzone=%u\n", g_scroll_deadzone);
        return false;
      }
      case SCRL_HY: {
        int8_t delta = (get_mods() & MOD_MASK_SHIFT) ? -1 : 1;
        g_scroll_hysteresis = _CONSTRAIN(g_scroll_hysteresis + delta, 0, 32);
        kbpf.sc_hyst = g_scroll_hysteresis;
        dprintf("scroll: hyst=%u\n", g_scroll_hysteresis);
        return false;
      }
      case DBG_TOG:
        keyball_oled_mode_toggle();
        return false;
      case DBG_NP:
        keyball_oled_next_page();
        return false;
      case DBG_PP:
        keyball_oled_prev_page();
        return false;
      default:
        return true;
    }
    return false;
  }

  return true;
}

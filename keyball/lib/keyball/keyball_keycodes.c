#include "keyball_keycodes.h"
#include "keyball.h"
#include "keyball_move.h"
#include "keyball_oled.h"
#include "keyball_scroll.h"
#include "os_detection.h"
#include "keyball_swipe.h"

#define _CONSTRAIN(amt, low, high)                                             \
  ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

static void add_cpi(int16_t delta) {
  int16_t v = (int16_t)keyball_get_cpi() + delta;
  if (v < 1)
    v = 1;
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
      set_auto_mouse_enable(false);
      set_auto_mouse_timeout(AUTO_MOUSE_TIME);
#endif
      break;

    case KBC_SAVE:
      kbpf.move_gain_lo_fp[keyball_os_idx()] =
          (uint8_t)_CONSTRAIN(g_move_gain_lo_fp, 1, 255);
      kbpf.move_th1[keyball_os_idx()] =
          (uint8_t)_CONSTRAIN(g_move_th1, 0, kbpf.move_th2[keyball_os_idx()] - 1);
      kbpf_write(); // OSごとの全データを一括保存
      dprintf("KB profiles saved (magic=0x%08lX ver=%u)\n",
              (unsigned long)kbpf.magic, kbpf.version);
      break;

    // Pointer settings
    case CPI_I100:
      add_cpi(100);
      break;
    case CPI_D100:
      add_cpi(-100);
      break;
    case MVGL:
      g_move_gain_lo_fp = _CONSTRAIN(
          g_move_gain_lo_fp + ((get_mods() & MOD_MASK_SHIFT) ? -8 : 8), 16,
          255);
      kbpf.move_gain_lo_fp[keyball_os_idx()] =
          (uint8_t)_CONSTRAIN(g_move_gain_lo_fp, 1, 255);
      dprintf("move: gain_lo=%ld/256\n", (long)g_move_gain_lo_fp);
      break;
    case MVTH1: {
      int8_t delta = (get_mods() & MOD_MASK_SHIFT) ? -1 : 1;
      g_move_th1 =
          _CONSTRAIN(g_move_th1 + delta, 0, kbpf.move_th2[keyball_os_idx()] - 1);
      kbpf.move_th1[keyball_os_idx()] =
          (uint8_t)_CONSTRAIN(g_move_th1, 0, kbpf.move_th2[keyball_os_idx()] - 1);
      dprintf("move: th1=%d\n", g_move_th1);
      return false;
    }

    // Scroll control
    case SCRL_TO:
      keyball_set_scroll_mode(!keyball_get_scroll_mode());
      break;
    case SCRL_STI: {
      uint8_t v = keyball_get_scroll_div();
      keyball_set_scroll_div(v + 1); // 上限は内部でクランプ
    } break;
    case SCRL_STD: {
      uint8_t v = keyball_get_scroll_div();
      if (v > 0) keyball_set_scroll_div(v - 1); // 下限未満での回り込み防止
    } break;
    case SCRL_PST: { // プリセット切替
      uint8_t os = keyball_os_idx();
      uint8_t host = (uint8_t)detected_host_os();
      if (host == OS_MACOS) {
        // macOS は固定: {120,120}
        kbpf.scroll_interval[os] = 120;
        kbpf.scroll_value[os]    = 120;
        kbpf.scroll_preset[os]   = 2; // MAC
        dprintf("scroll preset(OS=%u): mac {120,120}\n", os);
      } else {
        // それ以外: {120,1} <-> {1,1}
        if (kbpf.scroll_preset[os] == 1) {
          kbpf.scroll_preset[os] = 0;
          kbpf.scroll_interval[os] = 120;
          kbpf.scroll_value[os]    = 1;
          dprintf("scroll preset(OS=%u): {120,1}\n", os);
        } else {
          kbpf.scroll_preset[os] = 1;
          kbpf.scroll_interval[os] = 1;
          kbpf.scroll_value[os]    = 1;
          dprintf("scroll preset(OS=%u): {1,1}\n", os);
        }
      }
      return false;
    }
    case SCRL_INV: {
      uint8_t i = keyball_os_idx();
      kbpf.scroll_invert[i] = !kbpf.scroll_invert[i];
      dprintf("invert toggle OS=%u -> %u\n", i, kbpf.scroll_invert[i]);
    } break;
    // SCRL_DZ / SCRL_HY は廃止

#if KEYBALL_SCROLLSNAP_ENABLE == 2
    // Scroll snap
    case SSNP_HOR:
      keyball_set_scrollsnap_mode(KEYBALL_SCROLLSNAP_MODE_HORIZONTAL);
      dprintf("SSNP: mode=HOR\n");
      break;
    case SSNP_VRT:
      keyball_set_scrollsnap_mode(KEYBALL_SCROLLSNAP_MODE_VERTICAL);
      dprintf("SSNP: mode=VRT\n");
      break;
    case SSNP_FRE:
      keyball_set_scrollsnap_mode(KEYBALL_SCROLLSNAP_MODE_FREE);
      dprintf("SSNP: mode=FRE\n");
      break;
#endif

#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
    // Automatic mouse layer
    case AML_TO:
      set_auto_mouse_enable(!get_auto_mouse_enable());
      dprintf("AML: enable=%u layer=%u timeout=%u\n",
              get_auto_mouse_enable() ? 1u : 0u,
              (unsigned)get_auto_mouse_layer(),
              (unsigned)get_auto_mouse_timeout());
      break;
    case AML_I50: {
      uint16_t v = get_auto_mouse_timeout() + 50;
      if (v > 1000) v = 1000;
      set_auto_mouse_timeout(v);
      dprintf("AML: timeout=%u\n", (unsigned)get_auto_mouse_timeout());
    } break;
    case AML_D50: {
      uint16_t v = get_auto_mouse_timeout();
      v = (v > 50) ? (uint16_t)(v - 50) : 0;
      if (v < 100) v = 100;
      set_auto_mouse_timeout(v);
      dprintf("AML: timeout=%u\n", (unsigned)get_auto_mouse_timeout());
    } break;
#endif

    // Swipe control
    case SW_RT: {
      kb_swipe_params_t p = keyball_swipe_get_params();
      int v = (int)p.reset_ms + ((get_mods() & MOD_MASK_SHIFT) ? -10 : 10);
      if (v < 0)
        v = 0;
      keyball_swipe_set_reset_ms((uint16_t)v);
      return false;
    }
    case SW_ST: {
      kb_swipe_params_t p = keyball_swipe_get_params();
      int step = p.step + ((get_mods() & MOD_MASK_SHIFT) ? -10 : 10);
      if (step < 10)
        step = 10;
      keyball_swipe_set_step((uint16_t)step);
      return false;
    }
    case SW_DZ: {
      kb_swipe_params_t p = keyball_swipe_get_params();
      int dz = p.deadzone + ((get_mods() & MOD_MASK_SHIFT) ? -1 : 1);
      if (dz < 0)
        dz = 0;
      keyball_swipe_set_deadzone((uint8_t)dz);
      return false;
    }
    case SW_FRZ:
      keyball_swipe_toggle_freeze();
      return false;

    // Debug
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

#include "keyball_keycodes.h"
#include "keyball.h"
#include "keyball_move.h"
#include "keyball_oled.h"
#include "keyball_scroll.h"
#include "os_detection.h"
#include "keyball_swipe.h"
#include "keyball_multi.h"
#include "timer.h"
#ifdef HAPTIC_ENABLE
#    include "haptic.h"
#endif
#if defined(DYNAMIC_KEYMAP_ENABLE)
#    include "dynamic_keymap.h"
#endif

#define _CONSTRAIN(amt, low, high)                                             \
  ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// (removed) CPI adjustment helper no longer used

static uint16_t g_swipe_keydown_ms = 0;

// --- KB-level default helpers (used when user override is absent) ---

// multi key Aの動作
static void kb_default_multi_a(kb_swipe_tag_t tag) {
  // tagなしの動作
  if (tag == 0) {
    // Undo: Win/Linux=Ctrl+Z, mac/iOS=GUI+Z
    tap_code16_os(C(KC_Z), G(KC_Z), G(KC_Z), C(KC_Z), C(KC_Z));
    return;
  }
  // tagありの時の動作
  switch (tag) {
    case KBS_TAG_APP:
      // Desktop move left (Win: Win+Ctrl+Left, Mac: Ctrl+Left)
      tap_code16_os(G(C(KC_LEFT)), LCTL(KC_LEFT), LCTL(KC_LEFT), KC_NO, KC_NO);
      break;
    case KBS_TAG_TAB:
      tap_code16(S(C(KC_TAB))); // prev tab
      break;
    case KBS_TAG_BRO:
      // Browser back (Win), mac: GUI+Left
      tap_code16_os(KC_WBAK, G(KC_LEFT), G(KC_LEFT), KC_NO, KC_NO);
      break;
    case KBS_TAG_VOL:
      tap_code(KC_MNXT);
      break;
    case KBS_TAG_WIN:
      if (detected_host_os() == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_LEFT); }
      else                                  { tap_code16(C(A(KC_LEFT))); }
      break;
    default:
      tap_code16(KC_F1);
      break;
  }
}

static void kb_default_multi_b(kb_swipe_tag_t tag) {
  if (tag == 0) {
    // Redo: Win/Linux=Ctrl+Y, mac/iOS=GUI+Shift+Z
    tap_code16_os(C(KC_Y), S(G(KC_Z)), S(G(KC_Z)), C(KC_Y), C(KC_Y));
    return;
  }
  switch (tag) {
    case KBS_TAG_APP:
      // Desktop move right (Win: Win+Ctrl+Right, Mac: Ctrl+Right)
      tap_code16_os(G(C(KC_RIGHT)), LCTL(KC_RIGHT), LCTL(KC_RIGHT), KC_NO, KC_NO);
      break;
    case KBS_TAG_TAB:
      tap_code16(C(KC_TAB)); // next tab
      break;
    case KBS_TAG_BRO:
      // Browser forward (Win), mac: GUI+Right
      tap_code16_os(KC_WFWD, G(KC_RIGHT), G(KC_RIGHT), KC_NO, KC_NO);
      break;
    case KBS_TAG_VOL:
      tap_code(KC_MPRV);
      break;
    case KBS_TAG_WIN:
      if (detected_host_os() == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_RIGHT); }
      else                                  { tap_code16(C(A(KC_RIGHT))); }
      break;
    default:
      tap_code16(KC_F2);
      break;
  }
}

static void kb_default_multi_c(kb_swipe_tag_t tag) {
  if (tag == 0) { tap_code16(KC_F3); return; }
  switch (tag) {
    case KBS_TAG_APP:
      // Task view / Mission Control
      tap_code16_os(C(KC_TAB), LCTL(KC_UP), LCTL(KC_UP), KC_NO, KC_NO);
      break;
    case KBS_TAG_TAB:
      // Last tab
      tap_code16_os(S(C(KC_T)), S(G(KC_T)), S(G(KC_T)), KC_NO, KC_NO);
      break;
    case KBS_TAG_BRO:
      // Reload
      tap_code16_os(C(KC_R), G(KC_R), G(KC_R), KC_NO, KC_NO);
      break;
    case KBS_TAG_VOL:
      tap_code(KC_VOLU);
      break;
    case KBS_TAG_WIN:
      if (detected_host_os() == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_UP); }
      else                                  { tap_code16(C(A(KC_UP))); }
      break;
    default:
      tap_code16(KC_F3);
      break;
  }
}

static void kb_default_multi_d(kb_swipe_tag_t tag) {
  if (tag == 0) { tap_code16(KC_F4); return; }
  switch (tag) {
    case KBS_TAG_APP:
      // Show desktop / App Expose
      tap_code16_os(G(KC_D), LCTL(KC_DOWN), LCTL(KC_DOWN), KC_NO, KC_NO);
      break;
    case KBS_TAG_TAB:
      // Close tab
      tap_code16_os(C(KC_W), G(KC_W), G(KC_W), KC_NO, KC_NO);
      break;
    case KBS_TAG_BRO:
      // New tab
      tap_code16_os(C(KC_T), G(KC_T), G(KC_T), KC_NO, KC_NO);
      break;
    case KBS_TAG_VOL:
      tap_code(KC_VOLD);
      break;
    case KBS_TAG_WIN:
      if (detected_host_os() == OS_WINDOWS) { register_code(KC_LGUI); tap_code(KC_DOWN); }
      else                                  { tap_code16(C(A(KC_DOWN))); }
      break;
    default:
      tap_code16(KC_F4);
      break;
  }
}

// kbレベルのカスタムキー動作
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
      g_move_th2 = kbpf.move_th2[keyball_os_idx()];
      keyball_swipe_set_step(kbpf.swipe_step);
      keyball_swipe_set_deadzone(kbpf.swipe_deadzone);
      keyball_swipe_set_reset_ms(kbpf.swipe_reset_ms);
      keyball_swipe_set_freeze(kbpf.swipe_freeze);
      keyball_swipe_end();
#ifdef HAPTIC_ENABLE
      haptic_set_mode(kbpf.swipe_haptic_mode);
      keyball_swipe_haptic_reset_sequence();
#endif
      kbpf_write();
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
      set_auto_mouse_enable(kbpf.aml_enable ? true : false);
      set_auto_mouse_timeout(kbpf.aml_timeout);
      if (kbpf.aml_layer != 0xFFu) set_auto_mouse_layer(kbpf.aml_layer);
#endif
      keyball_refresh_scroll_layer();
      break;

    case KBC_SAVE:
      kbpf.move_gain_lo_fp[keyball_os_idx()] =
          (uint8_t)_CONSTRAIN(g_move_gain_lo_fp, 1, 255);
      // Save move th2 first so th1 can clamp against it
      kbpf.move_th2[keyball_os_idx()] = (uint8_t)_CONSTRAIN(g_move_th2, 1, 63);
      if (kbpf.move_th1[keyball_os_idx()] >= kbpf.move_th2[keyball_os_idx()]) {
        kbpf.move_th1[keyball_os_idx()] = kbpf.move_th2[keyball_os_idx()] - 1;
        g_move_th1 = kbpf.move_th1[keyball_os_idx()];
      } else {
        kbpf.move_th1[keyball_os_idx()] =
            (uint8_t)_CONSTRAIN(g_move_th1, 0, kbpf.move_th2[keyball_os_idx()] - 1);
      }
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
      kbpf.aml_enable  = get_auto_mouse_enable() ? 1 : 0;
      kbpf.aml_timeout = get_auto_mouse_timeout();
      kbpf.aml_layer   = get_auto_mouse_layer();
#endif
#if KEYBALL_SCROLLSNAP_ENABLE == 2
      kbpf.scrollsnap_mode = (uint8_t)keyball_get_scrollsnap_mode();
#endif
      // RGB ライトの現在状態をEEPROMへ永続化（on/off, HSV, mode）
#ifdef RGBLIGHT_ENABLE
      if (rgblight_is_enabled()) {
        rgblight_enable();  // フラグをEEPROMに保存
      } else {
        rgblight_disable(); // 無効もEEPROMに保存
      }
      // 現在の色とモードも保存（noeepromでないAPIを使う）
      rgblight_sethsv(rgblight_get_hue(), rgblight_get_sat(), rgblight_get_val());
      rgblight_mode(rgblight_get_mode());
#endif
#ifdef HAPTIC_ENABLE
      kbpf.swipe_haptic_mode = haptic_get_mode();
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

    // Speed adjustments -------------------------------------------------
    case SCSP_INC: {
      uint8_t v = keyball_get_scroll_div();
      if (v < 7) v++;
      keyball_set_scroll_div(v);
      return false;
    }
    case SCSP_DEC: {
      uint8_t v = keyball_get_scroll_div();
      if (v > 1) v--;
      keyball_set_scroll_div(v);
      return false;
    }
    case MOSP_INC: {
      uint16_t cpi = keyball_get_cpi();
      if (cpi < 12000) cpi += 100; // guard upper
      keyball_set_cpi(cpi);
      return false;
    }
    case MOSP_DEC: {
      uint16_t cpi = keyball_get_cpi();
      if (cpi > 100) cpi -= 100; // guard lower
      keyball_set_cpi(cpi);
      return false;
    }
    // Setting view controls
    case STG_TOG:
      keyball_oled_mode_toggle();
      return false;

    // Swipe action keys: begin on press
      // g_swipe_keydown_msはタップ動作判定のため
      // swipe_beginでスワイプ状態タグ付け
      // タグさえ付けてしまえば、他のキーでも同様の動作可能
    case APP_SW:  g_swipe_keydown_ms = timer_read(); keyball_swipe_begin(KBS_TAG_APP);  return false;
    case VOL_SW:  g_swipe_keydown_ms = timer_read(); keyball_swipe_begin(KBS_TAG_VOL);  return false;
    case BRO_SW:  g_swipe_keydown_ms = timer_read(); keyball_swipe_begin(KBS_TAG_BRO);  return false;
    case TAB_SW:  g_swipe_keydown_ms = timer_read(); keyball_swipe_begin(KBS_TAG_TAB);  return false;
    case WIN_SW:  g_swipe_keydown_ms = timer_read(); keyball_swipe_begin(KBS_TAG_WIN);  return false;
    case UTIL_SW: g_swipe_keydown_ms = timer_read(); keyball_swipe_begin(KBS_TAG_UTIL); return false;
    case ARR_SW:  g_swipe_keydown_ms = timer_read(); keyball_swipe_begin(KBS_TAG_ARR);  return false;
    case SW_EX1: g_swipe_keydown_ms = timer_read(); keyball_swipe_begin(KBS_TAG_EX1); return false; // Extension swipe 1
    case SW_EX2: g_swipe_keydown_ms = timer_read(); keyball_swipe_begin(KBS_TAG_EX2); return false; // Extension swipe 2

    // Arrow keys as swipe-direction proxies while swipe mode held
    // ボールの操作ではなく、アローキーを使用してもスワイプ動作の各方向が発火
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
      // マルチキーはスワイプタグごとに動作が変わるキー
    case MULTI_A: {
      kb_swipe_tag_t tag = keyball_swipe_is_active() ? keyball_swipe_mode_tag() : 0;
      if (keyball_on_multi_a) keyball_on_multi_a(tag); else kb_default_multi_a(tag);
      return false;
    }
    case MULTI_B: {
      kb_swipe_tag_t tag = keyball_swipe_is_active() ? keyball_swipe_mode_tag() : 0;
      if (keyball_on_multi_b) keyball_on_multi_b(tag); else kb_default_multi_b(tag);
      return false;
    }
    case MULTI_C: {
      kb_swipe_tag_t tag = keyball_swipe_is_active() ? keyball_swipe_mode_tag() : 0;
      if (keyball_on_multi_c) keyball_on_multi_c(tag); else kb_default_multi_c(tag);
      return false;
    }
    case MULTI_D: {
      kb_swipe_tag_t tag = keyball_swipe_is_active() ? keyball_swipe_mode_tag() : 0;
      if (keyball_on_multi_d) keyball_on_multi_d(tag); else kb_default_multi_d(tag);
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
      case UTIL_SW:
      case ARR_SW: {
        bool fired = keyball_swipe_fired_since_begin();
        uint16_t elapsed = timer_elapsed(g_swipe_keydown_ms);
        keyball_swipe_end();
        if (!fired && elapsed < TAPPING_TERM) {
          // User-level override first
          kb_swipe_tag_t tag = 0;
          if      (keycode == APP_SW) tag = KBS_TAG_APP;
          else if (keycode == VOL_SW) tag = KBS_TAG_VOL;
          else if (keycode == BRO_SW) tag = KBS_TAG_BRO;
          else if (keycode == TAB_SW) tag = KBS_TAG_TAB;
          else if (keycode == WIN_SW) tag = KBS_TAG_WIN;
          else if (keycode == UTIL_SW) tag = KBS_TAG_UTIL;
          else if (keycode == ARR_SW)  tag = KBS_TAG_ARR;
          else if (keycode == SW_EX1) tag = KBS_TAG_EX1;
          else if (keycode == SW_EX2) tag = KBS_TAG_EX2;

          // Generic fallback
          // void keyball_on_swipe_tap(kb_swipe_tag_t tag) を使用してuserレベルで上書き可能
          // 参考）lib_user/swipe_user.c
          if (keyball_on_swipe_tap && tag != 0) {
            keyball_on_swipe_tap(tag);
          } else {
            if      (keycode == APP_SW)  tap_code16_os(LGUI(KC_TAB), LCTL(KC_UP), LCTL(KC_UP), KC_NO, KC_NO);
            else if (keycode == VOL_SW)  tap_code(KC_MPLY);
            else if (keycode == BRO_SW)  tap_code16_os(LCTL(KC_R), LGUI(KC_R), LGUI(KC_R), LCTL(KC_R), KC_NO);
            else if (keycode == TAB_SW)  tap_code16_os(LCTL(KC_T), LGUI(KC_T), LGUI(KC_T), LCTL(KC_T), KC_NO);
            else if (keycode == WIN_SW)  tap_code16_os(LGUI(KC_UP), LCTL(RCTL(KC_F)), LCTL(RCTL(KC_F)), KC_NO, KC_NO);
            else if (keycode == UTIL_SW) { tap_code16(KC_ESC); tap_code16(KC_LNG2); }
            else if (keycode == ARR_SW)  tap_code(KC_NO);
            else if (keycode == SW_EX1)  tap_code(KC_F13);
            else if (keycode == SW_EX2)  tap_code(KC_F18);
          }
        }
        return false;
      }
    }
  }

  return true;
}

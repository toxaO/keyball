/*
Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H

#include "quantum.h"
#include "lib/keyball/keyball.h"
#include "features/translate_ansi_to_jis.h"
#include "features/select_word.h"

enum custom_keycodes {
  MY_MACRO_0 = SAFE_RANGE,  // 0x7E40
  MY_MACRO_1,  // 0x7E41
  MY_MACRO_2,  // 0x7E42
  MY_MACRO_3,  // 0x7E43
  MY_MACRO_4,  // 0x7E44
  MY_MACRO_5,  // 0x7E45
  A2J_TOGG,    // 0x7E46
  SELWORD,     // 0x7E47
  ARROW,       // 0x7E48
  SMTD_KEYCODES_BEGIN,  // 0x7E49
  CKC_ESC,  // 0x7E4A
  CKC_A,    // 0x7E4B
  CKC_S,    // 0x7E4C
  CKC_D,    // 0x7E4D
  CKC_F,    // 0x7E4E
  CKC_J,    // 0x7E4F
  CKC_K,    // 0x7E50
  CKC_L,    // 0x7E51
  CKC_SCLN, // 0x7E52
  CKC_SLSH, // 0x7E53
  CKC_QUOT, // 0x7E54
  CKC_LNG2, // 0x7E55
  CKC_SPC,  // 0x7E56
  CKC_LNG1, // 0x7E57
  CKC_ENT,  // 0x7E58
  SMTD_KEYCODES_END, // 0x7E59
  MY_USER_0 = KEYBALL_SAFE_RANGE + 32,  // 0x7E60
  M_UPDIR,
};

// #include "features/sm_td.h"

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  // keymap for default (VIA)
  [0] = LAYOUT_universal(
    KC_ESC        , KC_Q  , KC_W    , KC_E     , KC_R     , KC_T     ,                                       KC_Y     , KC_U          , KC_I     , KC_O     , KC_P     , KC_BSPC   ,
    LCTL_T(KC_ESC), LGUI_T(KC_A), LALT_T(KC_S), LSFT_T(KC_D) , LCTL_T(KC_F) , KC_G ,                         KC_H     , LCTL_T(KC_J)  , RSFT_T(KC_K) , LALT_T(KC_L) , LT(1,KC_SCLN) , KC_MINUS ,
    LSFT_T(KC_LSFT), KC_Z , KC_X    , KC_C     , KC_V     , KC_B     ,                                       KC_N     , KC_M          , KC_COMM  , KC_DOT   , LT(3,KC_SLSH), LT(3,KC_QUOT) ,
                  KC_LALT , KC_TAB  , LT(2,KC_LNG2)   , LT(3,KC_SPC) , LT(1,KC_LNG1) ,            QK_REP   , LT(2,KC_ENT) , XXXXXXX   , XXXXXXX  , A2J_TOGG
  ),

  [1] = LAYOUT_universal(
    _______  ,  KC_PGUP , MY_MACRO_0, KC_END  , MY_MACRO_2, MY_MACRO_5 ,                                     SELWORD  , KC_HOME  , KC_UP    , KC_END   , XXXXXXX  , XXXXXXX  ,
    _______  ,  KC_HOME , MY_MACRO_1, KC_PGDN , MY_MACRO_3, MY_MACRO_4 ,                                     KC_BSPC  , KC_LEFT  , KC_DOWN  , KC_RGHT  , XXXXXXX  , XXXXXXX  ,
    _______  ,  XXXXXXX , XXXXXXX   , XXXXXXX , XXXXXXX   , ARROW ,                                          KC_DEL   , KC_BTN1  , KC_BTN3  , KC_BTN2  , KC_BTN4  , KC_BTN5  ,
                  _______  , _______ , _______  ,         _______  , _______  ,                   _______  , _______  , _______       , _______  , KC_MUTE
  ),

  [2] = LAYOUT_universal(
    _______  , KC_F6    , KC_F7    , KC_F8    , KC_F9    , KC_F10  ,                                          S(KC_8)  , KC_7    , KC_8     , KC_9     , KC_SLSH  , KC_BSPC  ,
    _______  , KC_PSCR  , KC_F2    , KC_F3    , KC_F4    , KC_F5   ,                                          S(KC_EQL), KC_4    , KC_5     , KC_6     , KC_MINUS , XXXXXXX  ,
    _______  , _______  , _______  , _______  , KC_F11   , KC_F12  ,                                          KC_0     , KC_1    , KC_2     , KC_3     , KC_DOT   , XXXXXXX  ,
                  _______  , _______ , _______  ,         _______  , _______  ,                   QK_AREP  , _______  , _______       , _______  , S(KC_SCLN)
  ),

  [3] = LAYOUT_universal(
    _______  ,S(KC_3)   , S(KC_COMM)  , S(KC_DOT) , KC_SCLN    , KC_GRV  ,                                   S(KC_7)    , KC_LBRC    , KC_RBRC    , S(KC_MINS) , S(KC_5)    , _______ ,
    _______  ,S(KC_1)   , KC_MINS     , S(KC_EQL) , KC_EQL     , S(KC_QUOT) ,                                S(KC_BSLS) , S(KC_9)    , S(KC_0)    , S(KC_SCLN) , S(KC_SLSH) , KC_DOT ,
    _______  ,S(KC_6)   , KC_SLSH     , S(KC_8)   , KC_BSLS    , KC_QUOT ,                                   S(KC_GRV)  , S(KC_LBRC) , S(KC_RBRC) , S(KC_4)    , S(KC_2)    , KC_COMM ,
                  _______  , _______  , QK_BOOT  ,         _______  , KBC_SAVE ,                  QK_BOOT  , _______  , _______       , _______  , A2J_TOGG
  ),
};
// clang-format on

layer_state_t layer_state_set_user(layer_state_t state) {
    // Auto enable scroll mode when the highest layer is 3
    keyball_set_scroll_mode(get_highest_layer(state) == 3);
    return state;
}

#ifdef OLED_ENABLE

#include "lib/oledkit/oledkit.h"

// [CUSTOM]
static const char LFSTR_ON[] PROGMEM = "\xB2\xB3";
static const char LFSTR_OFF[] PROGMEM = "\xB4\xB5";
static bool jis_mode = false;

bool is_jis_mode(void) {
  return jis_mode;
}

void set_jis_mode(bool is_jis_mode) {
  jis_mode = is_jis_mode;
}

static const char *format_4d(int8_t d) {
    static char buf[5] = {0}; // max width (4) + NUL (1)
    char        lead   = ' ';
    if (d < 0) {
        d    = -d;
        lead = '-';
    }
    buf[3] = (d % 10) + '0';
    d /= 10;
    if (d == 0) {
        buf[2] = lead;
        lead   = ' ';
    } else {
        buf[2] = (d % 10) + '0';
        d /= 10;
    }
    if (d == 0) {
        buf[1] = lead;
        lead   = ' ';
    } else {
        buf[1] = (d % 10) + '0';
        d /= 10;
    }
    buf[0] = lead;
    return buf;
}

static char to_1x(uint8_t x) {
    x &= 0x0f;
    return x < 10 ? x + '0' : x + 'a' - 10;
}

void keyball_oled_render_keyinfo_custom(void) {
    // Format: `Key :  R{row}  C{col} K{kc} CW on/off`
    //
    // Where `kc` is 16 bit of keycode.
    //
    // `row`, `col`, and `kc` indicates the last processed key,
    // but `name`s indicate unreleased keys in best effort.
    //
    // It is aligned to fit with output of keyball_oled_render_ballinfo().
    // For example:
    //
    //     Key :  R2  C3 K06 CW on/off
    //     Ball:   0   0   0   0

    // "Key" Label
    oled_write_P(PSTR("Key \xB1"), false);

    // Row and column
    oled_write_char('\xB8', false);
    oled_write_char(to_1x(keyball.last_pos.row), false);
    oled_write_char('\xB9', false);
    oled_write_char(to_1x(keyball.last_pos.col), false);

    // Keycode
    oled_write_P(PSTR("\xBA\xBB:"), false);
    oled_write_char(to_1x(keyball.last_kc >> 12), false);
    oled_write_char(to_1x(keyball.last_kc >> 8), false);
    oled_write_char(to_1x(keyball.last_kc >> 4), false);
    oled_write_char(to_1x(keyball.last_kc), false);

    // indicate jis mode: on/off
    oled_write_P(PSTR(" JP"), false);
    if (is_jis_mode()) {
        oled_write_P(LFSTR_ON, false);
    } else {
        oled_write_P(LFSTR_OFF, false);
    }
}

void keyball_oled_render_ballinfo_custom(void) {
    // Format: `Ball:{mouse x}{mouse y}{mouse h}{mouse v}`
    //
    // Output example:
    //
    //     Ball: -12  34   0   0

    // 1st line, "Ball" label, mouse x, y, h, and v.
    oled_write_P(PSTR("Ball\xB1"), false);
    oled_write(format_4d(keyball.last_mouse.x), false);
    oled_write(format_4d(keyball.last_mouse.y), false);
//    oled_write(format_4d(keyball.last_mouse.h), false);
//    oled_write(format_4d(keyball.last_mouse.v), false);

    // indicate Caps Word mode: on/off
    oled_write_P(PSTR("    CW"), false);
    if (is_caps_word_on()) {
        oled_write_P(LFSTR_ON, false);
    } else {
        oled_write_P(LFSTR_OFF, false);
    }

    // 2nd line, empty label and CPI
    oled_write_P(PSTR("    \xB1\xBC\xBD"), false);
    oled_write(format_4d(keyball_get_cpi()) + 1, false);
    oled_write_P(PSTR("00 "), false);

    // indicate scroll snap mode: "VT" (vertical), "HN" (horiozntal), and "SCR" (free)
#if 1 && KEYBALL_SCROLLSNAP_ENABLE == 2
    switch (keyball_get_scrollsnap_mode()) {
        case KEYBALL_SCROLLSNAP_MODE_VERTICAL:
            oled_write_P(PSTR("VT"), false);
            break;
        case KEYBALL_SCROLLSNAP_MODE_HORIZONTAL:
            oled_write_P(PSTR("HO"), false);
            break;
        default:
            oled_write_P(PSTR("\xBE\xBF"), false);
            break;
    }
#else
    oled_write_P(PSTR("\xBE\xBF"), false);
#endif
    // indicate scroll mode: on/off
    if (keyball.scroll_mode) {
        oled_write_P(LFSTR_ON, false);
    } else {
        oled_write_P(LFSTR_OFF, false);
    }

    // indicate scroll divider:
    oled_write_P(PSTR(" \xC0\xC1"), false);
    oled_write_char('0' + keyball_get_scroll_div(), false);
}

#ifdef DYNAMIC_MACRO_ENABLE
// Dynamic Macros: Record and Replay Macros in Runtime
// https://github.com/qmk/qmk_firmware/blob/master/docs/feature_dynamic_macros.md

bool isRecording = false;
// Triggered when you start recording a macro.
void dynamic_macro_record_start_user(int8_t direction) {
    isRecording = true;
}

// Triggered when you play back a macro.
void dynamic_macro_play_user(int8_t direction) {
}

// Triggered on each keypress while recording a macro.
void dynamic_macro_record_key_user(int8_t direction, keyrecord_t *record) {
}

// Triggered when the macro recording is stopped.
void dynamic_macro_record_end_user(int8_t direction) {
    isRecording = false;
}
#endif

void oledkit_render_info_user(void) {
//    keyball_oled_render_keyinfo();
    keyball_oled_render_keyinfo_custom();
//    keyball_oled_render_ballinfo();
    keyball_oled_render_ballinfo_custom();
    keyball_oled_render_layerinfo();
// #ifdef DYNAMIC_MACRO_ENABLE
//     oled_write_P(PSTR("\n"), false);
//     oled_write_P(isRecording ? PSTR("[REC]") : PSTR("[   ]"), isRecording);
// #endif
}
#endif

// [CUSTOM]
static uint16_t registered_key = KC_NO;

uint16_t get_alt_repeat_key_keycode_user(uint16_t keycode, uint8_t mods) {
    bool shifted = (mods & MOD_MASK_SHIFT);  // Was Shift held?
    switch (keycode) {
      // Same-Finger Bigrams
      case KC_E: return KC_D;  // For "ED" bigram.
      case KC_D: return KC_E;  // For "DE" bigram.
      case KC_C: return KC_E;  // For "CE" bigram.
      case KC_L: return KC_O;  // For "LO" bigram.
      case KC_U: return KC_N;  // For "UN" bigram.
      case KC_M: return KC_U;  // For "MU" bigram.
      case KC_J: return KC_U;  // For "JU" bigram.
      case KC_N: return KC_U;  // For "NU" bigram.
      case KC_Y: return KC_U;  // For "YU" bigram.
      case KC_H: return KC_U;  // For "HU" bigram.
      case KC_K: return KC_I;  // For "KI" bigram.
      case KC_R: return KC_T;  // For "RT" bigram.
      case KC_F: return KC_T;  // For "FT" bigram.
      case KC_V: return KC_T;  // For "VT" bigram.
      case KC_B: return KC_R;  // For "BR" bigram.
      case KC_G: return KC_R;  // For "GR" bigram.
      case KC_S: return KC_W;  // For "SW" bigram.
      case KC_TAB:
        if (shifted) {        // If the last key was Shift + Tab,
            return KC_TAB;    // ... the reverse is Tab.
        } else {              // Otherwise, the last key was Tab,
            return S(KC_TAB); // ... and the reverse is Shift + Tab.
        }
        break;

      case KC_DOT: 
        return M_UPDIR;
        break;
    }

    return KC_TRNS;
}

void set_disable_ime(void) {
  tap_code16(KC_LNG2);
}

/*
void on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        SMTD_MT(CKC_ESC, KC_ESC, KC_LEFT_CTRL)
        SMTD_MT(CKC_A, KC_A, KC_LEFT_GUI)
        SMTD_MT(CKC_S, KC_S, KC_LEFT_ALT)
        SMTD_MT(CKC_D, KC_D, KC_LSFT)
        SMTD_MT(CKC_F, KC_F, KC_LEFT_CTRL)
        SMTD_MT(CKC_J, KC_J, KC_LEFT_CTRL)
        SMTD_MT(CKC_K, KC_K, KC_RSFT)
        SMTD_MT(CKC_L, KC_L, KC_LEFT_ALT)

        SMTD_LT(CKC_SCLN, KC_SCLN, 1)
        SMTD_LT(CKC_SLSH, KC_SLSH, 3)
        SMTD_LT(CKC_QUOT, KC_QUOT, 3)
        SMTD_LT(CKC_LNG2, KC_LNG2, 2)
        SMTD_LT(CKC_SPC, KC_SPC, 3)
        SMTD_LT(CKC_LNG1, KC_LNG1, 1)
        SMTD_LT(CKC_ENT, KC_ENT, 2) 
    }
}
*/

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
//   if (!process_smtd(keycode, record)) {
//       return false;
//   }

  if (record->event.pressed) {
    static uint32_t last_key_pressed = 0;
    uint32_t now = timer_read32();
    if (TIMER_DIFF_32(now, last_key_pressed) > AUTO_DISABLE_IME_TIME) {
      set_disable_ime();
    }

    last_key_pressed = now;
  }

  if (!process_select_word(keycode, record, SELWORD)) { return false; }

  const uint8_t mods = get_mods();
#ifndef NO_ACTION_ONESHOT
  uint8_t shift_mods = (mods | get_oneshot_mods()) & MOD_MASK_SHIFT;
#else
  uint8_t shift_mods = mods & MOD_MASK_SHIFT;
#endif  // NO_ACTION_ONESHOT

  switch (keycode) {
    case MY_MACRO_0:
      if (record->event.pressed) {
        // WIN + SPACE (JIS ⇔ US配列切替)
        SEND_STRING(SS_LGUI(SS_TAP(X_SPACE)));
      }
      return false;
      break;
    case MY_MACRO_1:
      if (record->event.pressed) {
        // CTRL + HOME (先頭へ移動)
        SEND_STRING(SS_LCTL(SS_TAP(X_HOME)));
      }
      return false;
      break;
    case MY_MACRO_2:
      if (record->event.pressed) {
        // CTRL + END (最後へ移動)
        SEND_STRING(SS_LCTL(SS_TAP(X_END)));
      }
      return false;
      break;
    case MY_MACRO_3:
      if (record->event.pressed) {
        // CTRL + F3 (カーソル位置の単語で次へ検索)
        SEND_STRING(SS_LCTL(SS_TAP(X_F3)));
      }
      return false;
      break;
    case MY_MACRO_4:
      if (record->event.pressed) {
        // ALT + F5 (VSCode: 差分を次へ検索)
        SEND_STRING(SS_LALT(SS_TAP(X_F5)));
      }
      return false;
      break;
    case MY_MACRO_5:
      if (record->event.pressed) {
        // click して行全体をコピー
        SEND_STRING(SS_TAP(X_BTN1) SS_TAP(X_HOME) SS_TAP(X_HOME) SS_LSFT(SS_TAP(X_END)) SS_LCTL("c"));
      }
      return false;
      break;

    case ARROW:  // Arrow macro, types -> or =>.
      if (record->event.pressed) {
        if (shift_mods) {
          if (shift_mods != MOD_MASK_SHIFT) {
#ifndef NO_ACTION_ONESHOT
            del_oneshot_mods(MOD_MASK_SHIFT);
#endif  // NO_ACTION_ONESHOT
            unregister_mods(MOD_MASK_SHIFT);
          }
          SEND_STRING("=>");
          register_mods(mods);            // Restore mods.
        } else {
          SEND_STRING("->");
        }
      }
//      return false;
      break;

    case M_UPDIR:
      if (record->event.pressed) {
        SEND_STRING(/*.*/"./");
      }
      return false;
      break;

    case KC_BSPC:
      if (record->event.pressed) {  // On key press.
        if (shift_mods) {  // At least one shift key is held.
          registered_key = KC_DEL;
          // If one shift is held, clear it from the mods. But if both
          // shifts are held, leave as is to send Shift + Del.
          if (shift_mods != MOD_MASK_SHIFT) {
#ifndef NO_ACTION_ONESHOT
            del_oneshot_mods(MOD_MASK_SHIFT);
#endif  // NO_ACTION_ONESHOT
            unregister_mods(MOD_MASK_SHIFT);
          }
        } else {
          registered_key = KC_BSPC;
        }

        register_code(registered_key);
        set_mods(mods);
      } else {  // On key release.
        unregister_code(registered_key);
      }
      return false;
      break;

    // holdで Shift、tapで Caps Word 起動
    case LSFT_T(KC_LSFT):
        if (record->tap.count && record->event.pressed) {
            caps_word_on(); // Turns Caps Word on
            return false;   // Return false to ignore further processing of key
        }
        break;

    case A2J_TOGG:
      if (record->event.pressed) {
        set_jis_mode(!is_jis_mode());
      }
      return false;
      break;
  }

  if (is_jis_mode()) {
    return process_record_user_a2j(keycode, record);
  }

  return true;
}

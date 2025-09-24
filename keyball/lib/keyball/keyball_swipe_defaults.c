// Default KB-level swipe behaviors for extension keys (weak; user can override)
#include "quantum.h"
#include "keyball_swipe.h"

__attribute__((weak)) void keyball_on_swipe_fire(kb_swipe_tag_t tag, kb_swipe_dir_t dir) {
    switch (tag) {
    case KBS_TAG_EX1:
        switch (dir) {
        case KB_SWIPE_UP:    tap_code16(KC_F14); break;
        case KB_SWIPE_RIGHT: tap_code16(KC_F15); break;
        case KB_SWIPE_DOWN:  tap_code16(KC_F16); break;
        case KB_SWIPE_LEFT:  tap_code16(KC_F17); break;
        default: break;
        }
        break;
    case KBS_TAG_EX2:
        switch (dir) {
        case KB_SWIPE_UP:    tap_code16(KC_F19); break;
        case KB_SWIPE_RIGHT: tap_code16(KC_F20); break;
        case KB_SWIPE_DOWN:  tap_code16(KC_F21); break;
        case KB_SWIPE_LEFT:  tap_code16(KC_F22); break;
        default: break;
        }
        break;
    default:
        break;
    }
}

__attribute__((weak)) void keyball_on_swipe_tap(kb_swipe_tag_t tag) {
    switch (tag) {
    case KBS_TAG_EX1: tap_code16(KC_F13); break;
    case KBS_TAG_EX2: tap_code16(KC_F18); break;
    default: break;
    }
}


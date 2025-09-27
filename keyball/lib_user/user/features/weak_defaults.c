#include "quantum.h"

#ifdef COMBO_ENABLE
// Vial/VIA では動的提供されるが、通常QMKビルドでは空配列が必要なため弱シンボルで用意
combo_t key_combos[] __attribute__((weak)) = {};
#endif

#ifdef TAP_DANCE_ENABLE
tap_dance_action_t tap_dance_actions[] __attribute__((weak)) = {};
#endif

#ifdef KEY_OVERRIDE_ENABLE
// keymap_introspection が参照するため、空の配列でも必ずシンボルを用意する
const key_override_t *key_overrides[] __attribute__((weak)) = { NULL };
#endif


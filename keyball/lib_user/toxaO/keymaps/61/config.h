/* mymap config for keyball61 (inherits defaults) */
#pragma once

// 共有設定を継承
#include "../../config.h"

// Vial 用: レイヤー数（mymap は多数のレイヤーを使用）
#undef DYNAMIC_KEYMAP_LAYER_COUNT
#define DYNAMIC_KEYMAP_LAYER_COUNT 16

// Vial 用: キーボード UID（固有値）
#define VIAL_KEYBOARD_UID {0x61, 0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07}

// VIA/Vial 互換維持用にファームウェアバージョンを明示（キーコード割当を更新したため）
#ifndef VIA_FIRMWARE_VERSION
#define VIA_FIRMWARE_VERSION 0x00010002
#endif

// Vial 用: VIAカスタム領域サイズ（Keyballプロファイル格納用）
// Vial のカスタム領域は使用しない（kbpf は EECONFIG_KB に保存）。
// 将来の互換のため定義は残すが、サイズは十分に確保する。
#undef VIA_EEPROM_CUSTOM_CONFIG_SIZE
#define VIA_EEPROM_CUSTOM_CONFIG_SIZE 128

// EEPROM キーボード領域サイズ（kbpf用に十分なサイズを確保）
#undef  EECONFIG_KB_DATA_SIZE
#define EECONFIG_KB_DATA_SIZE 128

// Vial機能: Combo はVialビルド時のみ有効（Unlock用の2キーを定義）
#ifdef VIAL_ENABLE
#define VIAL_COMBO_ENABLE
// Vialビルド時は TAPPING_TERM_PER_KEY を無効化して互換確保
#ifdef VIAL_ENABLE
#undef TAPPING_TERM_PER_KEY
#endif

// Vial 用: アンロックコンボ（任意の2キー。左上2キーを選択）
// Unlock用コンボ（左上2キー）
#define VIAL_UNLOCK_COMBO_ROWS { 0, 0 }
#define VIAL_UNLOCK_COMBO_COLS { 0, 1 }
#endif

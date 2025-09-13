// keyball44 mymap 用の Vial 設定
#pragma once

// 共有設定を継承
#include "../../config.h"

// Vial 用: レイヤー数（mymap は多数のレイヤーを使用）
#undef DYNAMIC_KEYMAP_LAYER_COUNT
#define DYNAMIC_KEYMAP_LAYER_COUNT 12

// Vial 用: キーボード UID（固有値）
#define VIAL_KEYBOARD_UID {0x4B, 0x66, 0x5A, 0xCA, 0x0B, 0xAD, 0x5B, 0x75}

// VIA/Vial 互換維持用にファームウェアバージョンを明示（キーコード割当を更新したため）
#ifndef VIA_FIRMWARE_VERSION
#define VIA_FIRMWARE_VERSION 0x00010002
#endif

// Vial 用: VIAカスタム領域サイズ（Keyballプロファイル格納用）
// Vial のカスタム領域は使用しない（kbpf は EECONFIG_KB に保存）。
// 将来の互換のため定義は残すが、サイズは十分に確保する。
#undef VIA_EEPROM_CUSTOM_CONFIG_SIZE
#define VIA_EEPROM_CUSTOM_CONFIG_SIZE 128

// キーボード用 EEPROM 領域（EECONFIG_KB）の確保（kbpf保存用）
#undef EECONFIG_KB_DATA_SIZE
#define EECONFIG_KB_DATA_SIZE 128

// Vial機能: Combo は Vial 側で扱う
#define VIAL_COMBO_ENABLE
// Vialビルド時は TAPPING_TERM_PER_KEY を無効化して互換確保
#ifdef VIAL_ENABLE
#undef TAPPING_TERM_PER_KEY
#endif

// Vial 用: アンロックコンボ（任意の2キー。左上2キーを選択）
#define VIAL_UNLOCK_COMBO_ROWS { 0, 0 }
#define VIAL_UNLOCK_COMBO_COLS { 0, 1 }

// keyball39 mymap 用の Vial 設定
#pragma once

// 共有設定を継承
#include "../../config.h"

// Vial 用: レイヤー数（mymap は多数のレイヤーを使用）
#undef DYNAMIC_KEYMAP_LAYER_COUNT
#define DYNAMIC_KEYMAP_LAYER_COUNT 12

// Vial 用: キーボード UID（固有値）
#define VIAL_KEYBOARD_UID {0x0B, 0x75, 0x3C, 0x87, 0x03, 0x50, 0x99, 0xA9}

// Vial 用: VIAカスタム領域サイズ（Keyballプロファイル格納用）
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

// VIA/Vial: レイアウトオプションの初期値を Right に設定
// 注意:
// - Vialのレイアウト選択肢は vial.json の labels の順序に依存します。
//   現在は [None=0, Right=1, Left=2, Dual=3] です。
// - 初期化タイミングは「Vial 側のNVM（via.cのマジック）が無効な時」に限り
//   via.c の eeconfig_init_via() で本定数が適用されます。
// - さらに、起動直後に一度だけ Right を強制するワンショット矯正を
//   keyball/lib/keyball/keyball.c に実装しています（VIA/Vial有効時のみ）。
//   これは kbpf.reserved の bit0 をフラグとして使用し、適用済みデバイスでは
//   二度と上書きしません（ユーザー設定を尊重）。
//
// 将来「初期値を Left に変えたい」場合の手順例:
//   1) この定数を Left=0x00000002 に変更。
//   2) 併せてワンショット矯正のターゲット（keyball.c 内）を Left に変更するか、
//      ワンショット矯正を無効化します。
//   3) 既存デバイスで一度だけ矯正を掛け直したい場合は、
//      - KBC_RST を押して kbpf.reserved をクリア → 再起動で新ターゲットが適用
//       （または KBPF_VER_CUR を更新して kbpf デフォルトを再生成）
//      - あるいは Vial 側の NVM をアプリから初期化（デバイスによって手順が異なる）
//
// 以上を踏まえ、配布版では Right を既定とする。
#undef  VIA_EEPROM_LAYOUT_OPTIONS_DEFAULT
#define VIA_EEPROM_LAYOUT_OPTIONS_DEFAULT 0x00000001

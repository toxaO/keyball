#include "quantum.h"
#include "keycode_string.h"
#include "keycode_user.h"

#if KEYCODE_STRING_ENABLE

// ユーザー定義（QK_USER_*）キーコードの表示名を提供するテーブル
// Vial/VIA のキーマップ表示やログで 0x7eXX などの16進表示になるのを防ぎます。
static const keycode_string_name_t keycode_string_names_user_table[] PROGMEM = {
    { APP_SW,   "APP_SW"   },
    { VOL_SW,   "VOL_SW"   },
    { BRO_SW,   "BRO_SW"   },
    { TAB_SW,   "TAB_SW"   },
    { WIN_SW,   "WIN_SW"   },
    { MULTI_A,  "MULTI_A"  },
    { MULTI_B,  "MULTI_B"  },
    { EISU_S_N, "EISU_S_N" },
    { KANA_C_N, "KANA_C_N" },
    { ESC_LNG2, "ESC_LNG2" },
};

// quantum/keycode_string.c の weak シンボルを上書き
const keycode_string_name_t* keycode_string_names_data_user = keycode_string_names_user_table;
uint16_t                     keycode_string_names_size_user = ARRAY_SIZE(keycode_string_names_user_table);

#endif // KEYCODE_STRING_ENABLE


RGBLIGHT_ENABLE = no

OLED_ENABLE = yes

VIA_ENABLE = yes

# OS判別用
OS_DETECTION_ENABLE = yes

# KEY_OVERRIDE_ENABLE = yes
REPEAT_KEY_ENABLE = yes
CAPS_WORD_ENABLE = yes
EXTRAKEY_ENABLE = yes
DYNAMIC_MACRO_ENABLE = yes
DYNAMIC_TAPPING_TERM_ENABLE = yes
# DEFERRED_EXEC_ENABLE = yes
TAP_DANCE_ENABLE = yes
COMBO_ENABLE = yes
DEBOUNCE_TYPE = sym_eager_pk

# BACKLIGHT_ENABLE = yes
RGBLIGHT_ENABLE = yes

SRC += lib/keyball/keyball.c
SRC += features/translate_ansi_to_jis.c
SRC += features/select_word.c

# Vial用の追加設定
LTO_ENABLE = yes       #Scotto氏は推している
VIAL_ENABLE = yes
QMK_SETTINGS = yes   #VialのGUI上で上級修行僧としてマニアックな設定を可能とし、タップダンスのディープな設定を可能とするためにも必須

#推奨の呪文　QMK_SETTINGS = yesにして個別に有効化が必要
KEY_OVERRIDE_ENABLE = yes
MAGIC_ENABLE = yes
GRAVE_ESC_ENABLE = yes
MOUSEKEY_ENABLE = yes
NKRO_ENABLE = yes

#エンコーダ、OLED、七色発行体も利用可能なので、お好みでどうぞ
ENCODER_MAP_ENABLE = yes

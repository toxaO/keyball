# default
RGBLIGHT_ENABLE = yes
OLED_ENABLE = yes

# personal setting
EXTRAKEY_ENABLE = yes
MACRO_ENABLE = yes
KEY_OVERRIDE_ENABLE = yes
OS_DETECTION_ENABLE = yes
TAP_DANCE_ENABLE = no
DEFERRED_EXEC_ENABLE = yes

CONSOLE_ENABLE = yes
COMMAND_ENABLE = no
SPACE_CADET_ENEBLE = no
GRAVE_ESC_ENABLE = no
MAGIC_ENABLE = no
MUSIC_MODE = no

# 不具合解決（以下のようにしておかないとバグる）
# EXTRAFLAGS += -flto
# LTO_ENABLE = yes
KEYBOARD_SHARED_EP = no

# Include my library
SRC += lib_user/toxaO/features/layer_user.c
SRC += lib_user/toxaO/features/macro_user.c
SRC += lib_user/toxaO/features/oled_user.c
SRC += lib_user/toxaO/features/swipe_user.c
SRC += lib_user/toxaO/features/multi_user.c
SRC += lib_user/toxaO/features/util_user.c
SRC += lib_user/toxaO/features/combo_user.c

# Vial互換のために動的キーマップ/RAWを明示有効化
DYNAMIC_KEYMAP_ENABLE = yes
RAW_ENABLE = yes

# vial-qmk は -DNO_DEBUG を有効にするため、コンソール出力を行うために明示的に解除
OPT_DEFS += -UNO_DEBUG

# Vialビルドでは quantum/via.c を強制的に取り込む（raw_hid_receive 実装のため）
ifeq ($(strip $(VIAL_ENABLE)), yes)
SRC += quantum/via.c
endif

# default
RGBLIGHT_ENABLE = yes
OLED_ENABLE = yes

# personal setting
EXTRAKEY_ENABLE = yes
MACRO_ENABLE = yes
KEY_OVERRIDE_ENABLE = no
OS_DETECTION_ENABLE = yes
TAP_DANCE_ENABLE = no

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
SRC += lib_user/features/layer_user.c
SRC += lib_user/features/macro_user.c
SRC += lib_user/features/oled_user.c
# SRC += lib_user/features/swipe_user.c
SRC += lib_user/features/swipe_user.c
SRC += lib_user/features/util_user.c
SRC += lib_user/features/combo_user.c

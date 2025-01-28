RGBLIGHT_ENABLE = yes
OLED_ENABLE = yes
EXTRAKEY_ENABLE = yes
MACRO_ENABLE = yes
KEY_OVERRIDE_ENABLE = yes
OS_DETECTION_ENABLE = yes
TAP_DANCE_ENABLE = yes

# 容量削減
LTO_ENABLE = yes
CONSOLE_ENABLE = no
COMMAND_ENABLE = no
SPACE_CADET_ENEBLE = no
GRAVE_ESC_ENABLE = no
MAGIC_ENABLE = no
MUSIC_MODE = no

# Include my library
SRC += mylib/features/layer.c
SRC += mylib/features/macro_key.c
SRC += mylib/features/oled.c
SRC += mylib/features/swipe.c
SRC += mylib/features/util.c

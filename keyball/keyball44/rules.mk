# Link Time Optimization required for size.
# LTO_ENABLE = yes

# Build Options
BOOTMAGIC_ENABLE = no       # Enable Bootmagic Lite
EXTRAKEY_ENABLE = yes        # Audio control and System control
CONSOLE_ENABLE = yes         # Console for debug
COMMAND_ENABLE = no         # Commands for debug and configuration
NKRO_ENABLE = no            # Enable N-Key Rollover
BACKLIGHT_ENABLE = no       # Enable keyboard backlight functionality
AUDIO_ENABLE = no           # Audio output

HAPTIC_ENABLE = yes
HAPTIC_DRIVER = drv2605l

# Keyball39 is split keyboard.
SERIAL_DRIVER = vendor

# Optical sensor driver for trackball.
POINTING_DEVICE_ENABLE = yes
POINTING_DEVICE_DRIVER = pmw3360

MOUSEKEY_ENABLE = yes

# Enabled only one of RGBLIGHT and RGB_MATRIX if necessary.
RGBLIGHT_ENABLE = no        # Enable RGBLIGHT
RGB_MATRIX_ENABLE = no      # Enable RGB_MATRIX (not work yet)
RGB_MATRIX_DRIVER = ws2812

# Do not enable SLEEP_LED_ENABLE. it uses the same timer as BACKLIGHT_ENABLE
SLEEP_LED_ENABLE = no       # Breathing sleep LED during USB suspend

# To support OLED
OLED_ENABLE = no                # Please Enable this in each keymaps.
SRC += lib/oledkit/oledkit.c    # OLED utility for Keyball series.

# Include common library
SRC += lib/keyball/keyball.c
SRC += lib/keyball/keyball_move.c
SRC += lib/keyball/keyball_scroll.c
SRC += lib/keyball/keyball_keycodes.c
SRC += lib/keyball/keyball_swipe.c
SRC += lib/keyball/keyball_oled.c
SRC += lib/keyball/keyball_kbpf.c
SRC += lib/keyball/keyball_led.c

# Disable other features to squeeze firmware size
SPACE_CADET_ENABLE = no
MAGIC_ENABLE = no

# Vialのアンロックコンボ定義はキーマップ側のconfig.hに移動（重複定義防止）

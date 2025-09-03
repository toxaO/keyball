#define WS2812_DI_PIN GP0
#ifdef RGBLIGHT_ENABLE
#    define RGBLIGHT_DEFAULT_MODE RGBLIGHT_MODE_RAINBOW_SWIRL
#endif

#ifndef OLED_FONT_H
#    define OLED_FONT_H "keyboards/keyball/lib/logofont/logofont.c"
#    define OLED_FONT_START 32
#    define OLED_FONT_END 195
#endif

// This has been reported to help with recognizing the keyboard after booting up when its USB is already connected.
#define SPLIT_WATCHDOG_ENABLE
#define SPLIT_WATCHDOG_TIMEOUT 3000

#define SPI_DRIVER SPID0
#define SPI_SCK_PIN GP22
#define SPI_MISO_PIN GP20
#define SPI_MOSI_PIN GP23
#define PMW33XX_CS_PIN GP21
#define SPLIT_POINTING_ENABLE
#define POINTING_DEVICE_COMBINED

// Rotation settings for left side trackball.
#define POINTING_DEVICE_ROTATION_90
#define POINTING_DEVICE_INVERT_Y

// Rotation settings for right side trackball.
#define POINTING_DEVICE_ROTATION_270_RIGHT
#define POINTING_DEVICE_INVERT_Y_RIGHT

#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 1000U

#define MOUSE_EXTENDED_REPORT
#define WHEEL_EXTENDED_REPORT
#define POINTING_DEVICE_HIRES_SCROLL_ENABLE

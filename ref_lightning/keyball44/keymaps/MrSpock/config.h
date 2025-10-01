/*
This is the c configuration file for the keymap

Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WA-jRRANTY; sdf  without even the implied warranty of
MERCHANTABILITjY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
//#define BALL

/*
RGBLIGHTを改造した関係でVIA.cの以下の関数の中のコードをすべて空関数にする。

void via_qmk_rgblight_command(uint8_t *data, uint8_t length)
void via_qmk_rgblight_get_value(uint8_t *data)
void via_qmk_rgblight_set_value(uint8_t *data)

また、keyball.cは不要な部分をコメントアウト
*/
#define VIA_C_MODIFICATION 

#define OLEDKIT_DISABLE
#define SPLIT_LED_STATE_ENABLE

#define USB_MAX_POWER_CONSUMPTION 500

#define RGBLIGHT_ENABLE
#define WS2812_DI_PIN   D3
#define RGBLIGHT_SPLIT
#define RGBLED_NUM      60
#define RGBLED_SPLIT    { 30, 30 }  // (30 + 29)

#define TAP_CODE_DELAY 5
#define DYNAMIC_KEYMAP_LAYER_COUNT 5
#define LAYER_STATE_8bit
#define COMBO_TERM 40
#define KEYBALL_CPI_DEFAULT 500      // 光学センサーPMW3360DM の解像度 (CPI) の規定値
#define KEYBALL_SCROLL_DIV_DEFAULT 4
#define KEYBALL_SCROLLSNAP_ENABLE 1

#define SPLIT_TRANSACTION_IDS_USER USER_SYNC_KEY_COUNTER
#ifdef BALL
    #define OLED_FONT_H "font.c"
    #define OLED_FONT_START 48
    #define OLED_FONT_END 90
#endif

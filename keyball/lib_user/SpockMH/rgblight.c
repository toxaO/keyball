/* Copyright 2016-2017 Yang Liu
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "quantum.h"
#include "progmem.h"
#include "rgblight.h"
#include "color.h"
#include "led_tables.h"
#include <lib/lib8tion/lib8tion.h>


#define RGBLIGHT_SPLIT_SET_CHANGE_MODE rgblight_status.change_flags |= RGBLIGHT_STATUS_CHANGE_MODE
#define RGBLIGHT_SPLIT_SET_CHANGE_HSVS rgblight_status.change_flags |= RGBLIGHT_STATUS_CHANGE_HSVS
#define RGBLIGHT_SPLIT_SET_CHANGE_MODEHSVS rgblight_status.change_flags |= (RGBLIGHT_STATUS_CHANGE_MODE | RGBLIGHT_STATUS_CHANGE_HSVS)

#define _RGBM_SINGLE_DYNAMIC(sym) RGBLIGHT_MODE_##sym,

static uint8_t mode_base_table[] = {
    0, // RGBLIGHT_MODE_zero
#include "rgblight_modes.h"
};

#define RGBLIGHT_DEFAULT_MODE RGBLIGHT_MODE_HEATMAP

#define RGBLIGHT_DEFAULT_HUE 0
#define RGBLIGHT_DEFAULT_SAT UINT8_MAX
#if !defined(RGBLIGHT_DEFAULT_VAL)
#    define RGBLIGHT_DEFAULT_VAL RGBLIGHT_LIMIT_VAL
#endif

uint16_t effect_timer;

rgblight_config_t rgblight_config;
rgblight_status_t rgblight_status         = {.timer_enabled = false};
bool              is_rgblight_initialized = false;

#ifndef LED_ARRAY
LED_TYPE led[RGBLED_NUM];
#    define LED_ARRAY led
#endif

rgblight_ranges_t rgblight_ranges = {0, RGBLED_NUM, 0, RGBLED_NUM, RGBLED_NUM};

void rgblight_set_clipping_range(uint8_t start_pos, uint8_t num_leds) {
    rgblight_ranges.clipping_start_pos = start_pos;
    rgblight_ranges.clipping_num_leds  = num_leds;
}

void rgblight_set_effect_range(uint8_t start_pos, uint8_t num_leds) {
    if (start_pos >= RGBLED_NUM) return;
    if (start_pos + num_leds > RGBLED_NUM) return;
    rgblight_ranges.effect_start_pos = start_pos;
    rgblight_ranges.effect_end_pos   = start_pos + num_leds;
    rgblight_ranges.effect_num_leds  = num_leds;
}

__attribute__((weak)) RGB rgblight_hsv_to_rgb(HSV hsv) {
    return hsv_to_rgb(hsv);
}

void sethsv_raw(uint8_t hue, uint8_t sat, uint8_t val, LED_TYPE *led1) {
    HSV hsv = {hue, sat, val};
    RGB rgb = rgblight_hsv_to_rgb(hsv);
    setrgb(rgb.r, rgb.g, rgb.b, led1);
}

void sethsv(uint8_t hue, uint8_t sat, uint8_t val, LED_TYPE *led1) {
    sethsv_raw(hue, sat, val > RGBLIGHT_LIMIT_VAL ? RGBLIGHT_LIMIT_VAL : val, led1);
}

void setrgb(uint8_t r, uint8_t g, uint8_t b, LED_TYPE *led1) {
    led1->r = r;
    led1->g = g;
    led1->b = b;
}

void eeconfig_update_rgblight_default(void) {
    rgblight_config.mode    = RGBLIGHT_DEFAULT_MODE;
    rgblight_config.rainbow_mode = false;
    rgblight_config.hue     = RGBLIGHT_DEFAULT_HUE;
    rgblight_config.sat     = RGBLIGHT_DEFAULT_SAT;
    rgblight_config.val     = RGBLIGHT_DEFAULT_VAL;
    RGBLIGHT_SPLIT_SET_CHANGE_MODEHSVS;
}

void rgblight_init(void) {
    if (is_rgblight_initialized) {
        return;
    }

    RGBLIGHT_SPLIT_SET_CHANGE_MODEHSVS;
    if (!rgblight_config.mode) {
        eeconfig_update_rgblight_default();
    }

    if (rgblight_config.enable) {
        rgblight_mode(rgblight_config.mode);
    }

    is_rgblight_initialized = true;
}

void rgblight_mode(uint8_t mode) {
    if (!rgblight_config.enable) {
        return;
    }
    if (mode > RGBLIGHT_MODES) {
        rgblight_config.mode = RGBLIGHT_MODES;
    } else {
        rgblight_config.mode = mode;
    }
    RGBLIGHT_SPLIT_SET_CHANGE_MODE;
    rgblight_sethsv(rgblight_config.hue, rgblight_config.sat, rgblight_config.val);
}

void rgblight_toggle(void) {
    if (rgblight_config.enable) {
        rgblight_disable();
    } else {
        rgblight_enable();
    }
}

void rgblight_enable(void) {
    rgblight_config.enable = 1;
    rgblight_mode(rgblight_config.mode);
}

void rgblight_disable(void) {
    rgblight_config.enable = 0;
    rgblight_config.rainbow_mode = !rgblight_config.rainbow_mode;
    RGBLIGHT_SPLIT_SET_CHANGE_MODE;
    rgblight_set();
}

bool rgblight_is_enabled(void) {
    return rgblight_config.enable;
}

void rgblight_increase_val(void) {
    uint8_t val = qadd8(rgblight_config.val, RGBLIGHT_VAL_STEP);
    val = val > RGBLIGHT_LIMIT_VAL ? RGBLIGHT_LIMIT_VAL : val;
    rgblight_sethsv(rgblight_config.hue, rgblight_config.sat, val);
}

void rgblight_decrease_val(void) {
    uint8_t val = qsub8(rgblight_config.val, RGBLIGHT_VAL_STEP);
    rgblight_sethsv(rgblight_config.hue, rgblight_config.sat, val);
}

void rgblight_sethsv(uint8_t hue, uint8_t sat, uint8_t val) {
    if (rgblight_config.enable) {

        if (rgblight_config.hue != hue || rgblight_config.sat != sat || rgblight_config.val != val) {
            RGBLIGHT_SPLIT_SET_CHANGE_HSVS;
        }
        rgblight_status.base_mode = mode_base_table[rgblight_config.mode];
        rgblight_config.hue = hue;
        rgblight_config.sat = sat;
        rgblight_config.val = val;
    }
}

uint8_t rgblight_get_hue(void) {
    return rgblight_config.hue;
}

uint8_t rgblight_get_sat(void) {
    return rgblight_config.sat;
}

uint8_t rgblight_get_val(void) {
    return rgblight_config.val;
}

void rgblight_set(void) {
    LED_TYPE *start_led;
    uint8_t   num_leds = rgblight_ranges.clipping_num_leds;

    if (!rgblight_config.enable) {
        for (uint8_t i = rgblight_ranges.effect_start_pos; i < rgblight_ranges.effect_end_pos; i++) {
            led[i].r = 0;
            led[i].g = 0;
            led[i].b = 0;
        }
    }
    start_led = led + rgblight_ranges.clipping_start_pos;
    ws2812_setleds(start_led, num_leds);
}

/* for split keyboard master side */
uint8_t rgblight_get_change_flags(void) {
    return rgblight_status.change_flags;
}

void rgblight_clear_change_flags(void) {
    rgblight_status.change_flags = 0;
}

void rgblight_get_syncinfo(rgblight_syncinfo_t *syncinfo) {
    syncinfo->config = rgblight_config;
    syncinfo->status = rgblight_status;
}

/* for split keyboard slave side */
void rgblight_update_sync(rgblight_syncinfo_t *syncinfo, bool write_to_eeprom) {

    if (syncinfo->status.change_flags & RGBLIGHT_STATUS_CHANGE_MODE) {
        if (syncinfo->config.enable) {
            rgblight_config.enable = 1; 
            rgblight_mode(syncinfo->config.mode);
        } else {
            rgblight_disable();
        }
        rgblight_config.rainbow_mode = syncinfo->config.rainbow_mode;
    }
    if (syncinfo->status.change_flags & RGBLIGHT_STATUS_CHANGE_HSVS) {
        rgblight_sethsv(syncinfo->config.hue, syncinfo->config.sat, syncinfo->config.val);
    }
}

///////////////////////////////////////////////////////////////////////////////
//RGBlight魔改造
///////////////////////////////////////////////////////////////////////////////

/*  
//Keyball44配置図
    L00,L01,L02,L03,L04,L05,    R05,R04,R03,R02,R01,R00, \
    L10,L11,L12,L13,L14,L15,    R15,R14,R13,R12,R11,R10, \
    L20,L21,L22,L23,L24,L25,    R25,R24,R23,R22,R21,R20, \
        L31,L32,L33,L34,L35,    R35,R34,        R31      \
    ) 
    { 
        {   L00,   L01,   L02,   L03,   L04,   L05 }, \
        {   L10,   L11,   L12,   L13,   L14,   L15 }, \
        {   L20,   L21,   L22,   L23,   L24,   L25 }, \
        { KC_NO,   L31,   L32,   L33,   L34,   L35 }, \
        {   R00,   R01,   R02,   R03,   R04,   R05 }, \
        {   R10,   R11,   R12,   R13,   R14,   R15 }, \
        {   R20,   R21,   R22,   R23,   R24,   R25 }, \
        { KC_NO,   R31, KC_NO, KC_NO,   R34,   R35 }, \
    }
*/

/*
//Keyball44 LED No.(親指は下向きを採用)
    17, 14, 10, 6, 3, 0,     56, 53, 50, 47, 43, 40, 
    18, 15, 11, 7, 4, 1,     57, 54, 51, 48, 44, 41, 
    19, 16, 12, 8, 5, 2,     58, 55, 52, 49, 45, 42, 
        13,  9,27,28,29,     30, 31,      46     
*/

// ROWとCOLに対応するLEDインデックスの定数配列
const uint8_t led_index[MATRIX_ROWS][MATRIX_COLS] = {
//col  0   1   2   3   4   5
    { 17, 14, 10,  6,  3,  0 },  // Row 0 left 
    { 18, 15, 11,  7,  4,  1 },  // Row 1 left
    { 19, 16, 12,  8,  5,  2 },  // Row 2 left
    { 59, 13,  9, 27, 28, 29 },  // Row 3 left
    { 40, 43, 47, 50, 53, 56 },  // Row 4 right
    { 41, 44, 48, 51, 54, 57 },  // Row 5 right
    { 42, 45, 49, 52, 55, 58 },  // Row 6 right
    { 59, 46, 59, 59, 31, 30 }   // Row 7 right
};

//各LEDのvalue(この値を使いヒートマップを再現)
//value[led_index[row][col]]と記載することでrow/colからLEDの値まで連動する。
static uint8_t value[RGBLED_NUM] = {0};

//各LEDのhue値(この値を使いmousemoveのhue値を保存)
static uint8_t hue_value[RGBLED_NUM] = {0};


void value_add(uint8_t row, uint8_t col, uint8_t add){
    value[led_index[row][col]]=qadd8(value[led_index[row][col]],add);
}

///////////////////////////////////////////////////////////////////////////////
//ヒートマップで使用する関数、押したキーとその上下左右が半値上昇
///////////////////////////////////////////////////////////////////////////////
void rgblight_value_range(uint8_t row, uint8_t col, uint8_t add){

    if(add>5){
        //押されたキーはadd分value値を追加
        value_add(row,col,add);
        //上下左右についてはaddの半値を追加、左右連結しないよう特定の行、列は除外
        if(row!=0 && row!=4){ value_add(row-1,col,add/2); }
        if(row!=3 && row!=7){ value_add(row+1,col,add/2); }
        if(col!=0){           value_add(row,col-1,add/2); }
        if(col!=5){           value_add(row,col+1,add/2); }
    } else {
        //addがない場合はvalue値を全部下げる
        for (uint8_t i = 0; i < RGBLED_NUM; i++) {
            value[i] = qsub8(value[i],1);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//マウスムーブで使用する関数
///////////////////////////////////////////////////////////////////////////////
void rgblight_value(uint8_t row, uint8_t col, bool update, bool scr){
    uint8_t i;
    static uint8_t last_row;
    static uint8_t last_col;
    uint8_t last_index = led_index[last_row][last_col];
    
    //ターゲットとなるキーにアップデート(移動)があった場合
    if(update){
        //キーが更新されていた場合に更新
        if(last_row!=row || last_col!=col){
            //現在のキーのLEDを255にする
            value[led_index[row][col]]=255;
            if(scr){
                //スクロールモードの場合、ターゲットキーを対象に一列同じ値更新
                for (i = 0; i < 6; i++) {
                    value[led_index[row][i]]=255;
                    if(rgblight_config.rainbow_mode){
                        hue_value[led_index[row][i]]=hue_value[last_index] - 20;
                    } else {
                        hue_value[led_index[row][i]]=qadd8(hue_value[last_index],128);
                    }
                }
            } else {
                //マウスモードの場合、ターゲットキーを対象に値更新
                if(rgblight_config.rainbow_mode){
                    hue_value[led_index[row][col]]=hue_value[last_index] - 16;
                } else {
                    hue_value[led_index[row][col]]=qadd8(hue_value[last_index],64);
                }
            }
        }
        last_row = row;
        last_col = col;
    } else {
    //ターゲットとなるキーにアップデート(移動)がなかった場合
        //value値を-2し軌跡を消してく
        for (i = 0; i < RGBLED_NUM; i++) {
            value[i] = qsub8(value[i],4);
        }   
        if (scr) {
            for (i = 0; i < 6; i++) {
                value[led_index[last_row][i]]=255;
            }   
        } else {
            value[last_index]=255; //前回の値を255でキープ
        } 
        if(rgblight_config.rainbow_mode){
            if(hue_value[last_index] != rgblight_config.hue){
                for (i = 0; i < RGBLED_NUM; i++) {
                    hue_value[i]--;
                }
            }
        } else {
            for (i = 0; i < RGBLED_NUM; i++) {
                hue_value[i] = qsub8(hue_value[i],1);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//エフェクト切り替え時のリセット関数
///////////////////////////////////////////////////////////////////////////////
void rgblight_value_reset(void){
    for (uint8_t i = 0; i < RGBLED_NUM; i++) {
        value[i] = 0;
        hue_value[i] = rgblight_config.hue;
    }
}

///////////////////////////////////////////////////////////////////////////////
//下側のLEDの制御関数
///////////////////////////////////////////////////////////////////////////////
void underled(bool on, uint8_t hue, uint8_t sat) {
    static uint8_t cur_hue=0;
    uint8_t val = on ? rgblight_config.val : 0;
    for (uint8_t i = 20; i < 40; i++) {
        if (rgblight_config.rainbow_mode) {
            sethsv((i-20)*13 + cur_hue, 255, val, (LED_TYPE *)&led[i]);
            
        } else {
            sethsv(hue, sat, val, (LED_TYPE *)&led[i]);
        }
    }
    cur_hue++;
}

///////////////////////////////////////////////////////////////////////////////
//ヒートマップエフェクト
///////////////////////////////////////////////////////////////////////////////
void rgblight_effect_heatmap(void) {
    if (rgblight_config.enable) {
        uint8_t i;
        uint8_t hue,val;

        for (i = 0; i < RGBLED_NUM; i++) {
            val = 150;
            //各LEDのvalue値で、LEDの挙動を制御
            //valueが75以下の場合は青で光量を上げる
            if(value[i] < 75){
                val = value[i] * 2;
                hue = 169;
            //それ以降はhueを上げていく、緑あたりは速度を上げ、赤付近は速度を低下
            }else if(value[i] < 117){ hue = 169 - (value[i]-75);
            }else if(value[i] < 159){ hue = 127 - (value[i]-117)*2;
            }else if(value[i] < 245){ hue =  43 - (value[i]-159)/2;
            }else{ hue = 0;
            }

            if (val>rgblight_config.val){
                val = rgblight_config.val;
            }
            sethsv(hue, 255, val, (LED_TYPE *)&led[i]);
        }
        if(host_keyboard_led_state().caps_lock || is_caps_word_on()){
            underled(true,rgblight_config.hue,255);
        }

        rgblight_set();
        rgblight_value_range(0,0,0); //value低下処理
    }
}

///////////////////////////////////////////////////////////////////////////////
//アイスウェーブエフェクト
///////////////////////////////////////////////////////////////////////////////
void rgblight_effect_icewave(void) {
    uint8_t sat;
    uint8_t led_sat=0;
    static uint8_t cur_sat=0;
    static bool flag=false;
    
    for (uint8_t i = 0; i < RGBLED_NUM; i++) {
        led_sat=(255 / RGBLED_NUM * i);
        if(((led_sat + cur_sat*2)/256)%2==0){
            sat = led_sat+cur_sat*2;
        } else{
            sat = 255-(led_sat+cur_sat*2);
        }
        
        sat = qadd8(sat,value[i]);

        sethsv(rgblight_config.hue, sat, rgblight_config.val, (LED_TYPE *)&led[i]);
    }
    underled(false,0,0);
    rgblight_set();
    
    if (cur_sat==255) {flag=true;}
    if (cur_sat==0) {flag=false;}
    if(flag){
        cur_sat--;
    } else {
        cur_sat++;
    }
    rgblight_value_range(0,0,0);
}
///////////////////////////////////////////////////////////////////////////////
//マウスムーブエフェクト
///////////////////////////////////////////////////////////////////////////////
void rgblight_effect_mousemove(void) {
    //ほぼrgblight_value関数で作りこんでいるため、各LEDにhsvを設定するのみ
    for (uint8_t i = 0; i < RGBLED_NUM; i++) {
        if(rgblight_config.rainbow_mode) {
            sethsv_raw(hue_value[i], 255, value[i], (LED_TYPE *)&led[i]);
        }else{
            sethsv_raw(rgblight_config.hue, hue_value[i], value[i], (LED_TYPE *)&led[i]); 
        }
    }
    underled(true,0,0);
    rgblight_set();
    rgblight_value(0,0,false,false); //軌跡削除処理
}

///////////////////////////////////////////////////////////////////////////////
//スクロールムーブエフェクト
///////////////////////////////////////////////////////////////////////////////
void rgblight_effect_scrollmove(void) {
    //ほぼrgblight_value関数で作りこんでいるため、各LEDにhsvを設定するのみ
    for (uint8_t i = 0; i < RGBLED_NUM; i++) {
        //valueがval値を超えている場合はval値を下げる
        if(rgblight_config.rainbow_mode) {
            sethsv_raw(hue_value[i], 255, value[i], (LED_TYPE *)&led[i]);
        }else{
            sethsv_raw(rgblight_config.hue, hue_value[i], value[i], (LED_TYPE *)&led[i]); 
        }
    }
    underled(true,0,0);
    rgblight_set();
    rgblight_value(0,0,false,true); //軌跡削除処理
}

///////////////////////////////////////////////////////////////////////////////
//スタティックエフェクト
///////////////////////////////////////////////////////////////////////////////
void rgblight_effect_static(void) {
    for (uint8_t i = 0; i < RGBLED_NUM; i++) {
        sethsv(rgblight_config.hue, 255, rgblight_config.val, (LED_TYPE *)&led[i]);
    }
    rgblight_set();
}

void rgblight_task(void) {
    if (rgblight_config.enable) {
        uint8_t interval_time = 255; // dummy interval
        static uint8_t last_mode;
        
        if (rgblight_status.base_mode != last_mode) {
            rgblight_value_reset();
        }
        
        if (rgblight_status.base_mode == RGBLIGHT_MODE_SCROLLMOVE) {
            interval_time = 16;
        } else if (rgblight_status.base_mode == RGBLIGHT_MODE_HEATMAP) {
            interval_time = 32;
        } else if (rgblight_status.base_mode == RGBLIGHT_MODE_ICEWAVE) {
            interval_time = 16;
        } else if (rgblight_status.base_mode == RGBLIGHT_MODE_MOUSEMOVE) {
            interval_time = 16;
        }
        if (timer_elapsed(effect_timer)>=interval_time){
            effect_timer = timer_read();
            if (rgblight_status.base_mode == RGBLIGHT_MODE_SCROLLMOVE) {
                rgblight_effect_scrollmove();
            } else if (rgblight_status.base_mode == RGBLIGHT_MODE_HEATMAP) {
                rgblight_effect_heatmap();
            } else if (rgblight_status.base_mode == RGBLIGHT_MODE_ICEWAVE) {
                rgblight_effect_icewave();
            } else if (rgblight_status.base_mode == RGBLIGHT_MODE_MOUSEMOVE) {
                rgblight_effect_mousemove();
            } else if (rgblight_status.base_mode == RGBLIGHT_MODE_STATIC) {
                rgblight_effect_static();
            }
        }
        last_mode = rgblight_status.base_mode;
    }
}

void rgblight_sethue(uint8_t hue){
    rgblight_config.hue = hue;
    RGBLIGHT_SPLIT_SET_CHANGE_HSVS;
}

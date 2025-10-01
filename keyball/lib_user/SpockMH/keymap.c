/*
Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
以下の機能を備えたファームウェアに改造
- 右(ボール側)の場合はconfig.hで#define BALLを定義、
　左は定義せずにそれぞれ異なるファームウェアを適用することでサイズ問題を解消。
- マスターは基本右固定(左でも動くが、ライティングOFF、右OLED不使用)
- qmkコアな部分もちょこちょこといじってしまい、フルコピーではビルド通らないと思います。
　すいません。部分的にコピーするなど参考程度にご使用頂ければと思います。

(a)レイヤーの概要は以下の通り。
　(0)デフォルトレイヤー
　(1)左:記号、右:テンキー
　(2)ファンクション、ユーティリティー
　(3)マウスレイヤー(JLがマウス左右クリックのみデフォルトレイヤーから変更)
　(4)スクロールレイヤー

(b)Auto Mouse Layerの機能を簡易的に再現(容量削減)
　マウスレイヤーはデフォルトレイヤーからJに右クリック、Lに左クリックのみ変更を行っており、
　基本的にタイピングすることにより、デフォルトレイヤーに戻る。
　ただし、process_record_userにて右クリック、左クリック、コントロール、シフトが押された
　場合は、除外処理が走り、ダブルクリック、シフトクリック等に対応している。
　(1)デフォルトレイヤーから動作量が30を超えた場合にマウスレイヤーに自動移行
　(2)以下の条件でマウスレイヤーから抜ける。
　  - 30秒経過
　　- 特定のキー以外のタイピング
(c)RGBLightingを容量削減しつつ魔改造し、以下のように光るようにした。
　約4kb程度の容量で以下を実現した。
　(0)ヒートマップ：押す頻度の高いキーが青から赤に変わっていくアニメーション。
　(1)アイスウェーブ：基本は単色白で青色のグラデーションが走るアニメーション。
　(2)スタティック
　(3)マウスムーブ：マウスの動作に合わせて光るキーが軌跡を表示するアニメーション。
　(4)スクロールムーブ：横1列が光り、スクロール上下により軌跡を表示するーアニメーション。
  ライティングをOFF/ONとすることで、ノーマルモードとレインボーモードが切り替わる。
  マウスムーブとスクロールムーブの色が単色か虹色かを変更することができる。

(d)OLED表示
　右と左で異なる表示にしたかったが、ファームウェアサイズがたりず、OLEDの左右変更のため、
　ファームウェアを左右で変更する必要が出た。
　(右)状態表示、フォントはサイズ削減のためミニマム分のみを実装
　(左)SNOWアニメーション：タイピングやマウス操作により、雪が降ってきて積もるアニメーションを作成

(e)その他
　(1)レイヤー1はマウス操作で左=BS,右=DELで連続消しができるように変更
　(2)レイヤー2はマウス操作で矢印キーを実現
　(3)USER0キーを押している間、一時的にCPIを3倍にする機能
　(4)AMLON/OFF時にWin+F13を出力。PowerToysの機能と合わせてAMLかどうかをモニタ上でも確認できる。
*/

#include QMK_KEYBOARD_H
#include "quantum.h"
#include "transactions.h"
#include <lib/lib8tion/lib8tion.h>
#include <stdlib.h>
#include "rgblight.h"

static uint8_t ball_tension; //動き始めの誤操作防止用
static uint8_t hue = 169; 
const uint16_t AML_reset_time = 30000; //AMLのリセット時間
const  uint8_t AML_layer = 3; //AMLのレイヤー
const  uint8_t SCR_layer = 4; //スクロールのレイヤー
#ifndef BALL 
  static uint8_t keyCounter = 0; //キーカウンター
#endif
static uint16_t move_timer;
//左右同期用のデータ構造
typedef struct _sync_data_t{
  //キーのrow,col位置情報(ライティング用)
  uint8_t key_row:4;
  uint8_t key_col:3;
  //デフォルトレイヤー or NOT(ライティング用)
  bool layer_state:1;
  bool scr:1;
} sync_data_t;

sync_data_t sync_data = {0,0,false,false};

///////////////////////////////////////////////////////////////////////////////
//SLAVE側で発生する処理
///////////////////////////////////////////////////////////////////////////////
void user_sync_a_update_keyCounter_on_other_board(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data) {
#ifndef BALL //BALLではない(SLAVE側)場合のみ実行
  const sync_data_t* data = (const sync_data_t*)in_data;
  keyCounter++;// = data->key_counter;//SNOW用キーカウンター同期
  if(data->layer_state){
    //デフォルトレイヤーでない場合単キーの値上昇(mousemoveライティング用)
    rgblight_value(data->key_row,data->key_col,true,data->scr);
  } else {
    //デフォルトレイヤーの場合は上下左右含めた値上昇(ヒートマップ用)
    rgblight_value_range(data->key_row,data->key_col,32);
  }
#endif
}

///////////////////////////////////////////////////////////////////////////////
//イニシャライズ処理
///////////////////////////////////////////////////////////////////////////////j
void keyboard_post_init_user(void) {
  transaction_register_rpc(USER_SYNC_KEY_COUNTER, user_sync_a_update_keyCounter_on_other_board);
  rgblight_sethue(hue);
//左手マスターの場合はライティングOFF
#ifndef BALL
  rgblight_disable();
#else
  rgblight_enable();
#endif
}

///////////////////////////////////////////////////////////////////////////////
//キー処理
///////////////////////////////////////////////////////////////////////////////
bool process_record_user(uint16_t keycode, keyrecord_t *record){  
  static bool AML_key_press = false;
  //キーイベント情報をSLAVE側に同期
#ifdef BALL
  if(!layer_state_is(AML_layer)){
    sync_data.key_row = record->event.key.row;
    sync_data.key_col = record->event.key.col;
    sync_data.layer_state = false;

    transaction_rpc_send(USER_SYNC_KEY_COUNTER, sizeof(sync_data), &sync_data);
    //MASTER側のライティング処理
    rgblight_value_range(record->event.key.row,record->event.key.col,32);
  }
#else
  keyCounter++; //Keypress+Keyreleaseでカウンター加算(snow用)
#endif

  //USER0を押している場合にCPIを3倍する処理
  if (keycode == QK_USER_0){
    if(record->event.pressed){
      keyball_set_cpi(keyball_get_cpi()*3);
      return true;
    } else {
      set_last_cpi(); //keyball側に元に戻す関数追加
    }
  }

  if (keycode == QK_USER_1 && record->event.pressed){
    hue += 3;
    rgblight_sethue(hue); //rgblightingにhueのみ変更する関数追加。
  }
  //SHITFまたはCABSWORDがONの時に、- 入力で _　出力(英語キーボードにそろえる処理)
  uint8_t mods = get_mods();
  if (keycode == 0x442D && record->event.pressed && 
      (is_caps_word_on() || (mods & MOD_MASK_SHIFT))) {
    tap_code16(0x287);
    return false;
  }

  //マウスレイヤーをOFFしないキーの処理
  //0x2150=ctrl(←)  //0x2207=shitf(D)  //0xD1,D2=右左クリック
  if( keycode == 0x2150 || keycode == 0x2207 ||
      keycode == 0xD1   || keycode == 0xD2     ){
    if(record->event.pressed) {
      AML_key_press = true;
    } else {
      AML_key_press = false;
      return true;
    }
  }

  //Layer4(スクロールレイヤー)がOFFになったときはマウスレイヤーをON　
  //0x442D=Layer4,-  //0x5080=LM4
  if((keycode == 0x442D || keycode == 0x5080) && 
      !record->event.pressed && layer_state_is(SCR_layer)){
    layer_on(AML_layer);
    return true;
  }
  if( AML_key_press ){
    return true;
  } 

  if(layer_state_is(AML_layer)){
    layer_off(AML_layer);
  }
  return true;
}

void call_rgblight(bool scr){
  sync_data.layer_state = true;
  sync_data.scr = scr;
  //更新された値でライティング更新
  rgblight_value(sync_data.key_row , sync_data.key_col,true,scr);
  //右側のROW,COLを左側に変換したうえで同期
  sync_data.key_row = sync_data.key_row - 4;
  sync_data.key_col = 5 - sync_data.key_col;
  transaction_rpc_send(USER_SYNC_KEY_COUNTER, sizeof(sync_data), &sync_data);
}

///////////////////////////////////////////////////////////////////////////////
//マウス処理
///////////////////////////////////////////////////////////////////////////////
report_mouse_t pointing_device_task_user(report_mouse_t mouse_report){
  //マスターの場合のみ処理する。
  if(is_keyboard_master()){
    //マウスの動作量をball_tensionに加算
    ball_tension=qadd8(ball_tension,abs(mouse_report.x)+abs(mouse_report.y));

    bool layer2 = layer_state_is(2);
    bool layerscr = layer_state_is(SCR_layer);
    //layer1と2はマウスを操作することでBSDEL,矢印キーを入力するが、
    //ある程度のテンションを設けないと細かい操作ができないため、時間とテンションで
    //扱いやすくした。
    //また垂直動作した時間の後は400msec間不動作させることで、上下操作時に左右動作しないようにした。
    static uint16_t arrow_timer = 0;   //垂直動作時間
    static int8_t arrow_tension_v = 0; //垂直テンション
    static int8_t arrow_tension_h = 0; //水平テンション
    //layer1とlayer2の際の処理(layer2のみ変数にしているのはFWサイズ削減)
    if(layer_state_is(1) || layer2){
      //ball_tensionが30以下の場合はリターン
      if (ball_tension < 30){
        mouse_report.x = 0;
        mouse_report.y = 0;
        return mouse_report;
      }
      //マウス動作量がxとyどちらが大きいか判断
      if (abs(mouse_report.x) >= abs(mouse_report.y)){
        //xが大きい場合、動作量を水平テンションに追加
        arrow_tension_h += mouse_report.x;
        //垂直動作から400msec経過後
        if(timer_elapsed(arrow_timer) > 400){
          //水平テンションが規定値以上の場合、キーコードを出力
          //右方向処理
          if (arrow_tension_h > 9){
            if(layer2) {
              tap_code(KC_RIGHT);
            } else {
              tap_code(KC_DEL);
            }
            arrow_tension_h = 0; //動作した場合はテンションを0にする。
          //左方向処理
          } else if (arrow_tension_h < -9){
            if(layer2){
              tap_code(KC_LEFT);
            } else {
              tap_code(KC_BSPC);
            }
            arrow_tension_h = 0; //動作した場合はテンションを0にする。
          }
          //水平方向が動作した場合、垂直方向が動作しにくいように、テンション量を下げる。
          if (mouse_report.x != 0){
            arrow_tension_v += (arrow_tension_v > 0 ? -2 : 2);
          }
        }
      } else {
        //yが大きい場合、動作量を垂直テンションに追加
        arrow_tension_v += mouse_report.y;
        //垂直テンションが規定以上の場合、キーコードを出力
        //下方向(値は+側)
        if (arrow_tension_v > 12){
          if(layer2){
            tap_code(KC_DOWN);
          }
          arrow_timer = timer_read();
          arrow_tension_v =0;
        //上方向(値は-側)
        } else if (arrow_tension_v < -12){
          if(layer2){
            tap_code(KC_UP);
          }
          arrow_timer = timer_read();
          arrow_tension_v =0;
        }
      }
      mouse_report.x = 0;
      mouse_report.y = 0;
      return mouse_report;
    }

    //MouseMoveライティングの処理
    //AMLの場合にマウスの動作に合わせてキーのROWとCOLを変更
    if(layer_state_is(AML_layer)||layerscr){
      bool move = false;
      //ROWが左側だった場合、右側に変更(右側の情報を最後に左に変換し同期)
      if(sync_data.key_row < 4){
        sync_data.key_row = sync_data.key_row + 4;
        sync_data.key_col = 5 - sync_data.key_col;
      }
      arrow_tension_h += mouse_report.x / 2;
      arrow_tension_v += mouse_report.y / 2 - mouse_report.v * 4;
      //テンションが一定以上であれば上下左右のキー座標変更処理
      if(arrow_tension_h > 50){
        if(sync_data.key_col!=0){
          sync_data.key_col = sync_data.key_col - 1;
        }
        arrow_tension_h = 0;
        move = true;
      } else if (arrow_tension_h < -50){
        if(sync_data.key_col!=5){
          sync_data.key_col = sync_data.key_col + 1;
        }
        arrow_tension_h = 0;
        move = true;
      }
      if(arrow_tension_v > 50){
        if(sync_data.key_row!=6){
          sync_data.key_row = sync_data.key_row + 1;
        } else if (layerscr){
          sync_data.key_row = 4;
        }
        arrow_tension_v =0;
        move = true;
      } else if (arrow_tension_v < -50){
        if(sync_data.key_row!=4){
          sync_data.key_row = sync_data.key_row - 1;
        } else if (layerscr){
          sync_data.key_row = 6;
        }
        arrow_tension_v =0;
        move = true;
      }
      //キー座標が変更となっていた場合にライティング情報変更及び同期処理
      if(move){
        call_rgblight(layerscr);
      }
    }
    //AML実装,マウスが動作している場合
    if (mouse_report.x != 0 || mouse_report.y != 0){
      //ball_tensionが30以上の場合にAML ON
      if (ball_tension > 30 && !layer_state_is(3)){
        layer_on(AML_layer);
      }
      move_timer = timer_read(); //動作時間を保存
    //マウスが動作していない場合
    }  else  {
      //AMLリセットタイマーを超過した場合、AMLレイヤーOFF
      if (timer_elapsed(move_timer) > AML_reset_time && layer_state_is(3)){
        layer_off(AML_layer);
        ball_tension=0;
      }
      //少し動いた場合も800msecでball_tensionをリセット
      if (timer_elapsed(move_timer) > 800){
        ball_tension=0;
      }
    }
  }
  return mouse_report;
}


///////////////////////////////////////////////////////////////////////////////
//レイヤー変更処理
///////////////////////////////////////////////////////////////////////////////
layer_state_t layer_state_set_user(layer_state_t state)
{
  uint8_t layer = get_highest_layer(state);
  //レイヤー4の場合スクロールモードに変更
  keyball_set_scroll_mode(layer == SCR_layer);

  ball_tension=0;
  
#ifdef BALL //BALL側のみ実装
  static uint8_t old_layer;
  //レイヤー3のON/OFFに合わせてWin+F13を出力
  //Powertoysのマウス蛍光ペン機能でAMLのON/OFFを画面上で可視化可能
  if(layer==AML_layer && old_layer!=AML_layer){
    tap_code16(0x868);
  } else if(layer!=AML_layer && old_layer==AML_layer){
    tap_code16(0x868);  
  }
  //layer1/2はキーコード出力のため、CPIを1に固定しどんな状況でも同じ感度でキーコード出力
  if(layer==1 || layer==2){
    keyball_set_cpi(1);
  }
  //layer1/2から変更となった場合にCPIを復旧
  if( (old_layer==1 || old_layer==2) && 
      (layer    !=1 && layer    !=2) ){
    set_last_cpi();
  }
  old_layer = layer;
  //レイヤー毎のモード変更
  
  if(layer==1){
    ball_tension=0;
    //layer1,テンキーレイヤーを有効にするため、numlockもONにする。
    if (!host_keyboard_led_state().num_lock){
      tap_code(0x53);
    }
    rgblight_mode(RGBLIGHT_MODE_ICEWAVE);

  } else if (layer== 2){
    rgblight_mode(RGBLIGHT_MODE_STATIC);

  } else if (layer== AML_layer){
    rgblight_mode(RGBLIGHT_MODE_MOUSEMOVE);
        sync_data.key_row = 5;
        sync_data.key_col = 4;
        call_rgblight(false);

  } else if (layer== SCR_layer){
    rgblight_mode(RGBLIGHT_MODE_SCROLLMOVE);
        sync_data.key_row = 5;
        sync_data.key_col = 4;
        call_rgblight(true);
  } else {
    rgblight_mode(RGBLIGHT_MODE_HEATMAP);
  }

#else
  if(layer==1){
      ball_tension=0;
      if (!host_keyboard_led_state().num_lock){
        tap_code(0x53);
      }
  }
#endif

  return state;
}

///////////////////////////////////////////////////////////////////////////////
//コンボ設定
///////////////////////////////////////////////////////////////////////////////
enum combos { 
  WR_M0, 
  SF_M1, 
  XV_M2,
  MDOT_M3,
  JL_L0,
  M12_L0,
  XC_NUM_LOCK,
  SD_CAPS_WORD,
  WE_CAPS_LOCK,
  HK_F10,
  DG_F4,
  DF_muhenkan,
  JK_henkan,
  MCOMM_Grave,
  WINALT_PrtSC,
};

const uint16_t PROGMEM we_combo[] =   {KC_W,   KC_E,    COMBO_END};
const uint16_t PROGMEM wr_combo[] =   {KC_W,   KC_R,    COMBO_END};
const uint16_t PROGMEM sf_combo[] =   {KC_S,   0x2409 , COMBO_END}; //0x2409=Alt,f
const uint16_t PROGMEM xv_combo[] =   {KC_X,   0x2819,  COMBO_END}; //0x2819=Win,v
const uint16_t PROGMEM mdot_combo[] = {KC_M,   KC_DOT,  COMBO_END};
const uint16_t PROGMEM jl_combo[] =   {KC_J,   KC_L,    COMBO_END};
const uint16_t PROGMEM m12_combo[] =  {0xD1,   0xD2,    COMBO_END}; //D1,D2=mouse1/2
const uint16_t PROGMEM xc_combo[] =   {KC_X,   KC_C,    COMBO_END};
const uint16_t PROGMEM sd_combo[] =   {KC_S,   0x2207,  COMBO_END}; //0x2207=Shift,d
const uint16_t PROGMEM hk_combo[] =   {KC_H,   0x420E,  COMBO_END}; //0x420E=layer2,k
const uint16_t PROGMEM dg_combo[] =   {0x2207, KC_G,    COMBO_END}; //0x2207=Shift,d
const uint16_t PROGMEM df_combo[] =   {0x2207, 0x2409,  COMBO_END}; //0x2207=Shift,d
const uint16_t PROGMEM jk_combo[] =   {KC_J,   0x420E,  COMBO_END}; //0x420E=layer2,k
const uint16_t PROGMEM mcomm_combo[] ={KC_M,   KC_COMM, COMBO_END};
const uint16_t PROGMEM winal_combo[] ={0xE3,   0xE2,    COMBO_END}; //E3=win,E2=alt

combo_t key_combos[] = {
  [WR_M0]        = COMBO(wr_combo,    0x7700), //Macro0
  [SF_M1]        = COMBO(sf_combo,    0x7701), //Macro1
  [XV_M2]        = COMBO(xv_combo,    0x7702), //Macro2
  [MDOT_M3]      = COMBO(mdot_combo,  0x7703), //Macro3
  [JL_L0]        = COMBO(jl_combo,    0x5000), //LM0
  [M12_L0]       = COMBO(m12_combo,   0x5000), //LM0
  [XC_NUM_LOCK]  = COMBO(xc_combo,    0x53),   //Numlock
  [SD_CAPS_WORD] = COMBO(sd_combo,    0x7C73), //CapsWordToggle
  [WE_CAPS_LOCK] = COMBO(we_combo,    0x0239), //CapsLock
  [HK_F10]       = COMBO(hk_combo,    KC_F10),
  [DG_F4]        = COMBO(dg_combo,    KC_F4),
  [DF_muhenkan]  = COMBO(df_combo,    KC_INTERNATIONAL_5),
  [JK_henkan]    = COMBO(jk_combo,    KC_INTERNATIONAL_4),
  [MCOMM_Grave]  = COMBO(mcomm_combo, KC_GRAVE),
  [WINALT_PrtSC] = COMBO(winal_combo, KC_PRINT_SCREEN),
};


///////////////////////////////////////////////////////////////////////////////
//OLED
///////////////////////////////////////////////////////////////////////////////
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_270;
}

#ifdef BALL
  #include QMK_KEYBOARD_H

  //数値を文字列に変換します。指定桁数の右寄せ
  static const char *itoc(uint8_t number, uint8_t width) {
    static char str[5]; 
    uint8_t i = 0;
    width = width > 4 ? 4 : width;
    do {
      str[i++] = number % 10 + '0';
      number /= 10;
    } while (number != 0);

    while (i < width) {
        str[i++] = ' ';
    }

    int len = i;
    for (int j = 0; j < len / 2; j++) {
      char temp = str[j];
      str[j] = str[len - j - 1];
      str[len - j - 1] = temp;
    }

    str[i] = '\0';
    return str;
  }

  // Lockキー状態表示
  static void print_lock_key_status(void) {
      
    const led_t led_state = host_keyboard_led_state();

    oled_set_cursor(0, 0);
    oled_write(layer_state_is(4) ? ">>S<<" : "  S  ", false);
    oled_write(layer_state_is(3) ? ">>M<<" : "  M  ", false);
    oled_write(layer_state_is(2) ? ">>2<<" : "  2  ", false);
    oled_write(layer_state_is(1) ? ">>1<<" : "  1  ", false);
    oled_write(layer_state_is(0) ? ">>0<<" : "  0  ", false);
    
    oled_set_cursor(0, 6);
    oled_write("CPI", false);
    oled_write(itoc(keyball_get_cpi(), 2), false);
    oled_write("ROW", false);
    oled_write(itoc(sync_data.key_row, 2), false);
    oled_write("COL", false);
    oled_write(itoc(sync_data.key_col, 2), false);
    
    oled_set_cursor(0, 10);
    oled_write(led_state.caps_lock   ? "CAP @" : "CAP =", false);
    oled_write(is_caps_word_on()     ? "CWD @" : "CWD =", false);
    oled_write(rgblight_is_enabled() ? "LED @" : "LED =", false);
    
    oled_set_cursor(0, 14);
    oled_write("H ", false);
    oled_write(itoc(hue,3) , false);
    oled_write("V ", false);
    oled_write(itoc(rgblight_get_val(),3) , false);

  }

  void oledkit_render_info_user(void) {
      print_lock_key_status();
  }

#else
  #include "snow.c"
#endif

bool oled_task_user(void) {
  oledkit_render_info_user();
  return true;
}

///////////////////////////////////////////////////////////////////////////////
//KeyMAp
///////////////////////////////////////////////////////////////////////////////
// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  // keymap for default
  [0] = LAYOUT(
    KC_ESC , KC_Q   , KC_W   , KC_E   , KC_R   , KC_T   ,                      KC_Y   , KC_U   , KC_I   , KC_O   , KC_P   , KC_EQUAL,
    0x222B , KC_A   , KC_S   , 0x2207 , 0x2409 , KC_G   ,                      KC_H   , KC_J   , 0x420E , KC_L   , 0x442D , KC_ENTER,
    0x7E40 , 0x221D , KC_X   , KC_C   , 0x2819 , KC_B   ,                      KC_N   , KC_M   , KC_COMM, KC_DOT , KC_SLSH, 0x4287  ,
                      KC_LGUI, KC_LALT, 0x2150 , 0x412C , 0x424F ,     0x312A ,0x324C                            , 0X5261
  ),	

  [1] = LAYOUT(
    0X43D  , S(KC_1), S(KC_2), S(KC_3), S(KC_4), S(KC_5),                      0x22D  , KC_KP_7, KC_KP_8, KC_KP_9, 0x22E  , 0x2F   ,
    _______, S(KC_6), S(KC_7), 0X2233 , 0x2434 , KC_UP  ,                      0x57   , KC_KP_4, KC_KP_5, KC_KP_6, 0x4456 , _______,
    _______, 0x11D  , 0x11B  , 0x106  , 0x119  , KC_DOWN,                      0x55   , KC_KP_1, KC_KP_2, KC_KP_3, 0x54   , 0x289  ,
                      _______, _______, _______, _______, _______,    0x2163 , 0x3262                            , _______
  ),

  [2] = LAYOUT(
    _______,KC_F9   , KC_F10 , KC_F11 , KC_F12 , 0x32   ,                      RGB_TOG, 0x7E41 , KC_UP  , KC_NO  , KC_NO  , _______,
    0x868  ,KC_F5   , KC_F6  , 0x2240 , KC_F8  , 0x4B   ,                      RGB_VAI, KC_LEFT, KC_DOWN,KC_RIGHT, KC_NO  , _______,
    _______,KC_F1   , KC_F2  , KC_F3  , KC_F4  , 0x4E   ,                      RGB_VAD, KC_NO  , KC_NO  , KC_NO  , 0x7C01 , _______,
                      QK_BOOT, 0x7705 , _______, _______, _______,    _______, _______                           , QK_BOOT
  ),

  [3] = LAYOUT(
    _______, _______, _______, _______, _______, _______,                      _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______,                      _______, 0xD1   , _______, 0xD2   , 0x5080, _______,
    _______, _______, _______, _______, _______, _______,                      _______, _______, _______, _______, _______, _______,
                      _______, _______, _______, _______, _______,    _______, _______                           , _______
  ),

  [4] = LAYOUT(
    _______, _______, 0x950  , 0x852  , 0x94F  , _______,                      QK_KB_2, 0x424  , _______, 0x425  , _______, _______,
    _______, 0xA50  , 0x850  , 0x851  , 0x84F  , 0xA4F  ,                      0x326  , 0xD1   , _______, 0xD2   , _______, _______,
    _______, _______, 0x32D  , _______, 0x333  , _______,                      QK_KB_3, _______, _______, _______, _______, _______,
                      _______, _______, _______, _______, _______,    0xD4   , 0xD5                              , QK_KB_1
  ),
};
// clang-format on

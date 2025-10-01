#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// OLEDのサイズを定義（120ピクセルの高さ、32ピクセルの幅）
#define OLED_SNOW_HEIGHT 120
#define OLED_SNOW_WIDTH  32

// ピクセルの状態を管理するバイト数（OLEDのピクセル数 / 8）
#define OLED_SNOW_BYTES (OLED_SNOW_HEIGHT * OLED_SNOW_WIDTH / 8)

// 1ピクセルの描画・削除を行う関数
static inline void setPixel(char* pixels, uint8_t h, uint8_t w, bool pix) {
    uint16_t byteIdx = w + (h / 8) * OLED_SNOW_WIDTH; // ピクセル位置に対応するバイトインデックス
    int8_t byteMask = 1 << (h % 8);                    // ビットマスク（行方向の位置）
    if (pix) {
        pixels[byteIdx] |= byteMask;                   // ピクセルをONにする
    } else {
        pixels[byteIdx] &= ~byteMask;                  // ピクセルをOFFにする
    }
}

// 指定ピクセルがONかOFFかを取得する関数
static inline bool getPixel(char* pixels, uint8_t h, uint8_t w) {
    uint16_t byteIdx = w + (h / 8) * OLED_SNOW_WIDTH;
    int8_t byteMask = 1 << (h % 8);
    return (pixels[byteIdx] & byteMask) != 0; // ピクセル状態を取得
}

// 非常にシンプルな乱数生成関数
static uint8_t rand_basic(void) {
    static uint8_t seed = 0;
    seed = 89 * seed + 79; // 定数による次の値の生成
    return seed;
}

// 垂直方向の各行で粒が存在する横位置（列）を保持
typedef signed char lineIdx_t;
static lineIdx_t* active_snow = NULL;  // 各行における粒の横座標（なければ-1）
static char* pixels = NULL;            // OLEDに描画する全体ピクセルバッファ

// メモリ解放関数
static void free_memory(void) {
    if (pixels != NULL) {
        free(pixels);
        pixels = NULL;
    }
    if (active_snow != NULL) {
        free(active_snow);
        active_snow = NULL;
    }
}

// フォントデータ
const uint8_t font[] PROGMEM = {
    0x00, 0x00, 0x00,
    0x12, 0x25, 0x25, 0x29, 0x12,
    0x00, 0x00, 
    0x3F, 0x02, 0x0C, 0x10, 0x3F,
    0x00, 0x00,
    0x1E, 0x21, 0x21, 0x21, 0x1E,
    0x00, 0x00,
    0x0F, 0x30, 0x0C, 0x30, 0x0F,
    0x00, 0x00, 0x00,
};

// 新しい粒を追加する関数
static void add_new_snow(void) {
    static uint8_t snowCounter = 0; // 粒の生成制御用のカウンタ

    // 新しい粒の追加タイミングかチェック
    if (snowCounter != keyCounter) {
        snowCounter++;

        // ランダムな横位置に新しい粒を追加
        lineIdx_t w = rand_basic() % OLED_SNOW_WIDTH;
        bool full = false;

        // 上から順に空いている位置を探す（中央を超えたら満杯と判断）
        while (getPixel(pixels, 0, w)) {
            if (w == 0) {
                w = OLED_SNOW_WIDTH - 1;
            } else if (w == OLED_SNOW_WIDTH / 2) {
                full = true;
                break;
            } else {
                w--;
            }
        }

        if (!full) {
            // 新しい粒を最上段に追加
            setPixel(pixels, 0, w, true);
            active_snow[0] = w;
        } else {
            free_memory();
        }
    }
}

// 粒を下に移動させる処理
static void move_snow_down(void) {
    for (int8_t h = OLED_SNOW_HEIGHT - 2; h >= 0; h--) {
        lineIdx_t w = active_snow[h]; // 現在の行の粒の横位置

        if (w < 0 || w >= OLED_SNOW_WIDTH) {
            continue; // 範囲外または粒なし
        }

        lineIdx_t wn = -1; // 次に移動する横位置（初期値：未定）
        int8_t dx = (rand_basic() % 3) - 1; // -1, 0, 1 のいずれか（横の揺れ）

        if ((dx == -1 && w > 0) || (dx == 1 && w < OLED_SNOW_WIDTH - 1)) {
            if (!getPixel(pixels, h + 1, w + dx)) {
                wn = w + dx;
            }
        }

        // どこにも落とせなかったら真下をもう一度チェック
        if (wn == -1) {
            if (!getPixel(pixels, h + 1, w)) {
                wn = w;
            }
        }

        // 移動先が決まったらピクセルを移動させる
        if (wn != -1) {
            setPixel(pixels, h + 1, wn, true); // 新しい位置に描画
            active_snow[h + 1] = wn; // 新しい位置に記録
            setPixel(pixels, h, w, false); // 元の位置を消す
        }

        // 現在の行からは粒がなくなる
        active_snow[h] = -1;
    }
}

// メインの粒アニメーション関数
void oled_snow(void) {
    // active_snowが未初期化なら確保＆初期化
    if (active_snow == NULL) {
        active_snow = malloc(OLED_SNOW_HEIGHT);
        memset(active_snow, -1, OLED_SNOW_HEIGHT); // すべて「粒なし（-1）」に設定
    }

    // ピクセルバッファの確保と初期化（全ピクセルOFF）
    if (pixels == NULL) {
        pixels = malloc(OLED_SNOW_BYTES);
        memset(pixels, 0, OLED_SNOW_BYTES);
    }

    // 粒を下へ落とす処理
    move_snow_down();

    // 新しい粒の追加
    add_new_snow();

    // ピクセルバッファをOLEDに描画
    oled_set_cursor(0, 1);
    oled_write_raw(pixels, OLED_SNOW_BYTES);
    oled_set_cursor(0, 0);
    oled_write_raw_P((const char*)font, sizeof(font));
}

// QMKで呼ばれるユーザー描画関数の実装（ここで粒を降らせてる）
void oledkit_render_info_user(void) {
    oled_snow();
}

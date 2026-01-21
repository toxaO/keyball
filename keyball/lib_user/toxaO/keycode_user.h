// keyball39
// mymap2.0
#pragma once

#include "quantum.h"
#include "lib/keyball/keyball.h"  // キーボードレベルのキーコード（MULTI_*, *_SW など）

//------------------------------------------------------------
// Layer
//------------------------------------------------------------
enum layer_names {
    _Def = 0,
    _NumP,
    _Pad,
    _Sym,
    _Cur,
    _Mou,
    _Scr,
};

#define _Sym_SPC   LT(_Sym, KC_SPC)
#define _Mou_SCLN  LT(_Mou, KC_SCLN)
#define _Cur_ENT   LT(_Cur, KC_ENT)

//------------------------------------------------------------
// カスタムキーコードの分担とSAFE_RANGEの使い方
//------------------------------------------------------------
// - KBレベル（QK_KB_*）は keyball.h 側で定義。Vial の customKeycodes は QK_KB_0.. の順で送出。
// - ユーザー側は「KEYBALL_SAFE_RANGE から連番」で定義する。
//   例: enum custom_keycodes { USER_A = KEYBALL_SAFE_RANGE, USER_B, ... };
// - SAFE_RANGE は keyball.h の列挙末尾に置かれ、KBキーの並び順次第で動的に決まる。
//   KB側で並び替えた場合は、vial.json の customKeycodes の並び（インデックス）も同じ順序へ更新して整合を取ること。
// - vialの仕様上、vial.jsonに記載して表示可能なのはQK_KB_31までかも。

//------------------------------------------------------------
// Custom Keycode
//------------------------------------------------------------
enum custom_keycodes {
    EISU_S_N = KEYBALL_SAFE_RANGE, // == 例:QK_KB_20であれば、以下はそこから順番のキーコードになる。
    KANA_C_N,
    ESC_LNG2,
    TG_PA_GU,

    // フリック用カスタムキーコード
    FLICK_A,
    FLICK_D,
    FLICK_G,
    FLICK_J,
    FLICK_M,
    FLICK_P,
    FLICK_T,
    FLICK_W,
};

//------------------------------------------------------------
// mod tap
//------------------------------------------------------------
#define MINUS_G    MT(MOD_LGUI, KC_MINUS)
#define KANA_C     MT(MOD_LCTL, KC_LNG1)
#define EISU_S     MT(MOD_LSFT, KC_LNG2)
#define CLN_C      MT(MOD_LCTL, CLN)
#define MINUS_S    MT(MOD_LSFT, KC_MINUS)
#define NumP_A     MT(MOD_LALT, KC_NO)
#define BSPC_G     MT(MOD_LGUI, KC_BSPC)
#define COMM_G     GUI_T(KC_COMM)
#define DOT_A      ALT_T(KC_DOT)
#define SLSH_S     SFT_T(KC_SLSH)

//------------------------------------------------------------
// mod + kc *JIS
//------------------------------------------------------------
#define MINUS      KC_MINUS       // "-"
#define EXCLAIM    LSFT(KC_1)     // "!"
#define D_QUOTE    LSFT(KC_2)     // """
#define HASH       LSFT(KC_3)     // "#"
#define DOLLAR     LSFT(KC_4)     // "$"
#define PERCENT    LSFT(KC_5)     // "%"
#define AND        LSFT(KC_6)     // "&"
#define QUOTE      LSFT(KC_7)     // "'"
#define L_PAREN    LSFT(KC_8)     // "("
#define R_PAREN    LSFT(KC_9)     // ")"
#define L_SQBR     KC_RBRC        // "["
#define R_SQBR     KC_BSLS        // "]"
#define L_BRC      LSFT(KC_RBRC)  // "{"
#define R_BRC      LSFT(KC_BSLS)  // "}"
#define LESS       LSFT(KC_COMM)  // "<"
#define MORE       LSFT(KC_DOT)   // ">"
#define BSLSH      LALT(KC_INT3)  // "\"
#define EQL        LSFT(KC_MINUS) // "="
#define TILDE      LSFT(KC_EQL)   // "~"
#define U_BAR      LSFT(KC_INT1)  // "_"
#define CARET      KC_EQL         // "^"
#define YEN        KC_INT3        // "¥"
#define V_BAR      LSFT(KC_INT3)  // "|"
#define AT         KC_LBRC        //"@"
#define CLN        KC_QUOTE       //":"
#define QUESTION   LSFT(KC_SLSH)  // "?"
#define PLUS       LSFT(KC_SCLN)  // "+"
#define CLN        KC_QUOTE       // ":"
#define AST        LSFT(KC_QUOTE) // "*"
#define B_QUO      LSFT(AT)       // "`"

//------------------------------------------------------------
// ※macOS karabinerでRCTLをFnにしていること前提
// RALT modでwinならLCTL、mac(ios)ならLGUI mod
//------------------------------------------------------------
#define COPY       RALT(KC_C)
#define PASTE      RALT(KC_V)
#define CUT        RALT(KC_X)
#define DELETE     RALT(KC_BSPC)
#define ALL        RALT(KC_A)
#define CLOSE      RALT(KC_W)
#define NEW_TAB    RALT(KC_T)
#define LAST_TAB   RALT(S(KC_T))
#define UNDO       RALT(KC_Z)
#define REDO       RALT(KC_Y)
#define QUIT       RALT(KC_Q)
#define FIND       RALT(KC_F)
#define SAVE       RALT(KC_S)
#define RELOAD     RALT(KC_R)
#define TAB_R      RALT(KC_TAB)
#define TAB_L      RALT(S(KC_TAB))
// home end
#define HOME       RALT(KC_LEFT)
#define END        RALT(KC_RIGHT)
#define PGDN       RALT(KC_DOWN)
#define PGUP       RALT(KC_UP)
// function key
#define F1         RALT(KC_F1)
#define F2         RALT(KC_F2)
#define F3         RALT(KC_F3)
#define F4         RALT(KC_F4)
#define F5         RALT(KC_F5)
#define F6         RALT(KC_F6)
#define F7         RALT(KC_F7)
#define F8         RALT(KC_F8)
#define F9         RALT(KC_F9)
#define F10        RALT(KC_F10)
#define F11        RALT(KC_F11)
#define F12        RALT(KC_F12)
#define F13        RALT(KC_F13)
#define F14        RALT(KC_F14)
#define F15        RALT(KC_F15)
#define F16        RALT(KC_F16)
#define F17        RALT(KC_F17)
#define F18        RALT(KC_F18)
#define F19        RALT(KC_F19)
#define F20        RALT(KC_F20)


//----------------------------------------
// mac
//----------------------------------------
#define m_MIS_CON  LCTL(KC_UP)
#define m_L_DESK   LCTL(KC_LEFT)
#define m_R_DESK   LCTL(KC_RIGHT)
// magnet
#define MGN_L      C(A(KC_LEFT))
#define MGN_R      C(A(KC_RIGHT))
#define MGN_U      C(A(KC_UP))
#define MGN_D      C(A(KC_DOWN))
#define MGN_RU     C(A(KC_I))
#define MGN_LU     C(A(KC_U))
#define MGN_RD     C(A(KC_K))
#define MGN_LD     C(A(KC_J))
#define MGN_LW     C(A(KC_T))
#define MGN_LN     C(A(KC_G))
#define MGN_RW     C(A(KC_E))
#define MGN_RN     C(A(KC_D))
#define MGN_CN     C(A(KC_R))
#define MGN_MAX    C(A(KC_ENT))
#define MGN_REC    C(A(KC_BSPC))

//----------------------------------------
// win
//----------------------------------------
#define w_Ueli     G(KC_1)
#define w_TASK     C(KC_TAB)
#define w_R_DESK   G(C(KC_RIGHT))
#define w_L_DESK   G(C(KC_LEFT))
#define w_DESK     G(KC_D)

//----------------------------------------
// simple KC
//----------------------------------------
#define XXXXX      KC_NO
#define XXX        KC_NO
#define ___        _______


//----------------------------------------
// One Shot
//----------------------------------------
#define OS_LSFT    OSM(MOD_LSFT)
#define OS_LCTL    OSM(MOD_LCTL)
#define OS_LGUI    OSM(MOD_LGUI)
#define OS_LALT    OSM(MOD_LALT)
#define OS_RALT    OSM(MOD_RALT)

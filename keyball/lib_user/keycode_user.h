// keyball39
// mymap2.0
#pragma once

#include "quantum.h"
/*
#HSV_COLOR_CODE
------------------------------
HSV_AZURE 青み
HSV_BLACK/HSV_OFF
HSV_BLUE
HSV_CHARTREUSE 淡い黄緑
HSV_CORAL ピンクみ
HSV_CYAN
HSV_GOLD
HSV_GOLDENROD 黄土
HSV_GREEN
HSV_MAGENTA
HSV_ORANGE
HSV_PINK
HSV_PURPLE
HSV_RED
HSV_SPRINGGREEN 鮮やかな黄緑
HSV_TEAL 青緑
HSV_TURQUOISE ターコイズ
HSV_WHITE
HSV_YELLOW
------------------------------
*/


// simple KC
#define XXXXX KC_NO
#define XXX KC_NO

// layer
#define _cDef_KANA LT(_cDef, KC_LNG1)
#define _Sym_SPC LT(_Sym, KC_SPC)
#define _MO_SCLN LT(_Mou, KC_SCLN)
#define _NumP_SCLN LT(_NumP, KC_SCLN)
#define _Cur_ENT LT(0, KC_ENT)
#define _Win_SLSH LT(_Win, KC_SLSH)
#define _cDef_MI LT(_cDef, KC_MINUS)
#define _NumP_ESC LT(_NumP, KC_ESC)
#define _Mou_NumP LT(_Mou, KC_NO)
#define _Mou_SCLN LT(0, KC_SCLN)
#define _Esc_NumP LT(0, Esc_NumP)
#define _MIN_NumP LT(_NumP, KC_MINUS)

// mod + kc *JIS
#define MINUS KC_MINUS // "-"
#define EXCLAIM LSFT(KC_1) // "!"
#define D_QUOTE LSFT(KC_2) // """
#define HASH LSFT(KC_3) // "#"
#define DOLLAR LSFT(KC_4) // "$"
#define PERCENT LSFT(KC_5) // "%"
#define AND LSFT(KC_6) // "&"
#define QUOTE LSFT(KC_7) // "'"
#define L_PAREN LSFT(KC_8) // "("
#define R_PAREN LSFT(KC_9) // ")"
#define L_SQBR KC_RBRC // "["
#define R_SQBR KC_BSLS // "]"
#define L_BRC LSFT(KC_RBRC) // "{"
#define R_BRC LSFT(KC_BSLS) // "}"
#define LESS LSFT(KC_COMM) // "<"
#define MORE LSFT(KC_DOT) // ">"
#define BSLSH LALT(KC_INT3) // "\"
#define EQL LSFT(KC_MINUS) // "="
#define TILDE LSFT(KC_EQL) // "~"
#define U_BAR LSFT(KC_INT1) // "_"
#define CARET KC_EQL // "^"
#define YEN KC_INT3 // "¥"
#define V_BAR LSFT(KC_INT3) // "|"
#define AT KC_LBRC //"@"
#define CLN KC_QUOTE //":"
#define QUESTION LSFT(KC_SLSH) // "?"
#define PLUS LSFT(KC_SCLN) // "+"
#define CLN KC_QUOTE // ":"
#define AST LSFT(KC_QUOTE) // "*"
#define B_QUO LSFT(AT) // "`"

// macOS karabinerでRCTLをFnにしていること前提
#define M_F1 RCTL(KC_F1)
#define M_F2 RCTL(KC_F2)
#define M_F3 RCTL(KC_F3)
#define M_F4 RCTL(KC_F4)
#define M_F5 RCTL(KC_F5)
#define M_F6 RCTL(KC_F6)
#define M_F7 RCTL(KC_F7)
#define M_F8 RCTL(KC_F8)
#define M_F9 RCTL(KC_F9)
#define M_F10 RCTL(KC_F10)
#define M_F11 RCTL(KC_F11)
#define M_F12 RCTL(KC_F12)
#define M_F13 RCTL(KC_F13)
#define M_F14 RCTL(KC_F13)
#define M_F15 RCTL(KC_F13)

// short cut command
// common
#define TAB_R LCTL(KC_TAB)
#define TAB_L C(S(KC_TAB))

// mac
#define m_COPY LGUI(KC_C)
#define m_PASTE LGUI(KC_V)
#define m_CUT LGUI(KC_X)
#define m_DELETE LGUI(KC_BSPC)
#define m_ALL LGUI(KC_A)
#define m_CLOSE LGUI(KC_W)
#define m_NEW_TAB LGUI(KC_T)
#define m_UNDO LGUI(KC_Z)
#define m_REDO LGUI(LSFT(KC_Z))
#define m_QUIT LGUI(KC_Q)
#define m_FIND LGUI(KC_F)
#define m_SAVE LGUI(KC_S)
#define m_MEMO RCTL(KC_Q)
#define m_MIS_CON LCTL(KC_UP)
#define m_APP_CON LCTL(KC_DOWN)
#define m_BACK LGUI(KC_LEFT)
#define m_FORWARD LGUI(KC_RIGHT)
#define m_L_DESK LCTL(KC_LEFT)
#define m_R_DESK LCTL(KC_RIGHT)
#define m_LAST_TAB G(S(KC_T))
#define m_SLEEP A(G(KC_Q))
#define m_LAUNCH LGUI(KC_SPC)
#define m_RELOAD LGUI(KC_R)
#define m_SCSH_3 LSG(KC_3)
#define m_SCSH_4 LSG(KC_4)
#define m_SCSH_5 LSG(KC_5)

// win
#define w_COPY C(KC_C)
#define w_PASTE C(KC_V)
#define w_CUT C(KC_X)
#define w_ALL C(KC_A)
#define w_CLOSE C(KC_W)
#define w_NEW_TAB C(KC_T)
#define w_LAST_TAB S(C(KC_T))
#define w_UNDO C(KC_Z)
#define w_REDO C(KC_Y)
#define w_RELOAD C(KC_R)
#define w_QUIT A(KC_F4)
#define w_FIND C(KC_F)
#define w_SAVE C(KC_S)
#define w_TASK C(KC_TAB)
#define w_R_DESK G(C(KC_RIGHT))
#define w_L_DESK G(C(KC_LEFT))
#define w_DESK G(KC_D)
#define w_Ueli G(KC_1)

// mod tap
#define MINUS_G MT(MOD_LGUI, KC_MINUS)
#define KANA_C MT(MOD_LCTL, KC_LNG1)
#define EISU_S MT(MOD_LSFT, KC_LNG2)
#define CLN_C MT(MOD_LCTL, CLN)
#define MINUS_S MT(MOD_LSFT, KC_MINUS)
#define NumP_A MT(MOD_LALT, KC_NO)
#define BSPC_G MT(MOD_LGUI, KC_BSPC)

// mousekey
#define L_CLICK KC_BTN1
#define R_CLICK KC_BTN2
#define M_CLICK KC_BTN3


// home end
// karabiner前提
#define m_HOME RCTL(KC_LEFT)
#define m_END RCTL(KC_RIGHT)
#define m_PGDN RCTL(KC_DOWN)
#define m_PGUP RCTL(KC_UP)

// magnet
#define MGN_L C(A(KC_LEFT))
#define MGN_R C(A(KC_RIGHT))
#define MGN_U C(A(KC_UP))
#define MGN_D C(A(KC_DOWN))
#define MGN_RU C(A(KC_I))
#define MGN_LU C(A(KC_U))
#define MGN_RD C(A(KC_K))
#define MGN_LD C(A(KC_J))
#define MGN_LW C(A(KC_T))
#define MGN_LN C(A(KC_G))
#define MGN_RW C(A(KC_E))
#define MGN_RN C(A(KC_D))
#define MGN_CN C(A(KC_R))
#define MGN_MAX C(A(KC_ENT))
#define MGN_REC C(A(KC_BSPC))


// layer_names------------------------------
enum layer_names {
    _Def = 0,
    _Sym,
    _mCur,
    _wCur,
    _NumP,
    _SW_Block,
    _mMou,
    _wMou,
    _Scr,
    _Pad,
    _Set,
};

// custom keycode --------------------
enum custom_keycodes {
    /* APP_SW = KEYBALL_SAFE_RANGE, */
    APP_SW = QK_USER,
    VOL_SW,
    BRO_SW,
    TAB_SW,
    WIN_SW,
    MULTI_A,
    MULTI_B,
    EISU_S_N,
    KANA_C_N,
};

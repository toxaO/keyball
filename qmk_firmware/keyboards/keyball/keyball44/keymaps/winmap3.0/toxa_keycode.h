#pragma once

/*
#HSV_COLOR_CODE
------------------------------
HSV_AZURE
HSV_BLACK/HSV_OFF
HSV_BLUE
HSV_CHARTREUSE
HSV_CORAL
HSV_CYAN
HSV_GOLD
HSV_GOLDENROD
HSV_GREEN
HSV_MAGENTA
HSV_ORANGE
HSV_PINK
HSV_PURPLE
HSV_RED
HSV_SPRINGGREEN
HSV_TEAL
HSV_TURQUOISE
HSV_WHITE
HSV_YELLOW
------------------------------
*/


// simple KC
#define XXXXX KC_NO
#define XXX KC_NO

// mod + kc
#define EXCLAIM LSFR(KC_1) // "!"
#define D_QUOTE LSFT(KC_2) // """
#define SHARP LSFT(KC_3) // "#"
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
#define UNDERBAR KC_INT1 // "_"
#define W_UNDER S(KC_INT1)
#define CARET KC_EQL // "^"
#define YEN KC_INT3 // "¥"
#define PIPE LSFT(KC_INT3) // "|"
#define AT KC_LBRC //"@"
#define CLN KC_QUOTE //":"
#define QUESTION LSFT(KC_SLSH) // "?"
#define PLUS LSFT(KC_SCLN) // "+"
#define AST LSFT(KC_QUOTE) // "*"
#define BACK_QUO LSFT(AT) // "`"

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

// short cut command
#define COPY LGUI(KC_C)
#define PASTE LGUI(KC_V)
#define CUT LGUI(KC_X)
#define DELETE LGUI(KC_BSPC)
#define S_ALL LGUI(KC_A)
#define CLOSE LGUI(KC_W)
#define NEW_TAB LGUI(KC_T)
#define UNDO LGUI(KC_Z)
#define REDO LGUI(LSFT(KC_Z))
#define QUIT LGUI(KC_Q)
#define FIND LGUI(KC_F)
#define SAVE LGUI(KC_S)
#define MEMO RCTL(KC_Q)
#define MIS_CON LCTL(KC_UP)
#define APP_CON LCTL(KC_DOWN)
#define BACK LGUI(KC_LEFT)
#define FORWARD LGUI(KC_RIGHT)
#define DESK_L LCTL(KC_LEFT)
#define DESK_R LCTL(KC_RIGHT)
#define W_DESK_L G(C(KC_LEFT))
#define W_DESK_R G(C(KC_RIGHT))
#define TAB_R LCTL(KC_TAB)
#define TAB_L C(S(KC_TAB))
#define LAST_TAB G(S(KC_T))
#define SLEEP A(G(KC_Q))
#define ZOOM_IN LGUI(LSFT(KC_SCLN))
#define ZOOM_OUT LGUI(KC_MINUS)
#define SPOT_L LGUI(KC_SPC)
#define RELOAD LGUI(KC_R)
#define SCSH_3 LSG(KC_3)
#define SCSH_4 LSG(KC_4)
#define SCSH_5 LSG(KC_5)
#define EJECT_SSD G(A(BSLSH)) // eject backup SSD



// mod tap
#define KANA_C MT(MOD_LCTL, KC_LNG1)
#define EISU_S MT(MOD_LSFT, KC_LNG2)
#define TAB_C MT(MOD_LCTL, KC_TAB)
#define SLSH_GUI MT(MOD_LGUI, KC_SLSH)
#define AT_A LALT_T((AT))
#define CLN_C MT(MOD_LCTL, CLN)
#define MINUS_S MT(MOD_LSFT, KC_MINUS)
#define ESC_A LALT_T(KC_ESC)
#define UNDERB_S LSFT_T(UNDERBAR)
#define Z_GUI MT(MOD_LGUI, KC_Z)
#define CHAPP_AC MT(MOD_LCTL | MOD_LALT, KC_NO)
#define CHWIN_Fn MT(MOD_RCTL, KC_NO)


// mousekey
#define L_CLICK KC_BTN1
#define R_CLICK KC_BTN2
#define M_CLICK KC_BTN3


// home end
#define HOME RCTL(KC_LEFT)
#define END RCTL(KC_RIGHT)

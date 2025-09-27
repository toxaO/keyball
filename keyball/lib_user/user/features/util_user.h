#ifndef MYLIB_UTIL_H
#define MYLIB_UTIL_H


extern int host_os;

// int16_t my_abs(int16_t num);

// OS依存送出はKBレベルの tap_code16_os() を使用します（keyball.h）。

void tap_code16_with_oneshot(uint16_t keycode);

// void reset_eeprom(void);

#endif // UTIL_H

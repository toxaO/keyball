#ifndef MYLIB_UTIL_H
#define MYLIB_UTIL_H


extern int host_os;

int16_t my_abs(int16_t num);

void tap_code16_os(
    uint16_t win,
    uint16_t mac,
    uint16_t ios,
    uint16_t linux,
    uint16_t unsure);

#endif // UTIL_H

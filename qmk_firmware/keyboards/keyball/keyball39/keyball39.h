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

#pragma once

#include "quantum.h"
#include "lib/keyball/keyball.h"

// clang-format off

#define LAYOUT_right_ball( \
    L00, L01, L02, L03, L04,              R04, R03, R02, R01, R00, \
    L10, L11, L12, L13, L14,              R14, R13, R12, R11, R10, \
    L20, L21, L22, L23, L24,              R24, R23, R22, R21, R20, \
    L30, L31, L32, L33, L34, L35,    R35, R34,                R30  \
    ) \
    { \
        {   L00,   L01,   L02,   L03,   L04, KC_NO }, \
        {   L10,   L11,   L12,   L13,   L14, KC_NO }, \
        {   L20,   L21,   L22,   L23,   L24, KC_NO }, \
        {   L30,   L31,   L32,   L33,   L34,   L35 }, \
        {   R00,   R01,   R02,   R03,   R04, KC_NO }, \
        {   R10,   R11,   R12,   R13,   R14, KC_NO }, \
        {   R20,   R21,   R22,   R23,   R24, KC_NO }, \
        {   R30, KC_NO, KC_NO, KC_NO,   R34,   R35 }, \
    }

#define LAYOUT_left_ball( \
    L00, L01, L02, L03, L04,              R04, R03, R02, R01, R00, \
    L10, L11, L12, L13, L14,              R14, R13, R12, R11, R10, \
    L20, L21, L22, L23, L24,              R24, R23, R22, R21, R20, \
    L30,                L34, L35,    R35, R34, R33, R32, R31, R30  \
    ) \
    { \
        {   L00,   L01,   L02,   L03,   L04, KC_NO }, \
        {   L10,   L11,   L12,   L13,   L14, KC_NO }, \
        {   L20,   L21,   L22,   L23,   L24, KC_NO }, \
        {   L30, KC_NO, KC_NO, KC_NO,   L34,   L35 }, \
        {   R00,   R01,   R02,   R03,   R04, KC_NO }, \
        {   R10,   R11,   R12,   R13,   R14, KC_NO }, \
        {   R20,   R21,   R22,   R23,   R24, KC_NO }, \
        {   R30,   R31,   R32,   R33,   R34,   R35 }, \
    }

#define LAYOUT_dual_ball( \
    L00, L01, L02, L03, L04,              R04, R03, R02, R01, R00, \
    L10, L11, L12, L13, L14,              R14, R13, R12, R11, R10, \
    L20, L21, L22, L23, L24,              R24, R23, R22, R21, R20, \
    L30,                L34, L35,    R35, R34,                R30  \
    ) \
    { \
        {   L00,   L01,   L02,   L03,   L04, KC_NO }, \
        {   L10,   L11,   L12,   L13,   L14, KC_NO }, \
        {   L20,   L21,   L22,   L23,   L24, KC_NO }, \
        {   L30, KC_NO, KC_NO, KC_NO,   L34,   L35 }, \
        {   R00,   R01,   R02,   R03,   R04, KC_NO }, \
        {   R10,   R11,   R12,   R13,   R14, KC_NO }, \
        {   R20,   R21,   R22,   R23,   R24, KC_NO }, \
        {   R30, KC_NO, KC_NO, KC_NO,   R34,   R35 }, \
    }

#define LAYOUT_no_ball( \
    L00, L01, L02, L03, L04,              R04, R03, R02, R01, R00, \
    L10, L11, L12, L13, L14,              R14, R13, R12, R11, R10, \
    L20, L21, L22, L23, L24,              R24, R23, R22, R21, R20, \
    L30, L31, L32, L33, L34, L35,    R35, R34, R33, R32, R31, R30  \
    ) \
    { \
        {   L00,   L01,   L02,   L03,   L04, KC_NO }, \
        {   L10,   L11,   L12,   L13,   L14, KC_NO }, \
        {   L20,   L21,   L22,   L23,   L24, KC_NO }, \
        {   L30,   L31,   L32,   L33,   L34,   L35 }, \
        {   R00,   R01,   R02,   R03,   R04, KC_NO }, \
        {   R10,   R11,   R12,   R13,   R14, KC_NO }, \
        {   R20,   R21,   R22,   R23,   R24, KC_NO }, \
        {   R30,   R31,   R32,   R33,   R34,   R35 }, \
    }

#define LED_LAYOUT( \
    L00,  L01,  L02,  L03,  L04,              R04,  R03,  R02,  R01,  R00, \
    L10,  L11,  L12,  L13,  L14,              R14,  R13,  R12,  R11,  R10, \
    L20,  L21,  L22,  L23,  L24,              R24,  R23,  R22,  R21,  R20, \
    L30,  L31,  L32,                                                  R30, \
   UL00,       UL01,                                     UR01,       UR00, \
   UL10,  UL11,      UL12, UL13,       UR13, UR12,       UR11,       UR10) \
   { \
   L04, L14, L24, \
   L03, L13, L23, \
   L02, L12, L22, L32, \
   L01, L11, L21, L31, \
   L00, L10, L20, L30, \
   UL01, UL00, UL10, UL11, UL12, UL13, \
   \
   UR13, UR12, UR11, UR10, UR00, UR01, \
   R00, R10, R20, R30, \
   R01, R11, R21, \
   R02, R12, R22, \
   R03, R13, R23, \
   R04, R14, R24 }

#define RGBLIGHT_LED_MAP LED_LAYOUT( \
    0,  1,  2,  3,  4,     24, 25, 26, 27, 28, \
    5,  6,  7,  8,  9,     29, 30, 31, 32, 33, \
   10, 11, 12, 13, 14,     34, 35, 36, 37, 38, \
   15, 16, 17,                             39, \
   18,     19,                 40,         41, \
   19, 21,     22, 23,     42, 43,     44, 45  \
   )

// clang-format on

#define LAYOUT LAYOUT_right_ball
#define LAYOUT_universal LAYOUT_no_ball

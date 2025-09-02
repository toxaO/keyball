/*
Copyright 2019 @foostan
Copyright 2020 Drashna Jaelre <@drashna>

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

/* Select hand configuration */

#if !defined(MASTER_RIGHT) && !defined(MASTER_LEFT)
    #error "You must define either MASTER_LEFT or MASTER_RIGHT in your config.h"
    #define MASTER_RIGHT
#endif

#define TAPPING_TERM_PER_KEY

#define TRI_LAYER_LOWER_LAYER 2
#define TRI_LAYER_UPPER_LAYER 3
#define TRI_LAYER_ADJUST_LAYER 4

#define HK_MAIN_DEFAULT_POINTER_DEFAULT_MULTIPLIER 3.5
#define HK_PERIPHERAL_DEFAULT_POINTER_DEFAULT_MULTIPLIER 1

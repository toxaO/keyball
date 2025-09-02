#pragma once

#include "pointing.h"

const char* hk_scroll_mode_to_string(hk_scroll_lock mode);
const char* hk_cursor_mode_to_string(hk_cursor_mode mode);
const char* hk_pointer_kind_to_string(hk_pointer_kind kind);
void debug_hk_mouse_report(const char* prefix, const report_mouse_t* report);
void debug_hk_state_to_console(hk_state_t* state);

#include "rpc.h"
#include "pointing.h"
#include "hk_debug.h"

void hk_rpc_sync_state(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data) {
    const hk_state_t *incoming_state = (const hk_state_t*)in_data;

    printf("hk_rpc_sync_state: got state incoming_state->is_main_side: %u\n", incoming_state->is_main_side);
    if (!incoming_state->is_main_side) {
        printf("hk_rpc_sync_state: invalid state, ignoring\n");
        return;
    } else {
        g_hk_state = *incoming_state;
        g_hk_state.is_main_side = false;
        debug_hk_state_to_console(&g_hk_state);
    }
}

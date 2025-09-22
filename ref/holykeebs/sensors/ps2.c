#include "ps2.h"
#include "drivers/ps2/ps2_mouse.h"
#include "pointing_device_internal.h"

const pointing_device_driver_t ps2_pointing_device_driver = {
    .init       = ps2_mouse_init,
    .get_report = ps2_driver_read,
    .set_cpi    = ps2_driver_set_cpi,
    .get_cpi    = ps2_driver_get_cpi
};

uint16_t ps2_driver_get_cpi(void) {
    // TODO(idank): implement.
    return 0;
}
void ps2_driver_set_cpi(uint16_t cpi) {
    // TODO(idank): implement.
}

report_mouse_t ps2_driver_read(report_mouse_t mouse_report) {
    ps2_mouse_read(&mouse_report);
    mouse_report.buttons = 0;

    return mouse_report;
}

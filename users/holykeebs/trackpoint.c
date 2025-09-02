#include QMK_KEYBOARD_H
#include "trackpoint.h"

#define ENABLE_DRIFT_DETECTION false
#define OFFSET_DRIFT_THRESHOLD 1
#define HIST_LENGTH 10

int history_x[HIST_LENGTH] = {0};
int history_y[HIST_LENGTH] = {0};
uint32_t interval_hist[HIST_LENGTH] = {0};

// If the sensor is drifting, correct for it.
void drift_detection(report_mouse_t *report) {
    //add new information to the history array at 0 and shift the rest of the array down
    xprintf("drift detection starting: x: %d, y: %d\n", report->x, report->y);
    for (int i = HIST_LENGTH - 1; i > 0; i--) {
        history_x[i] = history_x[i - 1];
        history_y[i] = history_y[i - 1];
        interval_hist[i] = interval_hist[i - 1];
    }
    //add the new information to the history array
    history_x[0] = report->x;
    history_y[0] = report->y;
    interval_hist[0] = timer_read32();

    xprintf("latest interval: %lu\n", interval_hist[0]- interval_hist[1]);

    // if the latest interval is less than .1 sec, the sensor is not drifting and there is human input
    uint32_t latest_interval = interval_hist[0] - interval_hist[1];
    if (latest_interval < 75) {
        // check if other intervals are also < .1 sec
        return;
    }
    if (latest_interval < 90) {
        if (interval_hist[1] - interval_hist[2] < 50) {
            return;
        }
    }
    if (latest_interval > 150) {
        xprintf("drift detected interval\n");
        report->x = 0;
        report->y = 0;
        return;
    }

    // idle, so check history for 1/0 drift
    int sum_hist_x = 0;
    for (int i = 0; i < HIST_LENGTH; i++) {
        if (abs(history_x[i]) <= OFFSET_DRIFT_THRESHOLD) {
            sum_hist_x += history_x[i];
        }
        else {
            sum_hist_x = 100;
        }
    }
    int sum_hist_y = 0;
    for (int i = 0; i < HIST_LENGTH; i++) {
        if (abs(history_y[i]) <= OFFSET_DRIFT_THRESHOLD) {
            sum_hist_y += history_y[i];
        }
        else {
            sum_hist_y = 100;
        }
    }

    if (abs(sum_hist_x) <= HIST_LENGTH * OFFSET_DRIFT_THRESHOLD && abs(sum_hist_x) > 0) {
        xprintf("Drift detected x\n");
        report->x =0;
    }
    if (abs(sum_hist_y) <= HIST_LENGTH * OFFSET_DRIFT_THRESHOLD && abs(sum_hist_y) > 0) {
        xprintf("Drift detected y\n");
        report->y =0;
    }
}

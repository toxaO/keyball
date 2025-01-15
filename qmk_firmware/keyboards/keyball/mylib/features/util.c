#include "util.h"
#include "os_detection.h"

void initialize_host_os() {
    host_os = detected_host_os();
}

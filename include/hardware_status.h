#ifndef HARDWARE_STATUS_H
#define HARDWARE_STATUS_H

#include "../src/config.h"

struct HardwareStatus {
    bool valve_ok[MAX_VALVES];
    bool flow_ok[MAX_FLOW_SENSORS];
    bool temp_ok;
};

#endif

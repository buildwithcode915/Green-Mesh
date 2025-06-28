#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <Arduino.h>
#include "../../include/hardware_status.h"

class HTTPClientManager {
public:
    void sendHardwareStatus(const String& deviceNumber, const HardwareStatus& status);
    void sendSensorData(const String& deviceNumber, float flows[], int count, float temperature);
};

#endif

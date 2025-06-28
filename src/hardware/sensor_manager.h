#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>

class SensorManager {
public:
    void begin();
    bool isTemperatureSensorConnected();
    void readFlowRates(float rates[]);
    float readTemperature();
};

#endif

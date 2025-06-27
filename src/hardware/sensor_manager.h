#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>

class SensorManager {
public:
    void begin();
    void readFlowRates(float rates[]);
    float readTemperature();
};

#endif

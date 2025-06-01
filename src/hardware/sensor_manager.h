#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include "config.h"

struct SensorConfig {
    uint8_t valve_count;
    uint8_t flow_sensor_count;
    float temperature;
    bool sensors_detected;
};

class SensorManager {
private:
    SensorConfig currentConfig;
    int valve_pins[MAX_VALVES];
    int flow_sensor_pins[MAX_FLOW_SENSORS];
    int temp_sensor_pin;
    
    bool detectValves();
    bool detectFlowSensors();
    float readTemperature();
    bool isValveConnected(int pin);
    bool isFlowSensorConnected(int pin);

public:
    SensorManager();
    void begin();
    bool detectAllSensors();
    SensorConfig getCurrentConfig();
    void printSensorStatus();
    
    // Individual sensor operations
    bool activateValve(uint8_t valve_number);
    bool deactivateValve(uint8_t valve_number);
    float getFlowRate(uint8_t sensor_number);
    float getCurrentTemperature();
    
    // Status methods
    bool areSensorsDetected();
    uint8_t getValveCount();
    uint8_t getFlowSensorCount();
};

#endif
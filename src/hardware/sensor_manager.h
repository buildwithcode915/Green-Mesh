#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include "config.h"

struct SensorConfig {
    uint8_t valve_count;
    uint8_t flow_sensor_count;
    float temperature;
    bool temp_sensor_connected;
    bool sensors_detected;
};

class SensorManager {
private:
    SensorConfig currentConfig;
    int valve_pins[MAX_VALVES];
    int flow_sensor_pins[MAX_FLOW_SENSORS];
    int temp_sensor_pin;
    
    // State tracking arrays
    bool valve_states[MAX_VALVES];           // Track which valves are currently active
    bool flow_sensor_active[MAX_FLOW_SENSORS]; // Track which flow sensors are active/connected
    
    // Private detection methods
    bool detectValves();
    bool detectFlowSensors();
    float readTemperature();
    bool isValveConnected(int pin);
    bool isFlowSensorConnected(int pin);
    bool isTempSensorConnected();  // ← CHANGED: Removed parameter since it uses class member

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
    uint8_t getValveCount();                 // Total connected valves
    uint8_t getFlowSensorCount();           // Total connected flow sensors
    
    // Active state methods
    uint8_t getActiveValveCount();          // Currently active/open valves
    uint8_t getActiveFlowSensorCount();     // Currently active flow sensors
    bool isValveActive(uint8_t valve_number);
    bool isFlowSensorActive(uint8_t sensor_number);
};

#endif
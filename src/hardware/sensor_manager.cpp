#include "sensor_manager.h"

SensorManager::SensorManager() : temp_sensor_pin(TEMP_SENSOR_PIN) {
    // Initialize pin arrays
    int valve_pin_array[] = VALVE_PINS;
    int flow_pin_array[] = FLOW_SENSOR_PINS;
    
    for (int i = 0; i < MAX_VALVES; i++) {
        valve_pins[i] = valve_pin_array[i];
        valve_states[i] = false;  // Initialize valve states
    }
    
    for (int i = 0; i < MAX_FLOW_SENSORS; i++) {
        flow_sensor_pins[i] = flow_pin_array[i];
        flow_sensor_active[i] = false;  // Initialize flow sensor states
    }
    
    // Initialize config
    currentConfig = {0, 0, 0.0, false};
}

void SensorManager::begin() {
    Serial.println("Initializing Sensor Manager...");
    
    // Initialize valve pins as outputs
    for (int i = 0; i < MAX_VALVES; i++) {
        pinMode(valve_pins[i], OUTPUT); 
        digitalWrite(valve_pins[i], LOW); // Ensure all valves are initially off
        valve_states[i] = false;
    }
    
    // Initialize flow sensor pins as inputs with pullup
    for (int i = 0; i < MAX_FLOW_SENSORS; i++) {
        pinMode(flow_sensor_pins[i], INPUT_PULLUP);
        flow_sensor_active[i] = false;
    }
    
    // Initialize temperature sensor pin
    pinMode(temp_sensor_pin, INPUT);
    
    Serial.println("Sensor Manager initialized.");
}

bool SensorManager::detectAllSensors() {
    Serial.println("Starting sensor detection...");
    
    bool valves_detected = detectValves();
    bool flow_sensors_detected = detectFlowSensors();
    currentConfig.temperature = readTemperature();
    currentConfig.temp_sensor_connected = isTempSensorConnected(temp_sensor_pin);
    
    currentConfig.sensors_detected = valves_detected || flow_sensors_detected;
    
    printSensorStatus();
    return currentConfig.sensors_detected;
}

bool SensorManager::detectValves() {
    currentConfig.valve_count = 0;
    
    for (int i = 0; i < MAX_VALVES; i++) {
        if (isValveConnected(valve_pins[i])) {
            currentConfig.valve_count++;
            Serial.println("Valve detected on pin " + String(valve_pins[i]));
        }
    }
    
    Serial.println("Total valves detected: " + String(currentConfig.valve_count));
    return currentConfig.valve_count > 0;
}

bool SensorManager::detectFlowSensors() {
    currentConfig.flow_sensor_count = 0;
    
    for (int i = 0; i < MAX_FLOW_SENSORS; i++) {
        if (isFlowSensorConnected(flow_sensor_pins[i])) {
            currentConfig.flow_sensor_count++;
            flow_sensor_active[i] = true;
            Serial.println("Flow sensor detected on pin " + String(flow_sensor_pins[i]));
        } else {
            flow_sensor_active[i] = false;
        }
    }
    
    Serial.println("Total flow sensors detected: " + String(currentConfig.flow_sensor_count));
    return currentConfig.flow_sensor_count > 0;
}

float SensorManager::readTemperature() {
    // Average multiple readings for stability
    float total = 0;
    for (int i = 0; i < TEMP_SENSOR_SAMPLES; i++) {
        int raw_value = analogRead(temp_sensor_pin);
        float voltage = (raw_value * ADC_VREF) / ((1 << ADC_RESOLUTION) - 1);  // Convert to mV
        float temperature = (voltage - 500.0) / 10.0;  // TMP36 formula: (mV - 500) / 10
        total += temperature;
        delay(10);  // Small delay between readings
    }
    
    float avgTemperature = total / TEMP_SENSOR_SAMPLES;
    Serial.println("Temperature reading: " + String(avgTemperature) + "°C");
    return avgTemperature;
}

bool SensorManager::isValveConnected(int pin) {
    // Temporarily set pin as INPUT_PULLUP to detect connection
    pinMode(pin, INPUT_PULLUP);
    delay(10); // allow stabilization

    int state = digitalRead(pin);
    
    // If pin is floating (no connection), it should stay HIGH due to pullup
    // If valve is connected and pulling down, it should read LOW
    bool connected = (state == LOW);

    // Reset pin mode to OUTPUT (valve control mode)
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW); // default valve state

    return connected;
}


bool SensorManager::isFlowSensorConnected(int pin) {
    // Improved flow sensor detection
    Serial.println("Testing flow sensor on pin " + String(pin));
    
    // Method 1: Check for signal activity over time
    int readings[10];
    bool signal_variation = false;
    
    // Take multiple readings to check for signal activity
    for (int i = 0; i < 10; i++) {
        readings[i] = digitalRead(pin);
        delay(50);
    }
    
    // Check if there's any variation in readings (indicates sensor activity)
    for (int i = 1; i < 10; i++) {
        if (readings[i] != readings[0]) {
            signal_variation = true;
            break;
        }
    }
    
    // Method 2: Check electrical characteristics
    // For Hall effect flow sensors, check if pin is not constantly HIGH or LOW
    bool not_stuck_high = false;
    bool not_stuck_low = false;
    
    for (int i = 0; i < 10; i++) {
        int reading = digitalRead(pin);
        if (reading == LOW) not_stuck_high = true;
        if (reading == HIGH) not_stuck_low = true;
        delay(20);
    }
    
    // Method 3: Test with pull-up behavior
    // Flow sensors typically have open-collector outputs
    bool proper_pullup_behavior = not_stuck_high && not_stuck_low;
    
    // A real flow sensor should show some electrical activity or proper pull-up behavior
    // For now, we'll consider a sensor connected if it's not stuck at one level
    bool sensor_detected = proper_pullup_behavior || signal_variation;
    
    Serial.println("Flow sensor on pin " + String(pin) + ": " + 
                  (sensor_detected ? "DETECTED" : "NOT DETECTED"));
    
    return sensor_detected;
}

SensorConfig SensorManager::getCurrentConfig() {
    return currentConfig;
}

void SensorManager::printSensorStatus() {
    Serial.println("=== Sensor Detection Results ===");
    Serial.println("Valves connected: " + String(currentConfig.valve_count) + "/" + String(MAX_VALVES));
    Serial.println("Flow sensors connected: " + String(currentConfig.flow_sensor_count) + "/" + String(MAX_FLOW_SENSORS));
    Serial.println("Temp sensor connected: " + String(currentConfig.temp_sensor_connected ? "YES" : "NO"));
    Serial.println("Temperature: " + String(currentConfig.temperature) + "°C");
    Serial.println("Overall detection status: " + String(currentConfig.sensors_detected ? "SUCCESS" : "FAILED"));
    Serial.println("================================");
}

bool SensorManager::activateValve(uint8_t valve_number) {
    if (valve_number < 1 || valve_number > currentConfig.valve_count) {
        Serial.println("Invalid valve number: " + String(valve_number));
        return false;
    }
    
    digitalWrite(valve_pins[valve_number - 1], HIGH);
    valve_states[valve_number - 1] = true;
    Serial.println("Valve " + String(valve_number) + " activated");
    return true;
}

bool SensorManager::deactivateValve(uint8_t valve_number) {
    if (valve_number < 1 || valve_number > currentConfig.valve_count) {
        Serial.println("Invalid valve number: " + String(valve_number));
        return false;
    }
    
    digitalWrite(valve_pins[valve_number - 1], LOW);
    valve_states[valve_number - 1] = false;
    Serial.println("Valve " + String(valve_number) + " deactivated");
    return true;
}

uint8_t SensorManager::getActiveValveCount() {
    uint8_t active_count = 0;
    for (int i = 0; i < currentConfig.valve_count; i++) {
        if (valve_states[i]) {
            active_count++;
        }
    }
    return active_count;
}

uint8_t SensorManager::getActiveFlowSensorCount() {
    uint8_t active_count = 0;
    for (int i = 0; i < currentConfig.flow_sensor_count; i++) {
        if (flow_sensor_active[i]) {
            active_count++;
        }
    }
    return active_count;
}

float SensorManager::getFlowRate(uint8_t sensor_number) {
    if (sensor_number < 1 || sensor_number > currentConfig.flow_sensor_count) {
        Serial.println("Invalid flow sensor number: " + String(sensor_number));
        return -1.0;
    }
    
    if (!flow_sensor_active[sensor_number - 1]) {
        Serial.println("Flow sensor " + String(sensor_number) + " is not active");
        return 0.0;
    }
    
    // Implement flow rate calculation based on your sensor type
    int pin = flow_sensor_pins[sensor_number - 1];
    
    // Simple pulse counting method for flow calculation
    unsigned long pulse_count = 0;
    unsigned long start_time = millis();
    unsigned long sample_time = 1000;  // 1 second sample
    
    while (millis() - start_time < sample_time) {
        if (digitalRead(pin) == HIGH) {
            pulse_count++;
            while (digitalRead(pin) == HIGH) {
                // Wait for pulse to go low
                delayMicroseconds(10);
            }
        }
    }
    
    // Calculate flow rate based on calibration factor
    float flow_rate = (pulse_count * 60.0) / FLOW_CALIBRATION_FACTOR;  // L/min
    
    Serial.println("Flow sensor " + String(sensor_number) + " rate: " + String(flow_rate) + " L/min");
    return flow_rate;
}

float SensorManager::getCurrentTemperature() {
    currentConfig.temperature = readTemperature();
    return currentConfig.temperature;
}

bool SensorManager::areSensorsDetected() {
    return currentConfig.sensors_detected;
}

uint8_t SensorManager::getValveCount() {
    return currentConfig.valve_count;
}

uint8_t SensorManager::getFlowSensorCount() {
    return currentConfig.flow_sensor_count;
}

bool SensorManager::isValveActive(uint8_t valve_number) {
    if (valve_number < 1 || valve_number > currentConfig.valve_count) {
        return false;
    }
    return valve_states[valve_number - 1];
}

bool SensorManager::isFlowSensorActive(uint8_t sensor_number) {
    if (sensor_number < 1 || sensor_number > currentConfig.flow_sensor_count) {
        return false;
    }
    return flow_sensor_active[sensor_number - 1];
}

bool SensorManager::isTempSensorConnected(int pin) {
    int raw = analogRead(pin);
    float voltage = (raw * ADC_VREF) / ((1 << ADC_RESOLUTION) - 1);
    float temp = (voltage - 500.0) / 10.0;

    // TMP36 typically gives 0.1V–2V (10°C to 150°C)
    return (temp > -40.0 && temp < 125.0);  // Safe valid range
}

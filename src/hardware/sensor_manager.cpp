#include "sensor_manager.h"

SensorManager::SensorManager() : temp_sensor_pin(TEMP_SENSOR_PIN) {
    // Initialize pin arrays
    int valve_pin_array[] = VALVE_PINS;
    int flow_pin_array[] = FLOW_SENSOR_PINS;
    
    for (int i = 0; i < MAX_VALVES; i++) {
        valve_pins[i] = valve_pin_array[i];
    }
    
    for (int i = 0; i < MAX_FLOW_SENSORS; i++) {
        flow_sensor_pins[i] = flow_pin_array[i];
    }
    
    // Initialize config
    currentConfig = {0, 0, 0.0, false};
}

void SensorManager::begin() {
    Serial.println("Initializing Sensor Manager...");
    
    // Initialize valve pins as outputs
    for (int i = 0; i < MAX_VALVES; i++) {
        pinMode(valve_pins[i], OUTPUT);
        digitalWrite(valve_pins[i], LOW);  // All valves off initially
    }
    
    // Initialize flow sensor pins as inputs with pullup
    for (int i = 0; i < MAX_FLOW_SENSORS; i++) {
        pinMode(flow_sensor_pins[i], INPUT_PULLUP);
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
            Serial.println("Flow sensor detected on pin " + String(flow_sensor_pins[i]));
        }
    }
    
    Serial.println("Total flow sensors detected: " + String(currentConfig.flow_sensor_count));
    return currentConfig.flow_sensor_count > 0;
}

float SensorManager::readTemperature() {
    // Read analog value from temperature sensor
    int raw_value = analogRead(temp_sensor_pin);
    
    // Convert to temperature (assuming TMP36 or similar sensor)
    // Adjust this formula based on your actual temperature sensor
    float voltage = (raw_value * 3.3) / 4096.0;  // ESP32 12-bit ADC
    float temperature = (voltage - 0.5) * 100.0;  // TMP36 formula
    
    Serial.println("Temperature reading: " + String(temperature) + "°C");
    return temperature;
}

bool SensorManager::isValveConnected(int pin) {
    // Method to detect if valve is connected
    // This could involve checking electrical continuity, 
    // current draw, or feedback signals
    
    // Simple detection method - check for proper response
    digitalWrite(pin, HIGH);
    delay(SENSOR_DETECTION_DELAY);
    
    // Read some form of feedback (this depends on your valve type)
    // For now, assuming all configured pins have valves
    // You might read current, voltage feedback, or position sensors
    
    digitalWrite(pin, LOW);
    
    // Placeholder logic - replace with actual detection method
    // For example, if valves provide feedback or you can measure current
    return true;  // Assume valve is present for now
}

bool SensorManager::isFlowSensorConnected(int pin) {
    // Method to detect if flow sensor is connected
    // This could involve checking for signal patterns or impedance
    
    int stable_count = 0;
    int last_reading = digitalRead(pin);
    
    // Check for signal stability/activity
    for (int i = 0; i < SENSOR_STABLE_COUNT; i++) {
        delay(SENSOR_DETECTION_DELAY);
        int current_reading = digitalRead(pin);
        
        if (current_reading == last_reading) {
            stable_count++;
        }
        last_reading = current_reading;
    }
    
    // If we get stable readings, sensor might be connected
    // You might also check for pulse patterns indicating flow
    return stable_count >= (SENSOR_STABLE_COUNT - 1);
}

SensorConfig SensorManager::getCurrentConfig() {
    return currentConfig;
}

void SensorManager::printSensorStatus() {
    Serial.println("=== Sensor Detection Results ===");
    Serial.println("Valves detected: " + String(currentConfig.valve_count));
    Serial.println("Flow sensors detected: " + String(currentConfig.flow_sensor_count));
    Serial.println("Temperature: " + String(currentConfig.temperature) + "°C");
    Serial.println("Overall detection status: " + String(currentConfig.sensors_detected ? "SUCCESS" : "FAILED"));
    Serial.println("================================");
}

bool SensorManager::activateValve(uint8_t valve_number) {
    if (valve_number < 1 || valve_number > currentConfig.valve_count) {
        return false;
    }
    
    digitalWrite(valve_pins[valve_number - 1], HIGH);
    Serial.println("Valve " + String(valve_number) + " activated");
    return true;
}

bool SensorManager::deactivateValve(uint8_t valve_number) {
    if (valve_number < 1 || valve_number > currentConfig.valve_count) {
        return false;
    }
    
    digitalWrite(valve_pins[valve_number - 1], LOW);
    Serial.println("Valve " + String(valve_number) + " deactivated");
    return true;
}

float SensorManager::getFlowRate(uint8_t sensor_number) {
    if (sensor_number < 1 || sensor_number > currentConfig.flow_sensor_count) {
        return -1.0;
    }
    
    // Implement flow rate calculation based on your sensor type
    // This is a placeholder implementation
    int pin = flow_sensor_pins[sensor_number - 1];
    
    // Count pulses over a time period for flow calculation
    // This is simplified - implement according to your flow sensor specs
    return 0.0;  // Placeholder
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
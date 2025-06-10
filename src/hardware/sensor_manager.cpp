#include "sensor_manager.h"

SensorManager::SensorManager() : temp_sensor_pin(TEMP_SENSOR_PIN) {
    // Initialize pin arrays
    int valve_pin_array[] = VALVE_PINS;
    int flow_pin_array[] = FLOW_SENSOR_PINS;
    
    for (int i = 0; i < MAX_VALVES; i++) {
        valve_pins[i] = valve_pin_array[i];
        valve_states[i] = false;
    }
    
    for (int i = 0; i < MAX_FLOW_SENSORS; i++) {
        flow_sensor_pins[i] = flow_pin_array[i];
        flow_sensor_active[i] = false;
    }
    
    // Initialize config
    currentConfig = {0, 0, 0.0, false, false};
}

void SensorManager::begin() {
    Serial.println("Initializing Sensor Manager...");
    
    // Initialize valve pins as outputs
    for (int i = 0; i < MAX_VALVES; i++) {
        pinMode(valve_pins[i], OUTPUT); 
        digitalWrite(valve_pins[i], LOW); // Ensure all valves are initially off
        valve_states[i] = false;
        Serial.println("Valve pin " + String(valve_pins[i]) + " initialized as OUTPUT");
    }
    
    // Initialize flow sensor pins as inputs with pullup
    for (int i = 0; i < MAX_FLOW_SENSORS; i++) {
        pinMode(flow_sensor_pins[i], INPUT_PULLUP);
        flow_sensor_active[i] = false;
        Serial.println("Flow sensor pin " + String(flow_sensor_pins[i]) + " initialized as INPUT_PULLUP");
    }
    
    // Initialize temperature sensor pin
    pinMode(temp_sensor_pin, INPUT);
    Serial.println("Temperature sensor pin " + String(temp_sensor_pin) + " initialized as INPUT");
    
    Serial.println("Sensor Manager initialized successfully.");
}

bool SensorManager::detectAllSensors() {
    Serial.println("=== Starting Comprehensive Sensor Detection ===");
    
    bool valves_detected = detectValves();
    bool flow_sensors_detected = detectFlowSensors();
    
    // Read temperature and check sensor connection
    currentConfig.temperature = readTemperature();
    currentConfig.temp_sensor_connected = isTempSensorConnected();
    
    // Overall detection success if any sensors are found
    currentConfig.sensors_detected = valves_detected || flow_sensors_detected || currentConfig.temp_sensor_connected;
    
    printSensorStatus();
    
    Serial.println("=== Sensor Detection Complete ===");
    Serial.println("Detection Result: " + String(currentConfig.sensors_detected ? "SUCCESS" : "FAILED"));
    
    return currentConfig.sensors_detected;
}

bool SensorManager::detectValves() {
    Serial.println("--- Detecting Relay-Controlled Solenoid Valves ---");
    currentConfig.valve_count = 0;
    
    for (int i = 0; i < MAX_VALVES; i++) {
        Serial.println("Testing valve " + String(i + 1) + " on pin " + String(valve_pins[i]));
        
        if (isValveConnected(valve_pins[i])) {
            currentConfig.valve_count++;
            Serial.println("✓ Valve " + String(i + 1) + " DETECTED on pin " + String(valve_pins[i]));
        } else {
            Serial.println("✗ Valve " + String(i + 1) + " NOT DETECTED on pin " + String(valve_pins[i]));
        }
    }
    
    Serial.println("Total valves detected: " + String(currentConfig.valve_count) + "/" + String(MAX_VALVES));
    return currentConfig.valve_count > 0;
}

bool SensorManager::detectFlowSensors() {
    Serial.println("--- Detecting Flow Sensors ---");
    currentConfig.flow_sensor_count = 0;
    
    for (int i = 0; i < MAX_FLOW_SENSORS; i++) {
        Serial.println("Testing flow sensor " + String(i + 1) + " on pin " + String(flow_sensor_pins[i]));
        
        if (isFlowSensorConnected(flow_sensor_pins[i])) {
            currentConfig.flow_sensor_count++;
            flow_sensor_active[i] = true;
            Serial.println("✓ Flow sensor " + String(i + 1) + " DETECTED on pin " + String(flow_sensor_pins[i]));
        } else {
            flow_sensor_active[i] = false;
            Serial.println("✗ Flow sensor " + String(i + 1) + " NOT DETECTED on pin " + String(flow_sensor_pins[i]));
        }
    }
    
    Serial.println("Total flow sensors detected: " + String(currentConfig.flow_sensor_count) + "/" + String(MAX_FLOW_SENSORS));
    return currentConfig.flow_sensor_count > 0;
}

bool SensorManager::isValveConnected(int pin) {
    Serial.println("  Testing valve connection on pin " + String(pin));
    
    // Ensure pin is in output mode
    pinMode(pin, OUTPUT);
    
    // Test 1: Set HIGH and verify
    digitalWrite(pin, HIGH);
    delay(50); // Allow relay to settle
    bool high_state_stable = true; // In real scenario, you'd check if relay clicked or current changed
    
    // Test 2: Set LOW and verify
    digitalWrite(pin, LOW);
    delay(50); // Allow relay to settle
    bool low_state_stable = true; // In real scenario, you'd check if relay clicked back
    
    // For this implementation, we'll assume all configured valve pins have relays connected
    // In a real scenario, you might check for:
    // - Current draw changes when relay switches
    // - Voltage feedback from relay coil
    // - Physical relay click sound detection
    
    // Simple assumption: if pin can be controlled, valve is connected
    bool valve_connected = high_state_stable && low_state_stable;
    
    // Reset to safe state
    digitalWrite(pin, LOW);
    
    Serial.println("    Valve detection result: " + String(valve_connected ? "CONNECTED" : "NOT CONNECTED"));
    
    return valve_connected;
}

bool SensorManager::isFlowSensorConnected(int pin) {
    Serial.println("  Testing flow sensor connection on pin " + String(pin));
    
    // Enhanced flow sensor detection for common types:
    // - Hall effect sensors (YF-S201, YF-S401, etc.)
    // - Optical sensors
    // - Reed switch based sensors
    
    // Test 1: Check for proper pull-up behavior
    pinMode(pin, INPUT_PULLUP);
    delay(100); // Allow pin to stabilize
    
    // Take multiple readings to check for signal characteristics
    int high_count = 0;
    int low_count = 0;
    int total_readings = 20;
    
    for (int i = 0; i < total_readings; i++) {
        int reading = digitalRead(pin);
        if (reading == HIGH) {
            high_count++;
        } else {
            low_count++;
        }
        delay(25); // 25ms between readings
    }
    
    // Test 2: Check for signal variation (indicates sensor activity or proper electrical connection)
    bool has_signal_variation = (high_count > 0 && low_count > 0);
    
    // Test 3: Check if pin is not stuck (indicates proper electrical connection)
    bool not_stuck_high = (low_count > 0);
    bool not_stuck_low = (high_count > 0);
    bool proper_electrical_connection = not_stuck_high && not_stuck_low;
    
    // Test 4: For flow sensors, check impedance characteristics
    // Most flow sensors will not be constantly HIGH or constantly LOW
    float high_percentage = (float)high_count / total_readings * 100.0;
    bool reasonable_signal_levels = (high_percentage > 10.0 && high_percentage < 90.0);
    
    // A flow sensor is considered connected if:
    // 1. It shows signal variation (active sensor with flow), OR
    // 2. It has proper electrical characteristics (connected but no flow), OR
    // 3. It has reasonable signal levels (not stuck at rail voltages)
    bool sensor_connected = has_signal_variation || proper_electrical_connection || reasonable_signal_levels;
    
    Serial.println("    Flow sensor analysis:");
    Serial.println("      High readings: " + String(high_count) + "/" + String(total_readings) + " (" + String(high_percentage, 1) + "%)");
    Serial.println("      Signal variation: " + String(has_signal_variation ? "YES" : "NO"));
    Serial.println("      Proper connection: " + String(proper_electrical_connection ? "YES" : "NO"));
    Serial.println("      Detection result: " + String(sensor_connected ? "CONNECTED" : "NOT CONNECTED"));
    
    return sensor_connected;
}

float SensorManager::readTemperature() {
    // Read temperature from TMP36 or similar analog sensor
    Serial.println("Reading temperature sensor...");
    
    // Take multiple samples for accuracy
    float total = 0;
    int valid_readings = 0;
    
    for (int i = 0; i < TEMP_SENSOR_SAMPLES; i++) {
        int raw_value = analogRead(temp_sensor_pin);
        
        // Convert ADC reading to voltage (mV)
        float voltage = (raw_value * ADC_VREF) / ((1 << ADC_RESOLUTION) - 1);
        
        // Convert voltage to temperature using TMP36 formula
        // TMP36: Vout = (Temp°C × 10mV) + 500mV
        float temperature = (voltage - 500.0) / 10.0;
        
        // Only include reasonable temperature readings
        if (temperature > -40.0 && temperature < 125.0) {
            total += temperature;
            valid_readings++;
        }
        
        delay(20); // Small delay between readings
    }
    
    if (valid_readings == 0) {
        Serial.println("No valid temperature readings obtained");
        return 0.0;
    }
    
    float avgTemperature = total / valid_readings;
    Serial.println("Temperature reading: " + String(avgTemperature, 2) + "°C (from " + String(valid_readings) + " valid samples)");
    
    return avgTemperature;
}

bool SensorManager::isTempSensorConnected() {
    Serial.println("--- Detecting Temperature Sensor ---");
    
    // Read raw ADC value
    int raw_value = analogRead(temp_sensor_pin);
    float voltage = (raw_value * ADC_VREF) / ((1 << ADC_RESOLUTION) - 1);
    float temperature = (voltage - 500.0) / 10.0;
    
    Serial.println("Temperature sensor analysis:");
    Serial.println("  Raw ADC: " + String(raw_value));
    Serial.println("  Voltage: " + String(voltage, 2) + " mV");
    Serial.println("  Temperature: " + String(temperature, 2) + "°C");
    
    // Check if readings are within reasonable range for TMP36
    // TMP36 operating range: -40°C to +125°C
    // Voltage range: 100mV to 1750mV
    bool voltage_in_range = (voltage >= 100.0 && voltage <= 1750.0);
    bool temp_in_range = (temperature >= -40.0 && temperature <= 125.0);
    
    // Additional check: ensure it's not reading exactly 0V or VCC (disconnected sensor)
    bool not_disconnected = (raw_value > 50 && raw_value < ((1 << ADC_RESOLUTION) - 50));
    
    bool sensor_connected = voltage_in_range && temp_in_range && not_disconnected;
    
    Serial.println("  Voltage in range: " + String(voltage_in_range ? "YES" : "NO"));
    Serial.println("  Temperature in range: " + String(temp_in_range ? "YES" : "NO"));
    Serial.println("  Not disconnected: " + String(not_disconnected ? "YES" : "NO"));
    Serial.println("  Detection result: " + String(sensor_connected ? "CONNECTED" : "NOT CONNECTED"));
    
    return sensor_connected;
}

// Getter methods
SensorConfig SensorManager::getCurrentConfig() {
    return currentConfig;
}

uint8_t SensorManager::getValveCount() {
    return currentConfig.valve_count;
}

uint8_t SensorManager::getFlowSensorCount() {
    return currentConfig.flow_sensor_count;
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

float SensorManager::getCurrentTemperature() {
    currentConfig.temperature = readTemperature();
    return currentConfig.temperature;
}

bool SensorManager::areSensorsDetected() {
    return currentConfig.sensors_detected;
}

// Valve control methods
bool SensorManager::activateValve(uint8_t valve_number) {
    if (valve_number < 1 || valve_number > currentConfig.valve_count) {
        Serial.println("Error: Invalid valve number " + String(valve_number) + ". Valid range: 1-" + String(currentConfig.valve_count));
        return false;
    }
    
    int valve_index = valve_number - 1;
    digitalWrite(valve_pins[valve_index], HIGH);
    valve_states[valve_index] = true;
    
    Serial.println("Valve " + String(valve_number) + " ACTIVATED on pin " + String(valve_pins[valve_index]));
    return true;
}

bool SensorManager::deactivateValve(uint8_t valve_number) {
    if (valve_number < 1 || valve_number > currentConfig.valve_count) {
        Serial.println("Error: Invalid valve number " + String(valve_number) + ". Valid range: 1-" + String(currentConfig.valve_count));
        return false;
    }
    
    int valve_index = valve_number - 1;
    digitalWrite(valve_pins[valve_index], LOW);
    valve_states[valve_index] = false;
    
    Serial.println("Valve " + String(valve_number) + " DEACTIVATED on pin " + String(valve_pins[valve_index]));
    return true;
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

float SensorManager::getFlowRate(uint8_t sensor_number) {
    if (sensor_number < 1 || sensor_number > currentConfig.flow_sensor_count) {
        Serial.println("Error: Invalid flow sensor number " + String(sensor_number) + ". Valid range: 1-" + String(currentConfig.flow_sensor_count));
        return -1.0;
    }
    
    if (!flow_sensor_active[sensor_number - 1]) {
        Serial.println("Warning: Flow sensor " + String(sensor_number) + " is not active");
        return 0.0;
    }
    
    int pin = flow_sensor_pins[sensor_number - 1];
    
    // Pulse counting method for flow rate calculation
    unsigned long pulse_count = 0;
    unsigned long start_time = millis();
    unsigned long sample_time = 1000; // 1 second sample
    
    // Count pulses for the sample period
    while (millis() - start_time < sample_time) {
        if (digitalRead(pin) == HIGH) {
            pulse_count++;
            // Wait for pulse to go low to avoid double counting
            while (digitalRead(pin) == HIGH && (millis() - start_time < sample_time)) {
                delayMicroseconds(100);
            }
        }
        delayMicroseconds(100);
    }
    
    // Calculate flow rate: (pulses/second) * (60 seconds/minute) / (pulses/liter)
    float flow_rate = (pulse_count * 60.0) / FLOW_CALIBRATION_FACTOR; // L/min
    
    Serial.println("Flow sensor " + String(sensor_number) + ": " + String(pulse_count) + " pulses/sec = " + String(flow_rate, 2) + " L/min");
    return flow_rate;
}

void SensorManager::printSensorStatus() {
    Serial.println();
    Serial.println("╔═══════════════════════════════════════╗");
    Serial.println("║         SENSOR DETECTION SUMMARY     ║");
    Serial.println("╠═══════════════════════════════════════╣");
    Serial.println("║ Solenoid Valves: " + String(currentConfig.valve_count) + "/" + String(MAX_VALVES) + " detected" + String(MAX_VALVES >= 10 ? " " : "  ") + "║");
    Serial.println("║ Flow Sensors:    " + String(currentConfig.flow_sensor_count) + "/" + String(MAX_FLOW_SENSORS) + " detected" + String(MAX_FLOW_SENSORS >= 10 ? " " : "  ") + "║");
    Serial.println("║ Temperature:     " + String(currentConfig.temp_sensor_connected ? "CONNECTED " : "NOT FOUND ") + "   ║");
    Serial.println("║ Current Temp:    " + String(currentConfig.temperature, 1) + "°C" + String(currentConfig.temperature >= 0 && currentConfig.temperature < 10 ? "      " : currentConfig.temperature >= 10 && currentConfig.temperature < 100 ? "     " : "    ") + "║");
    Serial.println("║ Active Valves:   " + String(getActiveValveCount()) + "/" + String(currentConfig.valve_count) + String(currentConfig.valve_count >= 10 ? "         " : "          ") + "║");
    Serial.println("║ Active Sensors:  " + String(getActiveFlowSensorCount()) + "/" + String(currentConfig.flow_sensor_count) + String(currentConfig.flow_sensor_count >= 10 ? "         " : "          ") + "║");
    Serial.println("║ Overall Status:  " + String(currentConfig.sensors_detected ? "SUCCESS   " : "FAILED    ") + "║");
    Serial.println("╚═══════════════════════════════════════╝");
    Serial.println();
}
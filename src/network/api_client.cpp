#include "api_client.h"
#include <ArduinoJson.h> // Add ArduinoJson library

APIClient::APIClient() {}

APIClient::~APIClient() {
    http.end();
}

void APIClient::setTimeout(int timeout) {
    http.setTimeout(timeout);
}

bool APIClient::validateDevice(const String& customer_uid, const String& device_number, 
                              const String& ssid, const String& password) {
    Serial.println("Validating device with server...");
    
    http.begin(API_ENDPOINT);
    http.addHeader("Content-Type", "application/json");
    setTimeout(HTTP_TIMEOUT);

    StaticJsonDocument<256> doc; // Adjust size as needed
    doc["uid"] = customer_uid;
    doc["device_number"] = device_number;
    doc["ssid"] = ssid;
    doc["wifi_password"] = password;

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    Serial.println("Sending validation request: " + jsonPayload);

    int httpCode = http.POST(jsonPayload);

    if (httpCode > 0) {
        String response = http.getString();
        Serial.print("HTTP Response code: ");
        Serial.println(httpCode);
        Serial.println("Response: " + response);
        
        http.end();
        return (httpCode == 200);
    } else {
        Serial.print("HTTP Request failed, error: ");
        Serial.println(http.errorToString(httpCode));
    }
    
    http.end();
    return false;
}

bool APIClient::updateDeviceStatus(const String& customer_uid, const String& device_number,
                                  const SensorConfig& sensorConfig, SensorManager* sensorManager) {
    Serial.println("Updating device status with current sensor data...");
    
    http.begin(DEVICE_UPDATE_ENDPOINT);
    http.addHeader("Content-Type", "application/json");
    setTimeout(HTTP_TIMEOUT);

    // Get actual active counts instead of total detected counts
    uint8_t active_valves = 0;
    uint8_t active_flow_sensors = 0;
    
    if (sensorManager != nullptr) {
        active_valves = sensorManager->getActiveValveCount();
        active_flow_sensors = sensorManager->getActiveFlowSensorCount();
    }

    // Create JSON payload with both connected and active sensor data
    // StaticJsonDocument<384> doc; // Adjust size as needed
    // doc["uid"] = customer_uid;
    // doc["device_number"] = device_number;
    // doc["valve_connected"] = sensorConfig.valve_count;
    // doc["valve_active"] = active_valves;
    // doc["flow_sensor_connected"] = sensorConfig.flow_sensor_count;
    // doc["flow_sensor_active"] = active_flow_sensors;
    // doc["temp_sensor"] = serialized(String(sensorConfig.temperature, 2));
    // doc["temp_sensor_connected"] = sensorConfig.temp_sensor_connected;
    // doc["timestamp"] = millis();

    StaticJsonDocument<384> doc;
    doc["uid"] = customer_uid;
    doc["device_number"] = device_number;
    doc["valve_connected"] = sensorConfig.valve_count;
    doc["valve_active"] = active_valves;
    doc["flow_sensor_connected"] = sensorConfig.flow_sensor_count;
    doc["flow_sensor_active"] = active_flow_sensors;
    doc["temp_sensor_connected"] = sensorConfig.temp_sensor_connected;

    if (sensorConfig.temp_sensor_connected) {
        doc["temp_sensor"] = serialized(String(sensorConfig.temperature, 2));
    } else {
        doc["temp_sensor"] = nullptr; 
    }

    doc["timestamp"] = millis();


    String jsonPayload;
    serializeJson(doc, jsonPayload);

    Serial.println("Sending device update: " + jsonPayload);
    Serial.println("Connected - Valves: " + String(sensorConfig.valve_count) + 
                  ", Flow Sensors: " + String(sensorConfig.flow_sensor_count));
    Serial.println("Active - Valves: " + String(active_valves) + 
                  ", Flow Sensors: " + String(active_flow_sensors));

    int httpCode = http.POST(jsonPayload);

    if (httpCode > 0) {
        String response = http.getString();
        Serial.print("Update response code: ");
        Serial.println(httpCode);
        Serial.println("Response: " + response);
        
        http.end();
        return (httpCode == 200);
    } else {
        Serial.print("Device update failed, error: ");
        Serial.println(http.errorToString(httpCode));
    }
    
    http.end();
    return false;
}

// Overloaded method for backward compatibility
bool APIClient::updateDeviceStatus(const String& customer_uid, const String& device_number,
                                  const SensorConfig& sensorConfig) {
    return updateDeviceStatus(customer_uid, device_number, sensorConfig, nullptr);
}

bool APIClient::hasInternetConnection() {
    Serial.println("Checking internet connection...");
    
    http.begin(INTERNET_CHECK_URL);
    setTimeout(HTTP_TIMEOUT);
    
    int httpCode = http.GET();
    Serial.print("Internet check response: ");
    Serial.println(httpCode);
    
    http.end();
    return (httpCode == 204);
}
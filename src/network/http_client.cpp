#include "http_client.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

void HTTPClientManager::sendSensorData(const String& deviceNumber, float flows[], int count, float temperature) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin("http://192.168.31.156:8000/api/device/data");
        http.addHeader("Content-Type", "application/json");

        String json = "{\"device_number\":\"" + deviceNumber + "\",\"flow_rates\":[";
        for (int i = 0; i < count; i++) {
            json += String(flows[i]);
            if (i < count - 1) json += ",";
        }
        json += "],\"temperature\":" + String(temperature) + "}";

        int httpCode = http.POST(json);
        if (httpCode > 0) {
            Serial.println("✅ Data sent to backend.");
        } else {
            Serial.printf("❌ Failed to send. Code: %d\n", httpCode);
        }

        http.end();
    }
}

void HTTPClientManager::sendHardwareStatus(const String& deviceNumber, const HardwareStatus& status) {
    StaticJsonDocument<512> doc;
    doc["device_number"] = deviceNumber;

    JsonArray valveArray = doc.createNestedArray("valves");
    for (int i = 0; i < MAX_VALVES; i++) valveArray.add(status.valve_ok[i]);

    JsonArray flowArray = doc.createNestedArray("flow_sensors");
    for (int i = 0; i < MAX_FLOW_SENSORS; i++) flowArray.add(status.flow_ok[i]);

    doc["temperature_sensor"] = status.temp_ok;

    String payload;
    serializeJson(doc, payload);

    HTTPClient http;
    http.begin("http://192.168.31.156:8000/api/device/health-report");
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.POST(payload);
    Serial.printf("Hardware status sent (code %d)\n", httpCode);
    http.end();
}

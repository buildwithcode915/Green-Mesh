#include "http_client.h"
#include <WiFi.h>
#include <HTTPClient.h>

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

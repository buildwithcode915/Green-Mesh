#include "api_client.h"
#include <ArduinoJson.h>

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

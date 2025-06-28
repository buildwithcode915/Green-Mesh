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

    StaticJsonDocument<256> doc;
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

bool APIClient::hasInternetConnection() {
    Serial.println("Checking internet connectivity...");
    
    HTTPClient testHttp;
    testHttp.begin(CONNECTIVITY_CHECK_URL);
    testHttp.setTimeout(5000);
    
    int httpCode = testHttp.GET();
    testHttp.end();
    
    if (httpCode > 0) {
        Serial.print("Internet connectivity check - HTTP Response code: ");
        Serial.println(httpCode);
        return (httpCode == 200);
    } else {
        Serial.print("Internet connectivity check failed, error: ");
        Serial.println(testHttp.errorToString(httpCode));
        return false;
    }
}
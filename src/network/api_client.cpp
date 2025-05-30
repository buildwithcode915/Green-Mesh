#include "api_client.h"

APIClient::APIClient() {}

APIClient::~APIClient() {
    http.end();
}

void APIClient::setTimeout(int timeout) {
    http.setTimeout(timeout);
}

bool APIClient::validateDevice(const String& customer_uid, const String& device_number, 
                              const String& ssid, const String& password) {
    Serial.println("Validating device...");
    
    http.begin(API_ENDPOINT);
    http.addHeader("Content-Type", "application/json");
    setTimeout(HTTP_TIMEOUT);

    String json = "{\"uid\":\"" + customer_uid + 
                  "\",\"device_number\":\"" + device_number + 
                  "\",\"ssid\":\"" + ssid + 
                  "\",\"wifi_password\":\"" + password + "\"}";
    
    Serial.println("Sending validation request: " + json);
    
    int httpCode = http.POST(json);
    
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
    Serial.println("Checking internet connection...");
    
    http.begin(INTERNET_CHECK_URL);
    setTimeout(HTTP_TIMEOUT);
    
    int httpCode = http.GET();
    Serial.print("Internet check response: ");
    Serial.println(httpCode);
    
    http.end();
    return (httpCode == 204);
}

bool APIClient::updateDeviceStatus(const String& customer_uid, const String& device_number, 
                                   int valve, int flow_sensor, float temp_sensor) {
    http.begin(DEVICE_UPDATE_ENDPOINT);
    http.addHeader("Content-Type", "application/json");
    setTimeout(HTTP_TIMEOUT);

    String json = "{\"uid\":\"" + customer_uid + 
                  "\",\"device_number\":\"" + device_number + 
                  "\",\"valve\":" + String(valve) + 
                  ",\"flow_sensor\":" + String(flow_sensor) + 
                  ",\"temp_sensor\":" + String(temp_sensor, 2) + "}";

    Serial.println("Sending device update: " + json);

    int httpCode = http.POST(json);

    if (httpCode > 0) {
        String response = http.getString();
        Serial.print("Update response code: ");
        Serial.println(httpCode);
        Serial.println("Response: " + response);

        http.end();
        return (httpCode == 200);
    }

    Serial.print("Update failed, error: ");
    Serial.println(http.errorToString(httpCode));

    http.end();
    return false;
}

#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <HTTPClient.h>
#include <Arduino.h>
#include "config.h"

class APIClient {
private:
    HTTPClient http;

public:
    APIClient();
    ~APIClient();
    
    bool validateDevice(const String& customer_uid, const String& device_number, 
                       const String& ssid, const String& password);
    bool hasInternetConnection();
    bool updateDeviceStatus(const String& customer_uid, const String& device_number, 
                        int valve, int flow_sensor, float temp_sensor);

    
private:
    void setTimeout(int timeout);
};

#endif
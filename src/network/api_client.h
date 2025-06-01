#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <HTTPClient.h>
#include <Arduino.h>
#include "config.h"
#include "../hardware/sensor_manager.h"

class APIClient {
private:
    HTTPClient http;

public:
    APIClient();
    ~APIClient();
    
    bool validateDevice(const String& customer_uid, const String& device_number, 
                       const String& ssid, const String& password);
    bool updateDeviceStatus(const String& customer_uid, const String& device_number,
                           const SensorConfig& sensorConfig);
    bool hasInternetConnection();
    
private:
    void setTimeout(int timeout);
};

#endif
#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <Arduino.h>

class HTTPClientManager {
public:
    void sendSensorData(const String& deviceNumber, float flows[], int count, float temperature);
};

#endif

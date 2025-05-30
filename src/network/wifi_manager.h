#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <Arduino.h>
#include "config.h"

enum class WiFiState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    FAILED
};

class WiFiManager {
private:
    WiFiState currentState;
    
public:
    WiFiManager();
    WiFiState connectToWiFi(const String& ssid, const String& password);
    bool isConnected();
    String getLocalIP();
    void disconnect();
    WiFiState getCurrentState();
    
    // AP Mode functions
    bool startAPMode();
    void stopAPMode();
    String getAPIP();
};

#endif
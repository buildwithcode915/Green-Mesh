#include "wifi_manager.h"

WiFiManager::WiFiManager() : currentState(WiFiState::DISCONNECTED) {}

WiFiState WiFiManager::connectToWiFi(const String& ssid, const String& password) {
    currentState = WiFiState::CONNECTING;
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.println("Connecting to WiFi...");

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < WIFI_RETRY_COUNT) {
        delay(500);
        Serial.print(".");
        retry++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected.");
        currentState = WiFiState::CONNECTED;
        return WiFiState::CONNECTED;
    } else {
        Serial.println("WiFi Connection Failed.");
        currentState = WiFiState::FAILED;
        return WiFiState::FAILED;
    }
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED && currentState == WiFiState::CONNECTED;
}

String WiFiManager::getLocalIP() {
    return WiFi.localIP().toString();
}

void WiFiManager::disconnect() {
    WiFi.disconnect();
    currentState = WiFiState::DISCONNECTED;
}

WiFiState WiFiManager::getCurrentState() {
    return currentState;
}

bool WiFiManager::startAPMode() {
    WiFi.disconnect();
    delay(100);
    
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP_ADDR, AP_GATEWAY, AP_SUBNET);
    return WiFi.softAP(AP_SSID, AP_PASSWORD);
}

void WiFiManager::stopAPMode() {
    WiFi.softAPdisconnect(true);
}

String WiFiManager::getAPIP() {
    return WiFi.softAPIP().toString();
}
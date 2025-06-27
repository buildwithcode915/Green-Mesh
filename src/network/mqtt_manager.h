#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "../storage/preferences_manager.h"
#include "../config.h"

class MQTTManager {
private:
    WiFiClientSecure wifiClient;
    PubSubClient client;
    String deviceTopic;
    PreferencesManager* prefs;
    int valvePins[MAX_VALVES];

public:
    MQTTManager();
    void begin(PreferencesManager* preferences);
    void loop();
    void reconnect();
    void subscribeToTopic();
    void handleMessage(char* topic, byte* payload, unsigned int length);
    void publishHeartbeat(const String& topic);
};

#endif

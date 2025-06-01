#ifndef PREFERENCES_MANAGER_H
#define PREFERENCES_MANAGER_H

#include <Preferences.h>
#include <Arduino.h>
#include "../hardware/sensor_manager.h"

struct DeviceConfig {
    String ssid;
    String password;
    String customer_uid;
    String device_number;
    bool isOnboarded;
    bool isFirstBoot;
    SensorConfig sensorConfig;
};

class PreferencesManager {
private:
    Preferences preferences;
    static const char* NAMESPACE;
    static const char* SENSOR_NAMESPACE;

public:
    PreferencesManager();
    bool loadConfig(DeviceConfig& config);
    bool saveConfig(const DeviceConfig& config);
    bool saveCredentials(const String& ssid, const String& password, 
                        const String& customer_uid, const String& device_number);
    bool saveSensorConfig(const SensorConfig& sensorConfig);
    bool loadSensorConfig(SensorConfig& sensorConfig);
    bool markAsOnboarded();
    bool markFirstBootComplete();
    bool isFirstBoot();
    void clearAll();
    bool hasStoredCredentials();
};

#endif
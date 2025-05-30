#ifndef PREFERENCES_MANAGER_H
#define PREFERENCES_MANAGER_H

#include <Preferences.h>
#include <Arduino.h>

struct DeviceConfig {
    String ssid;
    String password;
    String customer_uid;
    String device_number;
    int valve;
    int flow_sensor;
    float temp_sensor;
    bool isOnboarded;
};

class PreferencesManager {
private:
    Preferences preferences;
    static const char* NAMESPACE;

public:
    PreferencesManager();
    bool loadConfig(DeviceConfig& config);
    bool saveConfig(const DeviceConfig& config);
    bool saveCredentials(const String& ssid, const String& password, 
                        const String& customer_uid, const String& device_number);
    bool markAsOnboarded();
    void clearAll();
    bool hasStoredCredentials();
};

#endif
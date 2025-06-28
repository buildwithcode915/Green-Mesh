#ifndef PREFERENCES_MANAGER_H
#define PREFERENCES_MANAGER_H

#include <Preferences.h>
#include <Arduino.h>

struct DeviceConfig {
    String ssid;
    String password;
    String customer_uid;
    String device_number;
    bool isOnboarded;
    bool isFirstBoot;
};

class PreferencesManager {
private:
    Preferences preferences;
    static const char* NAMESPACE;
    static String encryptString(const String& input);
    static String decryptString(const String& input);

public:
    PreferencesManager();
    bool loadConfig(DeviceConfig& config);
    bool saveConfig(const DeviceConfig& config);
    bool saveCredentials(const String& ssid, const String& password, 
                         const String& customer_uid, const String& device_number);
    bool markAsOnboarded();
    bool markFirstBootComplete();
    bool isFirstBoot();
    void clearAll();
    bool hasStoredCredentials();
    String getDeviceNumber();
    String getCustomerUID();

};



#endif
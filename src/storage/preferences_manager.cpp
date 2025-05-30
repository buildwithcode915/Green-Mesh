#include "preferences_manager.h"

const char* PreferencesManager::NAMESPACE = "wifi";

PreferencesManager::PreferencesManager() {}

bool PreferencesManager::loadConfig(DeviceConfig& config) {
    preferences.begin(NAMESPACE, true);
    
    config.ssid = preferences.getString("ssid", "");
    config.password = preferences.getString("pass", "");
    config.customer_uid = preferences.getString("customer_uid", "");
    config.device_number = preferences.getString("device_number", "");
    config.isOnboarded = preferences.getBool("onboarded", false);
    
    preferences.end();
    
    return !config.ssid.isEmpty() && !config.password.isEmpty() && 
           !config.customer_uid.isEmpty() && !config.device_number.isEmpty();
}

bool PreferencesManager::saveConfig(const DeviceConfig& config) {
    preferences.begin(NAMESPACE, false);
    
    bool success = true;
    success &= preferences.putString("ssid", config.ssid);
    success &= preferences.putString("pass", config.password);
    success &= preferences.putString("customer_uid", config.customer_uid);
    success &= preferences.putString("device_number", config.device_number);
    success &= preferences.putBool("onboarded", config.isOnboarded);
    
    preferences.end();
    return success;
}

bool PreferencesManager::saveCredentials(const String& ssid, const String& password, 
                                       const String& customer_uid, const String& device_number) {
    DeviceConfig config;
    config.ssid = ssid;
    config.password = password;
    config.customer_uid = customer_uid;
    config.device_number = device_number;
    config.isOnboarded = false;
    
    return saveConfig(config);
}

bool PreferencesManager::markAsOnboarded() {
    preferences.begin(NAMESPACE, false);
    bool success = preferences.putBool("onboarded", true);
    preferences.end();
    return success;
}

void PreferencesManager::clearAll() {
    preferences.begin(NAMESPACE, false);
    preferences.clear();
    preferences.end();
}

bool PreferencesManager::hasStoredCredentials() {
    DeviceConfig config;
    return loadConfig(config);
}
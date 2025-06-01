#include "preferences_manager.h"

const char* PreferencesManager::NAMESPACE = "wifi";
const char* PreferencesManager::SENSOR_NAMESPACE = "sensors";

PreferencesManager::PreferencesManager() {}

bool PreferencesManager::loadConfig(DeviceConfig& config) {
    preferences.begin(NAMESPACE, true);
    
    config.ssid = preferences.getString("ssid", "");
    config.password = preferences.getString("pass", "");
    config.customer_uid = preferences.getString("customer_uid", "");
    config.device_number = preferences.getString("device_number", "");
    config.isOnboarded = preferences.getBool("onboarded", false);
    config.isFirstBoot = preferences.getBool("first_boot", true);
    
    preferences.end();
    
    // Load sensor configuration
    loadSensorConfig(config.sensorConfig);
    
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
    success &= preferences.putBool("first_boot", config.isFirstBoot);
    
    preferences.end();
    
    // Save sensor configuration
    success &= saveSensorConfig(config.sensorConfig);
    
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
    config.isFirstBoot = true;  // This will be the first boot after setup
    
    return saveConfig(config);
}

bool PreferencesManager::saveSensorConfig(const SensorConfig& sensorConfig) {
    preferences.begin(SENSOR_NAMESPACE, false);
    
    bool success = true;
    success &= preferences.putUChar("valve_count", sensorConfig.valve_count);
    success &= preferences.putUChar("flow_count", sensorConfig.flow_sensor_count);
    success &= preferences.putFloat("temperature", sensorConfig.temperature);
    success &= preferences.putBool("sensors_detected", sensorConfig.sensors_detected);
    
    preferences.end();
    return success;
}

bool PreferencesManager::loadSensorConfig(SensorConfig& sensorConfig) {
    preferences.begin(SENSOR_NAMESPACE, true);
    
    sensorConfig.valve_count = preferences.getUChar("valve_count", 0);
    sensorConfig.flow_sensor_count = preferences.getUChar("flow_count", 0);
    sensorConfig.temperature = preferences.getFloat("temperature", 0.0);
    sensorConfig.sensors_detected = preferences.getBool("sensors_detected", false);
    
    preferences.end();
    return sensorConfig.sensors_detected;
}

bool PreferencesManager::markAsOnboarded() {
    preferences.begin(NAMESPACE, false);
    bool success = preferences.putBool("onboarded", true);
    preferences.end();
    return success;
}

bool PreferencesManager::markFirstBootComplete() {
    preferences.begin(NAMESPACE, false);
    bool success = preferences.putBool("first_boot", false);
    preferences.end();
    return success;
}

bool PreferencesManager::isFirstBoot() {
    preferences.begin(NAMESPACE, true);
    bool firstBoot = preferences.getBool("first_boot", true);
    preferences.end();
    return firstBoot;
}

void PreferencesManager::clearAll() {
    preferences.begin(NAMESPACE, false);
    preferences.clear();
    preferences.end();
    
    preferences.begin(SENSOR_NAMESPACE, false);
    preferences.clear();
    preferences.end();
}

bool PreferencesManager::hasStoredCredentials() {
    DeviceConfig config;
    return loadConfig(config);
}
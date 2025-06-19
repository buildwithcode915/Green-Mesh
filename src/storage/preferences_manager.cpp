#include "preferences_manager.h"
#include "mbedtls/aes.h" // Optional, only needed for real encryption

const char* PreferencesManager::NAMESPACE = "wifi";

PreferencesManager::PreferencesManager() {}

bool PreferencesManager::loadConfig(DeviceConfig& config) {
    preferences.begin(NAMESPACE, true);

    config.ssid = preferences.getString("ssid", "");
    String encryptedPass = preferences.getString("pass", "");
    config.password = decryptString(encryptedPass);
    config.customer_uid = preferences.getString("customer_uid", "");
    config.device_number = preferences.getString("device_number", "");
    config.isOnboarded = preferences.getBool("onboarded", false);
    config.isFirstBoot = preferences.getBool("first_boot", true);

    preferences.end();

    return !config.ssid.isEmpty() && !config.password.isEmpty() &&
           !config.customer_uid.isEmpty() && !config.device_number.isEmpty();
}

bool PreferencesManager::saveConfig(const DeviceConfig& config) {
    Serial.println("Opening preferences for saving config...");
    if (!preferences.begin(NAMESPACE, false)) {
        Serial.println("Failed to begin preferences in saveConfig()");
        return false;
    }

    String encryptedPassword = encryptString(config.password);

    bool success = true;
    success &= preferences.putString("ssid", config.ssid) > 0;
    success &= preferences.putString("pass", encryptedPassword) > 0;
    success &= preferences.putString("customer_uid", config.customer_uid) > 0;
    success &= preferences.putString("device_number", config.device_number) > 0;
    success &= preferences.putBool("onboarded", config.isOnboarded);
    success &= preferences.putBool("first_boot", config.isFirstBoot);

    preferences.end();

    Serial.println(success ? "Config saved successfully" : "Failed to save one or more fields");
    return success;
}

bool PreferencesManager::saveCredentials(const String& ssid, const String& password,
                                         const String& customer_uid, const String& device_number) {
    if (ssid.length() < 1 || ssid.length() > 32 ||
        password.length() < 8 || password.length() > 63 ||
        customer_uid.isEmpty() || device_number.isEmpty()) {
        Serial.println("Invalid credentials");
        return false;
    }

    String clean_ssid = ssid;
    clean_ssid.replace("\"", "");
    clean_ssid.replace("\\", "");

    DeviceConfig config;
    config.ssid = clean_ssid;
    config.password = password;
    config.customer_uid = customer_uid;
    config.device_number = device_number;
    config.isOnboarded = false;
    config.isFirstBoot = true;

    return saveConfig(config);
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
}

bool PreferencesManager::hasStoredCredentials() {
    DeviceConfig config;
    return loadConfig(config);
}

// Dummy placeholder encryption (replace with real encryption if needed)
String PreferencesManager::encryptString(const String& input) {
    return "enc_" + input;
}

String PreferencesManager::decryptString(const String& input) {
    if (input.startsWith("enc_")) {
        return input.substring(4);
    }
    return input;
}

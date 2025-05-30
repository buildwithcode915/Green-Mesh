#include <Arduino.h>
#include "config.h"
#include "hardware/led_controller.h"
#include "hardware/button_handler.h"
#include "storage/preferences_manager.h"
#include "network/wifi_manager.h"
#include "network/api_client.h"
#include "web/web_server.h"

// Global objects
LEDController ledController;
ButtonHandler buttonHandler;
PreferencesManager prefsManager;
WiFiManager wifiManager;
APIClient apiClient;
WebServerManager webServer;

// Device configuration
DeviceConfig deviceConfig;
bool validationSuccess = false;

// Function declarations
void handleDeviceSetup();
void handleWiFiConnection();
void handleDeviceValidation();
void handleResetButton();
void onCredentialsSaved(const String& ssid, const String& password, 
                       const String& customer_uid, const String& device_number);

void setup() {
    Serial.begin(115200);
    
    // Initialize hardware
    ledController.begin();
    buttonHandler.begin();
    
    // Setup web server callbacks
    webServer.setPreferencesManager(&prefsManager);
    webServer.setLEDController(&ledController);
    webServer.setCredentialsSavedCallback(onCredentialsSaved);
    
    // Check if reset button is pressed during boot
    if (buttonHandler.isPressedDuringBoot()) {
        Serial.println("Reset button pressed. Entering setup mode.");
        prefsManager.clearAll();
        handleDeviceSetup();
        return;
    }

    // Load stored configuration
    if (!prefsManager.loadConfig(deviceConfig)) {
        Serial.println("No stored credentials. Starting AP mode.");
        handleDeviceSetup();
        return;
    }
    
    // Try to connect to stored WiFi
    handleWiFiConnection();
}

void loop() {
    // Handle web server
    webServer.handleClient();
    
    // Check reset button in normal operation mode
    if (webServer.getCurrentMode() != ServerMode::SETUP_MODE) {
        if (buttonHandler.checkForReset()) {
            Serial.println("Reset button pressed during operation. Entering AP mode.");
            prefsManager.clearAll();
            ledController.blinkReset();
            delay(1000);
            ESP.restart();
        }
    }
}

void handleDeviceSetup() {
    // Start AP mode for device configuration
    if (wifiManager.startAPMode()) {
        Serial.print("Starting AP Mode. IP: ");
        Serial.println(wifiManager.getAPIP());
        
        webServer.startSetupMode();
    } else {
        Serial.println("Failed to start AP mode");
        ledController.blinkConnectionFailed();
    }
}

void handleWiFiConnection() {
    WiFiState wifiState = wifiManager.connectToWiFi(deviceConfig.ssid, deviceConfig.password);
    
    if (wifiState == WiFiState::CONNECTED) {
        Serial.println("WiFi Connected.");
        ledController.blinkWiFiConnected();
        
        // Check internet connectivity
        if (apiClient.hasInternetConnection()) {
            ledController.blinkInternetAvailable();
            handleDeviceValidation();
        } else {
            Serial.println("No internet connection.");
            ledController.blinkConnectionFailed();
            handleDeviceSetup();
        }
    } else {
        Serial.println("WiFi Connection Failed.");
        ledController.blinkConnectionFailed();
        handleDeviceSetup();
    }
}

void handleDeviceValidation() {
    // Check if device has already been onboarded
    if (deviceConfig.isOnboarded) {
        // Device is already onboarded, skip API validation
        Serial.println("Device already onboarded. Skipping API validation.");
        validationSuccess = true;
        ledController.blinkValidationSuccess();
        
        // Start success server
        webServer.startSuccessMode(deviceConfig, wifiManager.getLocalIP());
    } else {
        // First time setup - validate with API
        Serial.println("First-time setup. Validating with API...");
        if (apiClient.validateDevice(deviceConfig.customer_uid, deviceConfig.device_number, 
                                   deviceConfig.ssid, deviceConfig.password)) {
            validationSuccess = true;
            // Mark device as onboarded to avoid future API calls
            prefsManager.markAsOnboarded();
            deviceConfig.isOnboarded = true;
            
            ledController.blinkValidationSuccess();
            Serial.println("Device successfully validated!");
            
            // Start success server to display status to user
            webServer.startSuccessMode(deviceConfig, wifiManager.getLocalIP());
        } else {
            Serial.println("Device validation failed.");
            ledController.blinkValidationFailed();
            handleDeviceSetup(); // Start AP mode if validation fails
        }
    }
}

void onCredentialsSaved(const String& ssid, const String& password, 
                       const String& customer_uid, const String& device_number) {
    // Delay to allow the page to be sent before restarting
    delay(3000);
    ESP.restart();
}
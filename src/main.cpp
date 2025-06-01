#include <Arduino.h>
#include "config.h"
#include "hardware/led_controller.h"
#include "hardware/button_handler.h"
#include "hardware/sensor_manager.h"
#include "storage/preferences_manager.h"
#include "network/wifi_manager.h"
#include "network/api_client.h"
#include "web/web_server.h"

// Global objects
LEDController ledController;
ButtonHandler buttonHandler;
SensorManager sensorManager;
PreferencesManager prefsManager;
WiFiManager wifiManager;
APIClient apiClient;
WebServerManager webServer;

// Device configuration
DeviceConfig deviceConfig;
bool validationSuccess = false;
unsigned long lastUpdateTime = 0;

// Function declarations
void handleDeviceSetup();
void handleWiFiConnection();
void handleDeviceValidation();
void handleDeviceUpdate();
void handleResetButton();
void performSensorDetection();
void onCredentialsSaved(const String& ssid, const String& password, 
                       const String& customer_uid, const String& device_number);

void setup() {
    Serial.begin(115200);
    Serial.println("Starting Green Mesh IoT Device...");
    
    // Initialize hardware
    ledController.begin();
    buttonHandler.begin();
    sensorManager.begin();
    
    // Setup web server callbacks
    webServer.setPreferencesManager(&prefsManager);
    webServer.setLEDController(&ledController);
    webServer.setCredentialsSavedCallback(onCredentialsSaved);
    
    // Check if reset button is pressed during boot
    if (buttonHandler.isPressedDuringBoot()) {
        Serial.println("Reset button pressed. Clearing all data and entering setup mode.");
        prefsManager.clearAll();
        handleDeviceSetup();
        return;
    }

    // Load stored configuration
    if (!prefsManager.loadConfig(deviceConfig)) {
        Serial.println("No stored credentials found. Starting AP mode for initial setup.");
        handleDeviceSetup();
        return;
    }
    
    // Perform sensor detection on every boot
    performSensorDetection();
    
    // Try to connect to stored WiFi
    handleWiFiConnection();
}

void loop() {
    // Handle web server
    webServer.handleClient();
    
    // Check reset button in normal operation mode
    if (webServer.getCurrentMode() != ServerMode::SETUP_MODE) {
        if (buttonHandler.checkForReset()) {
            Serial.println("Reset button pressed during operation. Resetting device...");
            prefsManager.clearAll();
            ledController.blinkReset();
            delay(1000);
            ESP.restart();
        }
    }
    
    // Periodic device updates (only after successful onboarding)
    if (validationSuccess && millis() - lastUpdateTime > DEVICE_UPDATE_INTERVAL) {
        Serial.println("Performing periodic device update...");
        sensorManager.getCurrentTemperature();  // Update temperature reading
        handleDeviceUpdate();
        lastUpdateTime = millis();
    }
    
    delay(100);  // Small delay to prevent watchdog issues
}

void handleDeviceSetup() {
    Serial.println("Entering device setup mode...");
    
    // Start AP mode for device configuration
    if (wifiManager.startAPMode()) {
        Serial.print("AP Mode started successfully. IP: ");
        Serial.println(wifiManager.getAPIP());
        
        ledController.blinkAPMode();
        webServer.startSetupMode();
    } else {
        Serial.println("Failed to start AP mode");
        ledController.blinkConnectionFailed();
    }
}

void handleWiFiConnection() {
    Serial.println("Attempting to connect to WiFi...");
    
    WiFiState wifiState = wifiManager.connectToWiFi(deviceConfig.ssid, deviceConfig.password);
    
    if (wifiState == WiFiState::CONNECTED) {
        Serial.println("WiFi Connected successfully.");
        Serial.println("IP Address: " + wifiManager.getLocalIP());
        ledController.blinkWiFiConnected();
        
        // Check internet connectivity
        if (apiClient.hasInternetConnection()) {
            Serial.println("Internet connection verified.");
            ledController.blinkInternetAvailable();
            
            // Decide between validation or update based on first boot status
            if (deviceConfig.isFirstBoot || !deviceConfig.isOnboarded) {
                handleDeviceValidation();
            } else {
                handleDeviceUpdate();
            }
        } else {
            Serial.println("No internet connection available.");
            ledController.blinkConnectionFailed();
            handleDeviceSetup();
        }
    } else {
        Serial.println("WiFi connection failed.");
        ledController.blinkConnectionFailed();
        handleDeviceSetup();
    }
}

void handleDeviceValidation() {
    Serial.println("Performing device validation (first-time setup)...");
    
    if (apiClient.validateDevice(deviceConfig.customer_uid, deviceConfig.device_number, 
                               deviceConfig.ssid, deviceConfig.password)) {
        validationSuccess = true;
        
        // Mark device as onboarded and first boot complete
        prefsManager.markAsOnboarded();
        prefsManager.markFirstBootComplete();
        deviceConfig.isOnboarded = true;
        deviceConfig.isFirstBoot = false;
        
        ledController.blinkValidationSuccess();
        Serial.println("Device validation successful!");
        
        // Perform initial device update with sensor data
        handleDeviceUpdate();
        
        // Start success server to display status to user
        webServer.startSuccessMode(deviceConfig, wifiManager.getLocalIP());
    } else {
        Serial.println("Device validation failed.");
        ledController.blinkValidationFailed();
        handleDeviceSetup(); // Return to AP mode if validation fails
    }
}

void handleDeviceUpdate() {
    Serial.println("Updating device status with current sensor data...");
    
    // Get current sensor configuration
    SensorConfig currentSensors = sensorManager.getCurrentConfig();
    
    if (apiClient.updateDeviceStatus(deviceConfig.customer_uid, deviceConfig.device_number, currentSensors)) {
        Serial.println("Device update successful.");
        
        // Save updated sensor configuration
        prefsManager.saveSensorConfig(currentSensors);
        deviceConfig.sensorConfig = currentSensors;
        
        // Indicate successful update
        ledController.setColor(0, 255, 0);  // Green solid color
        delay(1000);
        ledController.clear();
    } else {
        Serial.println("Device update failed.");
        ledController.blinkConnectionFailed();
    }
}

void performSensorDetection() {
    Serial.println("Starting sensor detection process...");
    
    if (sensorManager.detectAllSensors()) {
        Serial.println("Sensor detection completed successfully.");
        
        // Get detected configuration
        SensorConfig detectedConfig = sensorManager.getCurrentConfig();
        
        // Save sensor configuration
        prefsManager.saveSensorConfig(detectedConfig);
        deviceConfig.sensorConfig = detectedConfig;
        
        // Visual indication of sensor detection
        ledController.blinkRGB(255, 255, 0, 2);  // Yellow blinks for sensor detection
    } else {
        Serial.println("No sensors detected or detection failed.");
        ledController.blinkRGB(255, 165, 0, 2);  // Orange blinks for no sensors
    }
}

void onCredentialsSaved(const String& ssid, const String& password, 
                       const String& customer_uid, const String& device_number) {
    Serial.println("Credentials saved. Preparing for device restart...");
    
    // Update device config
    deviceConfig.ssid = ssid;
    deviceConfig.password = password;
    deviceConfig.customer_uid = customer_uid;
    deviceConfig.device_number = device_number;
    deviceConfig.isOnboarded = false;
    deviceConfig.isFirstBoot = true;
    
    // Delay to allow the connecting page to be sent
    delay(3000);
    
    Serial.println("Restarting device to apply new configuration...");
    ESP.restart();
}
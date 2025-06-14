#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
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
unsigned long lastHeartbeat = 0;
const unsigned long HEARTBEAT_INTERVAL = 30000; // 30 seconds

// Function declarations
void handleDeviceSetup();
void handleWiFiConnection();
void handleDeviceValidation();
void handleResetButton();
void onCredentialsSaved(const String& ssid, const String& password, 
                       const String& customer_uid, const String& device_number);
void performHeartbeat();
void handleOperationalMode();

void setup() {
    Serial.begin(115200);
    delay(1000); // Give serial time to initialize
    Serial.println("=== Green Mesh IoT Device Starting ===");
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    
    // Initialize hardware
    ledController.begin();
    buttonHandler.begin();
    
    // Startup LED indication
    ledController.setColor(255, 255, 0); // Yellow during startup
    
    // Setup web server callbacks
    webServer.setPreferencesManager(&prefsManager);
    webServer.setLEDController(&ledController);
    webServer.setCredentialsSavedCallback(onCredentialsSaved);
    
    // Check if reset button is pressed during boot
    if (buttonHandler.isPressedDuringBoot()) {
        Serial.println("Reset button pressed during boot. Clearing all data and entering setup mode.");
        prefsManager.clearAll();
        ledController.blinkReset();
        handleDeviceSetup();
        return;
    }

    // Load stored configuration
    if (!prefsManager.loadConfig(deviceConfig)) {
        Serial.println("No stored credentials found. Starting AP mode for initial setup.");
        handleDeviceSetup();
        return;
    }
    
    Serial.println("Found stored credentials:");
    Serial.println("SSID: " + deviceConfig.ssid);
    Serial.println("Customer UID: " + deviceConfig.customer_uid);
    Serial.println("Device Number: " + deviceConfig.device_number);
    Serial.println("Is Onboarded: " + String(deviceConfig.isOnboarded));
    Serial.println("Is First Boot: " + String(deviceConfig.isFirstBoot));
    
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

    // Handle operational tasks if device is onboarded and connected
    if (deviceConfig.isOnboarded && wifiManager.isConnected()) {
        handleOperationalMode();
    }

    // WiFi connection monitoring
    if (!wifiManager.isConnected() && deviceConfig.isOnboarded) {
        Serial.println("WiFi connection lost. Attempting to reconnect...");
        handleWiFiConnection();
    }

    delay(100);  // Small delay to prevent watchdog issues
}

void handleDeviceSetup() {
    Serial.println("=== Entering Device Setup Mode ===");
    
    // Clear any existing LED state
    ledController.clear();
    
    // Start AP mode for device configuration
    if (wifiManager.startAPMode()) {
        Serial.print("AP Mode started successfully. IP: ");
        Serial.println(wifiManager.getAPIP());
        Serial.println("Connect to WiFi: " + String(AP_SSID));
        Serial.println("Password: " + String(AP_PASSWORD));
        
        ledController.blinkAPMode();
        webServer.startSetupMode();
        
        Serial.println("Web server started for setup. Navigate to http://" + wifiManager.getAPIP());
    } else {
        Serial.println("Failed to start AP mode");
        ledController.blinkConnectionFailed();
        delay(5000);
        ESP.restart(); // Restart and try again
    }
}

void handleWiFiConnection() {
    Serial.println("=== Attempting WiFi Connection ===");
    Serial.println("Connecting to: " + deviceConfig.ssid);
    
    ledController.setColor(0, 0, 255); // Blue while connecting
    
    WiFiState wifiState = wifiManager.connectToWiFi(deviceConfig.ssid, deviceConfig.password);
    
    if (wifiState == WiFiState::CONNECTED) {
        Serial.println("WiFi Connected successfully.");
        Serial.println("IP Address: " + wifiManager.getLocalIP());
        Serial.println("Signal Strength: " + String(WiFi.RSSI()) + " dBm");
        ledController.blinkWiFiConnected();
        
        // Check internet connectivity
        if (apiClient.hasInternetConnection()) {
            Serial.println("Internet connection verified.");
            ledController.blinkInternetAvailable();
            
            // Proceed with onboarding if needed
            if (deviceConfig.isFirstBoot || !deviceConfig.isOnboarded) {
                handleDeviceValidation();
            } else {
                Serial.println("Device already onboarded. Entering operational mode.");
                ledController.setColor(0, 255, 0); // Green for operational
                webServer.startSuccessMode(deviceConfig, wifiManager.getLocalIP());
            }
        } else {
            Serial.println("No internet connection available.");
            ledController.blinkConnectionFailed();
            delay(5000);
            handleDeviceSetup();
        }
    } else {
        Serial.println("WiFi connection failed.");
        Serial.println("Reason: " + String(WiFi.status()));
        ledController.blinkConnectionFailed();
        delay(5000);
        handleDeviceSetup();
    }
}

void handleDeviceValidation() {
    Serial.println("=== Performing Device Validation ===");
    Serial.println("This is a first-time setup or re-validation.");
    
    ledController.setColor(255, 255, 0); // Yellow during validation
    
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
        Serial.println("Device is now onboarded and operational.");
        
        // Start success server to display status to user
        webServer.startSuccessMode(deviceConfig, wifiManager.getLocalIP());
        
        // Set operational LED color
        ledController.setColor(0, 255, 0); // Green for success
    } else {
        Serial.println("Device validation failed.");
        Serial.println("This could be due to:");
        Serial.println("- Invalid customer UID or device number");
        Serial.println("- Server connection issues");
        Serial.println("- Account/device not registered");
        
        ledController.blinkValidationFailed();
        delay(5000);
        handleDeviceSetup(); // Return to AP mode if validation fails
    }
}

void handleOperationalMode() {
    // Perform periodic heartbeat
    if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
        performHeartbeat();
        lastHeartbeat = millis();
    }
    
    // Add other operational tasks here
    // - Sensor readings
    // - Data transmission
    // - Status monitoring
}

void performHeartbeat() {
    if (wifiManager.isConnected()) {
        Serial.println("Heartbeat: Device operational");
        Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
        Serial.println("WiFi RSSI: " + String(WiFi.RSSI()) + " dBm");
        
        // Optional: Send heartbeat to server
        // You can implement this based on your server requirements
    }
}

void onCredentialsSaved(const String& ssid, const String& password, 
                       const String& customer_uid, const String& device_number) {
    Serial.println("=== Credentials Saved Successfully ===");
    Serial.println("New configuration:");
    Serial.println("SSID: " + ssid);
    Serial.println("Customer UID: " + customer_uid);
    Serial.println("Device Number: " + device_number);
    
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
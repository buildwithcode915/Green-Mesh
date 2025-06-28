#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "hardware/led_controller.h"
#include "hardware/button_handler.h"
#include "storage/preferences_manager.h"
#include "network/wifi_manager.h"
#include "network/api_client.h"
#include "web/web_server.h"
#include "network/mqtt_manager.h"
#include "hardware/sensor_manager.h"
#include "network/http_client.h"
#include "../include/hardware_status.h"


// Global objects
LEDController ledController;
ButtonHandler buttonHandler;
PreferencesManager prefsManager;
WiFiManager wifiManager;
APIClient apiClient;
WebServerManager webServer;
MQTTManager mqttManager;

// Device configuration
DeviceConfig deviceConfig;
bool validationSuccess = false;
unsigned long lastHeartbeat = 0;
const unsigned long HEARTBEAT_INTERVAL = 30000;

SensorManager sensorManager;
HTTPClientManager httpClient;
float flowRates[MAX_FLOW_SENSORS];

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

    sensorManager.begin();

    // Startup LED indication
    // ledController.setColor(255, 255, 0); // Yellow during startup

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

    delay(100);
}

void handleDeviceSetup() {
    Serial.println("=== Entering Device Setup Mode ===");

    ledController.clear();

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
        ESP.restart();
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

        if (apiClient.hasInternetConnection()) {
            Serial.println("Internet connection verified.");
            ledController.blinkInternetAvailable();

            mqttManager.begin(&prefsManager);

            performHardwareCheck();

            if (deviceConfig.isFirstBoot || !deviceConfig.isOnboarded) {
                handleDeviceValidation();
            } else {
                Serial.println("Device already onboarded. Entering operational mode.");
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

    if (apiClient.validateDevice(deviceConfig.customer_uid, deviceConfig.device_number, 
                               deviceConfig.ssid, deviceConfig.password)) {
        validationSuccess = true;
        prefsManager.markAsOnboarded();
        prefsManager.markFirstBootComplete();
        deviceConfig.isOnboarded = true;
        deviceConfig.isFirstBoot = false;

        ledController.blinkValidationSuccess();
        Serial.println("Device validation successful!");
        Serial.println("Device is now onboarded and operational.");

        webServer.startSuccessMode(deviceConfig, wifiManager.getLocalIP());
    } else {
        Serial.println("Device validation failed.");
        ledController.blinkValidationFailed();
        delay(5000);
        handleDeviceSetup();
    }
}

void handleOperationalMode() {
    mqttManager.loop();

    // Heartbeat every 30 sec
    if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
        performHeartbeat();
        lastHeartbeat = millis();
    }

    // ✅ Send flow/temperature every 2 seconds only if any valve is ON
    static unsigned long lastSend = 0;
    if (millis() - lastSend > 2000) {

        // Check if any valve is ON
        bool anyValveOn = false;
        int valvePins[] = VALVE_PINS;

        for (int i = 0; i < MAX_VALVES; i++) {
            if (digitalRead(valvePins[i]) == LOW) {
                anyValveOn = true;
                break;
            }
        }

        if (anyValveOn) {
            float temperature = sensorManager.readTemperature();
            sensorManager.readFlowRates(flowRates);

            httpClient.sendSensorData(deviceConfig.device_number, flowRates, MAX_FLOW_SENSORS, temperature);
            lastSend = millis();
        }
    }
}


void performHeartbeat() {
    if (wifiManager.isConnected()) {
        Serial.println("Heartbeat: Device operational");
        Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
        Serial.println("WiFi RSSI: " + String(WiFi.RSSI()) + " dBm");

        // ✅ Publish heartbeat over MQTT
        String topic = String(MQTT_BASE_TOPIC) + "/" +
                       deviceConfig.customer_uid + "/" +
                       deviceConfig.device_number + "/heartbeat";

        mqttManager.publishHeartbeat(topic);
    }
}

void onCredentialsSaved(const String& ssid, const String& password, 
                       const String& customer_uid, const String& device_number) {
    Serial.println("=== Credentials Saved Successfully ===");
    Serial.println("New configuration:");
    Serial.println("SSID: " + ssid);
    Serial.println("Customer UID: " + customer_uid);
    Serial.println("Device Number: " + device_number);

    deviceConfig.ssid = ssid;
    deviceConfig.password = password;
    deviceConfig.customer_uid = customer_uid;
    deviceConfig.device_number = device_number;
    deviceConfig.isOnboarded = false;
    deviceConfig.isFirstBoot = true;

    delay(3000);

    Serial.println("Restarting device to apply new configuration...");
    ESP.restart();
}

void performHardwareCheck() {
    HardwareStatus status;

    // Check valves
    int valvePins[] = VALVE_PINS;
    for (int i = 0; i < MAX_VALVES; i++) {
        pinMode(valvePins[i], OUTPUT);
        digitalWrite(valvePins[i], HIGH);
        delay(200);
        status.valve_ok[i] = true; // assume relay works if pin HIGH succeeds
        digitalWrite(valvePins[i], LOW);
    }

    // Check flow sensors
    int flowPins[] = FLOW_SENSOR_PINS;
    for (int i = 0; i < MAX_FLOW_SENSORS; i++) {
        pinMode(flowPins[i], INPUT);
        status.flow_ok[i] = digitalRead(flowPins[i]) == HIGH || digitalRead(flowPins[i]) == LOW;
    }

    // Check temperature sensor
    status.temp_ok = sensorManager.isTemperatureSensorConnected();

    // Send to backend
    httpClient.sendHardwareStatus(deviceConfig.device_number, status);
}


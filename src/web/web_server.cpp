#include "web_server.h"

WebServerManager::WebServerManager() : server(80), currentMode(ServerMode::STOPPED), 
                                      prefsManager(nullptr), ledController(nullptr), 
                                      onCredentialsSaved(nullptr) {}

WebServerManager::~WebServerManager() {
    stop();
}

void WebServerManager::setPreferencesManager(PreferencesManager* prefs) {
    prefsManager = prefs;
}

void WebServerManager::setLEDController(LEDController* led) {
    ledController = led;
}

void WebServerManager::setCredentialsSavedCallback(void (*callback)(const String&, const String&, const String&, const String&)) {
    onCredentialsSaved = callback;
}

bool WebServerManager::startSetupMode() {
    stop();
    
    // Setup captive portal
    dnsServer.start(DNS_PORT, "*", AP_IP_ADDR);
    
    setupAPModeRoutes();
    server.begin();
    currentMode = ServerMode::SETUP_MODE;
    
    Serial.print("Setup mode server started on IP: ");
    Serial.println(AP_IP_ADDR);
    
    return true;
}

bool WebServerManager::startSuccessMode(const DeviceConfig& config, const String& ipAddress) {
    stop();
    
    setupSuccessModeRoutes(config, ipAddress);
    server.begin();
    currentMode = ServerMode::SUCCESS_MODE;
    
    Serial.println("Success page available at http://" + ipAddress);
    
    return true;
}

void WebServerManager::stop() {
    if (currentMode != ServerMode::STOPPED) {
        server.stop();
        if (currentMode == ServerMode::SETUP_MODE) {
            dnsServer.stop();
        }
        currentMode = ServerMode::STOPPED;
    }
}

void WebServerManager::handleClient() {
    if (currentMode == ServerMode::SETUP_MODE) {
        dnsServer.processNextRequest();
    }
    if (currentMode != ServerMode::STOPPED) {
        server.handleClient();
    }
}

ServerMode WebServerManager::getCurrentMode() {
    return currentMode;
}

void WebServerManager::setupAPModeRoutes() {
    server.on("/", HTTP_GET, [this]() {
        if (ledController) {
            ledController->blinkAPMode();
        }
        server.send(200, "text/html", HTMLPages::getSetupPage());
    });

    server.on("/save", HTTP_POST, [this]() {
        String ssid = server.arg("ssid");
        String password = server.arg("password");
        String customer_uid = server.arg("customer_uid");
        String device_number = server.arg("device_number");

        // Save to preferences
        if (prefsManager) {
            prefsManager->saveCredentials(ssid, password, customer_uid, device_number);
        }

        server.send(200, "text/html", HTMLPages::getConnectingPage());

        // Call callback if set
        if (onCredentialsSaved) {
            onCredentialsSaved(ssid, password, customer_uid, device_number);
        }
    });

    // Capture all requests in AP mode to redirect to setup page
    server.onNotFound([this]() {
        server.sendHeader("Location", "/", true);
        server.send(302, "text/plain", "");
    });
}

void WebServerManager::setupSuccessModeRoutes(const DeviceConfig& config, const String& ipAddress) {
    server.on("/", HTTP_GET, [config, ipAddress, this]() {
        server.send(200, "text/html", HTMLPages::getSuccessPage(
            config.device_number, config.customer_uid, config.ssid, ipAddress));
    });
}
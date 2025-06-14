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
    // Main setup page
    server.on("/", HTTP_GET, [this]() {
        Serial.println("Serving setup page for: " + server.uri());
        if (ledController) {
            ledController->setColor(0, 255, 0); // Green when accessed
        }
        server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        server.sendHeader("Pragma", "no-cache");
        server.sendHeader("Expires", "-1");
        server.send(200, "text/html", HTMLPages::getSetupPage());
    });

    // Handle form submission
    server.on("/save", HTTP_POST, [this]() {
        Serial.println("Processing form submission...");
        
        String ssid = server.arg("ssid");
        String password = server.arg("password");
        String customer_uid = server.arg("customer_uid");
        String device_number = server.arg("device_number");

        Serial.println("Received credentials:");
        Serial.println("SSID: " + ssid);
        Serial.println("Customer UID: " + customer_uid);
        Serial.println("Device Number: " + device_number);

        // Validate input
        if (ssid.length() == 0 || password.length() == 0 || 
            customer_uid.length() == 0 || device_number.length() == 0) {
            Serial.println("Invalid input - missing required fields");
            server.send(400, "text/html", 
                "<html><body><h2>Error</h2><p>All fields are required!</p>"
                "<a href='/'>Go Back</a></body></html>");
            return;
        }

        // Save to preferences
        if (prefsManager && prefsManager->saveCredentials(ssid, password, customer_uid, device_number)) {
            Serial.println("Credentials saved successfully");
            server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
            server.send(200, "text/html", HTMLPages::getConnectingPage());

            // Call callback if set
            if (onCredentialsSaved) {
                onCredentialsSaved(ssid, password, customer_uid, device_number);
            }
        } else {
            Serial.println("Failed to save credentials");
            server.send(500, "text/html", 
                "<html><body><h2>Error</h2><p>Failed to save credentials. Please try again.</p>"
                "<a href='/'>Go Back</a></body></html>");
        }
    });

    // Handle common requests that might cause issues
    server.on("/generate_204", HTTP_GET, [this]() {
        Serial.println("Android captive portal check");
        server.sendHeader("Location", "/", true);
        server.send(302, "text/plain", "");
    });

    server.on("/fwlink", HTTP_GET, [this]() {
        Serial.println("Windows captive portal check");
        server.sendHeader("Location", "/", true);
        server.send(302, "text/plain", "");
    });

    server.on("/hotspot-detect.html", HTTP_GET, [this]() {
        Serial.println("iOS captive portal check");
        server.sendHeader("Location", "/", true);
        server.send(302, "text/plain", "");
    });

    server.on("/canonical.html", HTTP_GET, [this]() {
        Serial.println("Ubuntu captive portal check");
        server.sendHeader("Location", "/", true);
        server.send(302, "text/plain", "");
    });

    server.on("/success.txt", HTTP_GET, [this]() {
        Serial.println("Success.txt request");
        server.send(200, "text/plain", "success");
    });

    // Handle favicon requests
    server.on("/favicon.ico", HTTP_GET, [this]() {
        Serial.println("Favicon request");
        server.send(404, "text/plain", "Not found");
    });

    // Catch-all handler for any other requests - redirect to setup page
    server.onNotFound([this]() {
        String requestedUrl = server.uri();
        Serial.println("Unknown request: " + requestedUrl + " from " + server.client().remoteIP().toString());
        
        // Log the request method and headers for debugging
        Serial.println("Method: " + String((server.method() == HTTP_GET) ? "GET" : "POST"));
        Serial.println("Args: " + String(server.args()));
        for (int i = 0; i < server.args(); i++) {
            Serial.println("  " + server.argName(i) + ": " + server.arg(i));
        }
        
        // Always redirect to setup page for captive portal functionality
        server.sendHeader("Location", "/", true);
        server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        server.sendHeader("Pragma", "no-cache");
        server.sendHeader("Expires", "-1");
        server.send(302, "text/plain", "Redirecting to setup page");
    });

    Serial.println("AP mode routes configured");
}

void WebServerManager::setupSuccessModeRoutes(const DeviceConfig& config, const String& ipAddress) {
    server.on("/", HTTP_GET, [config, ipAddress, this]() {
        Serial.println("Serving success page");
        server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        server.send(200, "text/html", HTMLPages::getSuccessPage(
            config.device_number, config.customer_uid, config.ssid, ipAddress));
    });

    // Handle status endpoint
    server.on("/status", HTTP_GET, [config, ipAddress, this]() {
        Serial.println("Status request");
        String status = "{";
        status += "\"device_number\":\"" + config.device_number + "\",";
        status += "\"customer_uid\":\"" + config.customer_uid + "\",";
        status += "\"ssid\":\"" + config.ssid + "\",";
        status += "\"ip_address\":\"" + ipAddress + "\",";
        status += "\"onboarded\":" + String(config.isOnboarded ? "true" : "false") + ",";
        status += "\"heap_free\":" + String(ESP.getFreeHeap()) + ",";
        status += "\"wifi_rssi\":" + String(WiFi.RSSI());
        status += "}";
        
        server.send(200, "application/json", status);
    });

    // Catch-all for success mode
    server.onNotFound([this]() {
        Serial.println("Unknown request in success mode: " + server.uri());
        server.sendHeader("Location", "/", true);
        server.send(302, "text/plain", "Redirecting to status page");
    });

    Serial.println("Success mode routes configured");
}
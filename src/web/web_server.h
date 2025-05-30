#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>
#include <DNSServer.h>
#include "html_pages.h"
#include "../storage/preferences_manager.h"
#include "../hardware/led_controller.h"
#include "config.h"

enum class ServerMode {
    SETUP_MODE,
    SUCCESS_MODE,
    STOPPED
};

class WebServerManager {
private:
    WebServer server;
    DNSServer dnsServer;
    ServerMode currentMode;
    PreferencesManager* prefsManager;
    LEDController* ledController;
    
    // Callback function pointers
    void (*onCredentialsSaved)(const String&, const String&, const String&, const String&);
    
public:
    WebServerManager();
    ~WebServerManager();
    
    void setPreferencesManager(PreferencesManager* prefs);
    void setLEDController(LEDController* led);
    void setCredentialsSavedCallback(void (*callback)(const String&, const String&, const String&, const String&));
    
    bool startSetupMode();
    bool startSuccessMode(const DeviceConfig& config, const String& ipAddress);
    void stop();
    void handleClient();
    
    ServerMode getCurrentMode();
    
private:
    void setupAPModeRoutes();
    void setupSuccessModeRoutes(const DeviceConfig& config, const String& ipAddress);
};

#endif
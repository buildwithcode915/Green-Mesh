#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <Adafruit_NeoPixel.h>
#include <DNSServer.h>

// Define hardware pins
#define RGB_LED_PIN 8
#define NUMPIXELS 1
#define RESET_BUTTON_PIN 0

// NeoPixel setup
Adafruit_NeoPixel pixels(NUMPIXELS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);

// Preferences for persistent storage
Preferences preferences;

// Web server for AP mode
WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

// Configuration variables
String ssid, password, customer_uid, device_number;
const char* AP_SSID = "Green Mesh";
const char* AP_PASSWORD = "Admin@123456";
bool setupMode = false;
bool validationSuccess = false;
bool isDeviceOnboarded = false;

// API endpoints
const char* API_ENDPOINT = "http://192.168.31.156:8000/api/device/onboard";
const char* Device_update = "http://192.168.31.156:8000/api/device/update";
const char* INTERNET_CHECK = "http://clients3.google.com/generate_204";

// AP mode IP configuration
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// Function declarations
void blinkRGB(uint8_t r, uint8_t g, uint8_t b, int times);
void startAPMode();
void handleReset();
bool validateDevice();
bool hasInternet();
void startSuccessServer();

void setup() {
  Serial.begin(115200);
  
  // Initialize LED
  pixels.begin();
  pixels.clear();
  pixels.show();
  
  // Initialize reset button with pull-up resistor
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  
  // Retrieve stored configuration
  preferences.begin("wifi", true);
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("pass", "");
  customer_uid = preferences.getString("customer_uid", "");
  device_number = preferences.getString("device_number", "");
  isDeviceOnboarded = preferences.getBool("onboarded", false);
  preferences.end();
  
  // Check if reset button is pressed during boot
  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    Serial.println("Reset button pressed. Entering setup mode.");
    // Clear stored credentials
    preferences.begin("wifi", false);
    preferences.clear();
    preferences.end();
    startAPMode();
    return;
  }

  // Check if we have stored credentials
  if (ssid.isEmpty() || password.isEmpty() || customer_uid.isEmpty() || device_number.isEmpty()) {
    Serial.println("No stored credentials. Starting AP mode.");
    startAPMode();
    return;
  }
  
  // Try to connect to the stored WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.println("Connecting to WiFi...");

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected.");
    blinkRGB(0, 0, 255, 5); // Blue blink - WiFi connected
    
    // Check internet connectivity
    if (hasInternet()) {
      blinkRGB(0, 255, 0, 5); // Green blink - Internet available
      
      // Check if device has already been onboarded
      if (isDeviceOnboarded) {
        // Device is already onboarded, skip API validation
        Serial.println("Device already onboarded. Skipping API validation.");
        validationSuccess = true;
        blinkRGB(255, 0, 255, 3); // Yellow blink - Already validated
        startSuccessServer();
      } else {
        // First time setup - validate with API
        Serial.println("First-time setup. Validating with API...");
        if (validateDevice()) {
          validationSuccess = true;
          // Mark device as onboarded to avoid future API calls
          preferences.begin("wifi", false);
          preferences.putBool("onboarded", true);
          preferences.end();
          
          blinkRGB(255, 255, 0, 3); // Yellow blink - Validation successful
          Serial.println("Device successfully validated!");
          
          // Start success server to display status to user
          startSuccessServer();
        } else {
          Serial.println("Device validation failed.");
          blinkRGB(255, 0, 0, 3); // Red blink - Validation failed
          startAPMode(); // Start AP mode if validation fails
        }
      }
    } else {
      Serial.println("No internet connection.");
      blinkRGB(255, 0, 0, 5); // Red blink - No internet
      startAPMode();
    }
  } else {
    Serial.println("WiFi Connection Failed.");
    blinkRGB(255, 0, 0, 5); // Red blink - WiFi connection failed
    startAPMode();
  }
}

void loop() {
  // Handle AP mode web server
  if (setupMode) {
    dnsServer.processNextRequest();
    server.handleClient();
  } else if (validationSuccess) {
    // Handle success server
    server.handleClient();
  }
  
  // Check reset button in normal operation mode
  if (!setupMode && digitalRead(RESET_BUTTON_PIN) == LOW) {
    delay(100); // Debounce
    if (digitalRead(RESET_BUTTON_PIN) == LOW) {
      Serial.println("Reset button pressed during operation. Entering AP mode.");
      handleReset();
    }
  }
}

void startAPMode() {
  setupMode = true;
  
  // Disconnect from any previous WiFi
  WiFi.disconnect();
  delay(100);
  
  // Configure AP
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  Serial.print("Starting AP Mode. IP: ");
  Serial.println(WiFi.softAPIP());

  // Setup captive portal
  dnsServer.start(DNS_PORT, "*", local_IP);

  // Setup web server routes
  server.on("/", HTTP_GET, []() {
    blinkRGB(0, 255, 0, 3); // Blink green when user connects to AP
    String html = R"rawliteral(
      <html><head><title>Green Mesh Setup</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body { font-family: Arial; background: #f4f4f4; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }
        .form-container { background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1); width: 300px; }
        h2 { text-align: center; color: #333; }
        input, button { width: 100%; padding: 10px; margin: 8px 0; border-radius: 5px; box-sizing: border-box; }
        input { border: 1px solid #ddd; }
        button { background: #28a745; color: white; border: none; cursor: pointer; font-weight: bold; }
        button:hover { background: #218838; }
        .status { text-align: center; margin-top: 10px; }
      </style></head>
      <body><div class='form-container'>
      <h2>Green Mesh Setup</h2>
      <form action="/save" method="POST">
        <input type="text" name="ssid" placeholder="WiFi SSID" required><br>
        <input type="password" name="password" placeholder="WiFi Password" required><br>
        <input type="text" name="customer_uid" placeholder="User ID" required><br>
        <input type="text" name="device_number" placeholder="Device ID" required><br>
        <button type="submit">Save & Connect</button>
      </form>
      <div class="status"></div>
      </div></body></html>
    )rawliteral";
    server.send(200, "text/html", html);
  });

  server.on("/save", HTTP_POST, []() {
    ssid = server.arg("ssid");
    password = server.arg("password");
    customer_uid = server.arg("customer_uid");
    device_number = server.arg("device_number");

    // Save to preferences
    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("pass", password);
    preferences.putString("customer_uid", customer_uid);
    preferences.putString("device_number", device_number);
    preferences.putBool("onboarded", false); // Reset onboarded status
    preferences.end();

    server.send(200, "text/html", R"rawliteral(
      <html><head><title>Connecting...</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body { font-family: Arial; text-align: center; margin-top: 50px; background: #f4f4f4; }
        .container { background: white; max-width: 400px; margin: 0 auto; padding: 30px; border-radius: 10px; box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1); }
        .loader { border: 5px solid #f3f3f3; border-top: 5px solid #28a745; border-radius: 50%; width: 50px; height: 50px; animation: spin 2s linear infinite; margin: 20px auto; }
        @keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }
      </style>
      </head>
      <body>
        <div class="container">
          <h2>Settings Saved!</h2>
          <p>The device will now restart and attempt to connect to your WiFi.</p>
          <div class="loader"></div>
          <p>This page will automatically close. Please reconnect to your home WiFi.</p>
        </div>
      </body></html>
    )rawliteral");

    // Delay to allow the page to be sent before restarting
    delay(3000);
    ESP.restart();
  });

  // Capture all requests in AP mode to redirect to setup page
  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  // Start web server
  server.begin();
}

void startSuccessServer() {
  // Disconnect from any previous WiFi
  server.stop();
  delay(100);
  
  // We're already connected to WiFi in STA mode
  // Just configure and start the web server
  
  Serial.print("Starting Success Server. IP: ");
  Serial.println(WiFi.localIP());

  // Setup web server routes for success page
  server.on("/", HTTP_GET, []() {
    String html = R"rawliteral(
      <html><head><title>Green Mesh Setup Complete</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body { font-family: Arial; background: #f4f4f4; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }
        .success-container { background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1); width: 350px; text-align: center; }
        h2 { color: #28a745; }
        .checkmark { color: #28a745; font-size: 80px; margin: 10px 0; }
        .details { background: #f8f9fa; padding: 15px; border-radius: 5px; margin: 20px 0; text-align: left; }
        .details p { margin: 5px 0; }
        .device-info { font-weight: bold; }
      </style></head>
      <body><div class='success-container'>
      <div class="checkmark">✓</div>
      <h2>Setup Complete!</h2>
      <p>Your Green Mesh device is successfully connected and activated.</p>
      <div class="details">
        <p><span class="device-info">Device ID:</span> )rawliteral" + device_number + R"rawliteral(</p>
        <p><span class="device-info">User ID:</span> )rawliteral" + customer_uid + R"rawliteral(</p>
        <p><span class="device-info">WiFi Network:</span> )rawliteral" + ssid + R"rawliteral(</p>
        <p><span class="device-info">IP Address:</span> )rawliteral" + WiFi.localIP().toString() + R"rawliteral(</p>
      </div>
      <p>You can now close this page and start using your Green Mesh device.</p>
      <p>If you need to reconfigure, press the reset button on the device.</p>
      </div></body></html>
    )rawliteral";
    server.send(200, "text/html", html);
  });

  // Start web server
  server.begin();
  
  // Print access instructions
  Serial.println("Success page available at http://" + WiFi.localIP().toString());
}

void handleReset() {
  // Clear stored preferences
  preferences.begin("wifi", false);
  preferences.clear();
  preferences.end();
  
  // Blink red to indicate reset
  blinkRGB(255, 0, 0, 3);

  // Restart the device to enter AP setup mode
  delay(1000);
  ESP.restart();
}


bool validateDevice() {
  Serial.println("Validating device...");
  HTTPClient http;
  
  // Configure timeout for the HTTP client (in milliseconds)
  http.setTimeout(10000);  // 10 second timeout
  
  // Start the HTTP connection
  http.begin(API_ENDPOINT);
  http.addHeader("Content-Type", "application/json");

  // Create JSON payload with the required parameters
  String json = "{\"uid\":\"" + customer_uid + 
                "\",\"device_number\":\"" + device_number + 
                "\",\"ssid\":\"" + ssid + 
                "\",\"wifi_password\":\"" + password + "\"}";
  
  Serial.println("Sending validation request: " + json);
  
  // Send POST request
  int httpCode = http.POST(json);
  
  // Check for successful response
  if (httpCode > 0) {
    String response = http.getString();
    Serial.print("HTTP Response code: ");
    Serial.println(httpCode);
    Serial.println("Response: " + response);
    
    // 200 OK means validation successful
    if (httpCode == 200) {
      http.end();
      return true;
    }
  } else {
    Serial.print("HTTP Request failed, error: ");
    Serial.println(http.errorToString(httpCode));
  }
  
  http.end();
  return false;
}

bool hasInternet() {
  Serial.println("Checking internet connection...");
  HTTPClient http;
  http.begin(INTERNET_CHECK);
  
  // Set timeout for internet check (in milliseconds)
  http.setTimeout(10000);  // 10 second timeout
  
  int httpCode = http.GET();
  Serial.print("Internet check response: ");
  Serial.println(httpCode);
  
  http.end();
  return (httpCode == 204);
}

void blinkRGB(uint8_t r, uint8_t g, uint8_t b, int times) {
  for (int i = 0; i < times; i++) {
    pixels.setPixelColor(0, pixels.Color(r, g, b));
    pixels.show();
    delay(300);
    pixels.clear();
    pixels.show();
    delay(300);
  }
}
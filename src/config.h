#ifndef CONFIG_H
#define CONFIG_H

// Hardware Configuration
#define RGB_LED_PIN 8
#define NUMPIXELS 1
#define RESET_BUTTON_PIN 0

// Network Configuration
#define AP_SSID "Green Mesh"
#define AP_PASSWORD "Admin@123456"
#define DNS_PORT 53

// API Configuration
#define API_ENDPOINT "http://192.168.31.156:8000/api/device/onboard"
#define DEVICE_UPDATE_ENDPOINT "http://192.168.31.156:8000/api/device/update"
#define INTERNET_CHECK_URL "http://clients3.google.com/generate_204"

// Network Timeouts
#define HTTP_TIMEOUT 10000
#define WIFI_RETRY_COUNT 20

// AP Mode IP Configuration
#define AP_IP_ADDR IPAddress(192, 168, 4, 1)
#define AP_GATEWAY IPAddress(192, 168, 4, 1)
#define AP_SUBNET IPAddress(255, 255, 255, 0)

#endif
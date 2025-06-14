#ifndef CONFIG_H
#define CONFIG_H

// Hardware Configuration
#ifndef RGB_LED_PIN
#define RGB_LED_PIN 8
#endif
#define NUMPIXELS 1
#define RESET_BUTTON_PIN 9

#define MAX_VALVES 4
#define MAX_FLOW_SENSORS 4

#define VALVE_PINS {2, 3, 4, 5}
#define FLOW_SENSOR_PINS {6, 7, 10, 18}
#define TEMP_SENSOR_PIN 1

#define ONE_WIRE_BUS TEMP_SENSOR_PIN

// Network Configuration
#define AP_SSID "Green Mesh"
#define AP_PASSWORD "Admin@123456"
#define DNS_PORT 53

// API Configuration
#define API_ENDPOINT "http://192.168.31.156:8000/api/device/onboard"
#define CONNECTIVITY_CHECK_URL "http://httpbin.org/get"

// Network Timeouts
#define HTTP_TIMEOUT 10000
#define WIFI_RETRY_COUNT 20

// AP Mode IP Configuration
#define AP_IP_ADDR IPAddress(192, 168, 4, 1)
#define AP_GATEWAY IPAddress(192, 168, 4, 1)
#define AP_SUBNET IPAddress(255, 255, 255, 0)

#endif
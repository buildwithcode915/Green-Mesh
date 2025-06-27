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
#define API_ENDPOINT "http://127.0.0.1:8000//api/device/onboard"
#define CONNECTIVITY_CHECK_URL "http://httpbin.org/get"
#define DEVICE_STATUS_BASE_URL "http://127.0.0.1:8000//api/uid/device-status?device_number="

// MQTT Configuration
#define MQTT_BROKER "90fee3b7e91445c59e86c74d7c7                                                            03685.s1.eu.hivemq.cloud"
#define MQTT_PORT 8883
#define MQTT_USER "navdeep"
#define MQTT_PASSWORD "Raushan@434"
#define MQTT_BASE_TOPIC "/greenmesh"


// Network Timeouts
#define HTTP_TIMEOUT 10000
#define WIFI_RETRY_COUNT 20

// AP Mode IP Configuration
#define AP_IP_ADDR IPAddress(192, 168, 4, 1)
#define AP_GATEWAY IPAddress(192, 168, 4, 1)
#define AP_SUBNET IPAddress(255, 255, 255, 0)

#endif
#ifndef CONFIG_H
#define CONFIG_H

// Hardware Configuration
#ifndef RGB_LED_PIN
#define RGB_LED_PIN 8
#endif
       // Built-in RGB LED on ESP32-C3 DevKitM-1
#define NUMPIXELS 1
#define RESET_BUTTON_PIN 9      // Boot button on ESP32-C3 DevKitM-1

// Update the MAX values to reflect your actual requirements
#define MAX_VALVES 4            // Your requirement: 4 valves
#define MAX_FLOW_SENSORS 4      // Your requirement: 4 flow sensors




#define VALVE_PINS {2, 3, 4, 5}                         
#define FLOW_SENSOR_PINS {6, 7, 10, 18}           
#define TEMP_SENSOR_PIN 8                               

#define ONE_WIRE_BUS TEMP_SENSOR_PIN

// Sensor Detection Settings
#define SENSOR_DETECTION_DELAY 100  // ms between sensor checks
#define SENSOR_STABLE_COUNT 5       // readings needed for stable detection

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

// Update Intervals
#define DEVICE_UPDATE_INTERVAL 300000  // 5 minutes in milliseconds

// ADC Configuration for Temperature Sensor
#define ADC_RESOLUTION 12               // 12-bit ADC resolution (0-4095)
#define ADC_VREF 3300                   // Reference voltage in mV
#define TEMP_SENSOR_SAMPLES 10          // Number of samples to average for stable reading

// Flow Sensor Configuration
#define FLOW_PULSE_TIMEOUT 5000         // Timeout for flow pulse detection (ms)
#define FLOW_CALIBRATION_FACTOR 7.5     // Pulses per liter (adjust based on your sensor)

// Valve Configuration
#define VALVE_OPERATION_DELAY 500       // Delay between valve operations (ms)
#define VALVE_STATE_CHANGE_DEBOUNCE 50  // Debounce time for valve state changes (ms)

#endif
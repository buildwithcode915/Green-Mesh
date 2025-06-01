#ifndef CONFIG_H
#define CONFIG_H

// Hardware Configuration
#define RGB_LED_PIN 8           // Built-in RGB LED on ESP32-C3 DevKitM-1
#define NUMPIXELS 1
#define RESET_BUTTON_PIN 9      // Boot button on ESP32-C3 DevKitM-1

// Update the MAX values to reflect your actual requirements
#define MAX_VALVES 4            // Your requirement: 4 valves
#define MAX_FLOW_SENSORS 4      // Your requirement: 4 flow sensors

// GPIO Pin Assignments for ESP32-C3 DevKitM-1
// Available GPIO pins: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 18, 19, 20, 21 (15 pins total)
// Special functions to consider:
// - GPIO8: Built-in RGB LED (WS2812) - already used
// - GPIO9: Boot button - already used
// - GPIO18, 19: USB D+/D- - can be used but avoid if using USB serial for debugging
// - GPIO20, 21: I2C SDA/SCL default pins
// - ADC1 capable pins: GPIO0, 1, 2, 3, 4 (for analog sensors like temperature)

// Optimized Pin allocation for 4 valves + 4 flow sensors + 1 temp sensor = 9 pins needed
// Total available after RGB LED and Boot button: 13 pins remaining - plenty of headroom!

#define VALVE_PINS {2, 3, 4, 5}                          // 4 valve control pins (digital output)
#define FLOW_SENSOR_PINS {6, 7, 10, 18}                  // 4 flow sensor pins (digital input with interrupts)
#define TEMP_SENSOR_PIN 0                                 // Temperature sensor (ADC1_CH0 - analog input)

// Remaining available pins for future expansion: 1, 19, 20, 21
// GPIO20, 21 are default I2C pins if you need to add I2C sensors later
// GPIO1, 19 are available for additional sensors or features

// Pin Function Summary:
// GPIO0:  Temperature sensor (ADC input)
// GPIO1:  Available for expansion
// GPIO2:  Valve 1 (Digital output)
// GPIO3:  Valve 2 (Digital output)  
// GPIO4:  Valve 3 (Digital output)
// GPIO5:  Valve 4 (Digital output)
// GPIO6:  Flow sensor 1 (Digital input)
// GPIO7:  Flow sensor 2 (Digital input)
// GPIO8:  RGB LED (WS2812)
// GPIO9:  Boot button
// GPIO10: Flow sensor 3 (Digital input)
// GPIO18: Flow sensor 4 (Digital input)
// GPIO19: Available for expansion
// GPIO20: Available (default I2C SDA)
// GPIO21: Available (default I2C SCL)

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

// ESP32-C3 DevKitM-1 Optimized Configuration Notes:
// ✅ Perfect fit: 9 pins needed, 13 available after system pins
// ✅ Temperature sensor on ADC-capable pin (GPIO0)
// ✅ Flow sensors on interrupt-capable pins for accurate pulse counting
// ✅ Valves on standard digital output pins
// ✅ I2C pins (20, 21) reserved for future expansion
// ✅ USB pins (18, 19) used efficiently - GPIO18 for flow sensor, GPIO19 available
// ✅ No pin conflicts or special function interference
//
// Future Expansion Options:
// - Add I2C sensors using GPIO20 (SDA) and GPIO21 (SCL)
// - Add more digital sensors on GPIO1 and GPIO19
// - Consider using interrupts for flow sensors for better accuracy

#endif
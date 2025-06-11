#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Adafruit_NeoPixel.h>
#include "config.h"

class LEDController {
private:
    Adafruit_NeoPixel pixels;

public:
    LEDController();
    void begin();
    void blinkRGB(uint8_t r, uint8_t g, uint8_t b, int times);
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void clear();
    
    // Predefined color patterns
    void blinkWiFiConnected();
    void blinkInternetAvailable();
    void blinkValidationSuccess();
    void blinkValidationFailed();
    void blinkConnectionFailed();
    void blinkAPMode();
    void blinkReset();
};

#endif
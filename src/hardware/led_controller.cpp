#include "led_controller.h"

LEDController::LEDController() : pixels(NUMPIXELS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800) {}

void LEDController::begin() {
    pixels.begin();
    pixels.clear();
    pixels.show();
}

void LEDController::blinkRGB(uint8_t r, uint8_t g, uint8_t b, int times) {
    for (int i = 0; i < times; i++) {
        pixels.setPixelColor(0, pixels.Color(r, g, b));
        pixels.show();
        delay(300);
        pixels.clear();
        pixels.show();
        delay(300);
    }
}

void LEDController::setColor(uint8_t r, uint8_t g, uint8_t b) {
    pixels.setPixelColor(0, pixels.Color(r, g, b));
    pixels.show();
}

void LEDController::clear() {
    pixels.clear();
    pixels.show();
}

// Predefined patterns
void LEDController::blinkWiFiConnected() {
    blinkRGB(0, 0, 255, 5); // Blue blink
}

void LEDController::blinkInternetAvailable() {
    blinkRGB(0, 255, 0, 5); // Green blink
}

void LEDController::blinkValidationSuccess() {
    blinkRGB(255, 255, 0, 3); // Yellow blink
}

void LEDController::blinkValidationFailed() {
    blinkRGB(255, 0, 0, 3); // Red blink
}

void LEDController::blinkConnectionFailed() {
    blinkRGB(255, 0, 0, 5); // Red blink
}

void LEDController::blinkAPMode() {
    blinkRGB(0, 255, 0, 3); // Green blink
}

void LEDController::blinkReset() {
    blinkRGB(255, 0, 0, 3); // Red blink
}

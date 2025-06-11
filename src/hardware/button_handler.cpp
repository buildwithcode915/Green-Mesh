#include "button_handler.h"

ButtonHandler::ButtonHandler() : lastDebounceTime(0), lastButtonState(HIGH), buttonState(HIGH) {}

void ButtonHandler::begin() {
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
}

bool ButtonHandler::isPressed() {
    return digitalRead(RESET_BUTTON_PIN) == LOW;
}

bool ButtonHandler::isPressedDuringBoot() {
    return digitalRead(RESET_BUTTON_PIN) == LOW;
}

bool ButtonHandler::checkForReset() {
    bool reading = digitalRead(RESET_BUTTON_PIN) == LOW;
    
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != buttonState) {
            buttonState = reading;
            if (buttonState) {
                lastButtonState = reading;
                return true;
            }
        }
    }
    
    lastButtonState = reading;
    return false;
}
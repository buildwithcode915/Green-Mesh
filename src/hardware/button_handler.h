#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>
#include "config.h"

class ButtonHandler {
private:
    unsigned long lastDebounceTime;
    bool lastButtonState;
    bool buttonState;
    static const unsigned long debounceDelay = 100;

public:
    ButtonHandler();
    void begin();
    bool isPressed();
    bool isPressedDuringBoot();
    bool checkForReset();
};

#endif
#ifndef HTML_PAGES_H
#define HTML_PAGES_H

#include <Arduino.h>

class HTMLPages {
public:
    static String getSetupPage();
    static String getConnectingPage();
    static String getSuccessPage(const String& deviceNumber, const String& customerUid, 
                                const String& ssid, const String& ipAddress);
};

#endif
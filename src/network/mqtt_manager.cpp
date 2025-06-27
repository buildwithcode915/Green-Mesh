#include "mqtt_manager.h"

MQTTManager::MQTTManager() : client(wifiClient) {}

void MQTTManager::begin(PreferencesManager* preferences) {
    prefs = preferences;

    // ✅ Properly copy GPIOs into valvePins[]
    const int pins[MAX_VALVES] = VALVE_PINS;
    memcpy(valvePins, pins, sizeof(pins));

    // ✅ Initialize valve GPIOs
    for (int i = 0; i < MAX_VALVES; ++i) {
        pinMode(valvePins[i], OUTPUT);
        digitalWrite(valvePins[i], HIGH);  // Start OFF
        Serial.printf("Valve %d initialized on GPIO %d\n", i + 1, valvePins[i]);
    }

    wifiClient.setInsecure(); // TLS for HiveMQ (dev mode)
    client.setServer(MQTT_BROKER, MQTT_PORT);

    client.setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->handleMessage(topic, payload, length);
    });

    String uid = prefs->getCustomerUID();
    String deviceNumber = prefs->getDeviceNumber();

    deviceTopic = String(MQTT_BASE_TOPIC) + "/" + uid + "/" + deviceNumber + "/control";
    Serial.println("Subscribing to MQTT topic: " + deviceTopic);
}

void MQTTManager::reconnect() {
    while (!client.connected()) {
        if (client.connect(prefs->getDeviceNumber().c_str(), MQTT_USER, MQTT_PASSWORD)) {
            Serial.println("MQTT Connected");
            subscribeToTopic();
        } else {
            Serial.print("MQTT Failed. State: ");
            Serial.println(client.state());
            delay(5000);
        }
    }
}

void MQTTManager::subscribeToTopic() {
    Serial.println("Subscribing to: " + deviceTopic);
    client.subscribe(deviceTopic.c_str());
}

void MQTTManager::loop() {
    if (!client.connected()) reconnect();
    client.loop();
}

void MQTTManager::handleMessage(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (int i = 0; i < length; i++) msg += (char)payload[i];
    Serial.println("MQTT Message: " + msg);

    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, msg)) {
        Serial.println("❌ JSON parse failed");
        return;
    }

    int valve = doc["valve_number"];
    String action = doc["action"];

    if (valve >= 1 && valve <= MAX_VALVES) {
        int pin = valvePins[valve - 1];
        digitalWrite(pin, (action == "on") ? LOW : HIGH);
        Serial.printf("✅ Valve %d (GPIO %d) turned %s\n", valve, pin, action.c_str());
    } else {
        Serial.println("⚠️ Invalid valve_number received");
    }
}

void MQTTManager::publishHeartbeat(const String& topic) {
    if (client.connected()) {
        client.publish(topic.c_str(), "online", true);
        Serial.println("📡 MQTT heartbeat published");
    }
}


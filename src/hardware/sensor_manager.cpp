#include "sensor_manager.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define TOTAL_SENSORS 4
int flowPins[TOTAL_SENSORS] = {6, 7, 10, 18};
volatile int flowCounts[TOTAL_SENSORS] = {0};

void IRAM_ATTR onFlow0() { flowCounts[0]++; }
void IRAM_ATTR onFlow1() { flowCounts[1]++; }
void IRAM_ATTR onFlow2() { flowCounts[2]++; }
void IRAM_ATTR onFlow3() { flowCounts[3]++; }

void (*flowInterrupts[])() = {onFlow0, onFlow1, onFlow2, onFlow3};

OneWire oneWire(1); // TEMP_SENSOR_PIN
DallasTemperature sensors(&oneWire);

void SensorManager::begin() {
    gpio_install_isr_service(0);

    for (int i = 0; i < TOTAL_SENSORS; i++) {
        pinMode(flowPins[i], INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(flowPins[i]), flowInterrupts[i], RISING);
    }

    sensors.begin();
}


void SensorManager::readFlowRates(float rates[]) {
    for (int i = 0; i < TOTAL_SENSORS; i++) {
        detachInterrupt(digitalPinToInterrupt(flowPins[i]));
        rates[i] = flowCounts[i] / 7.5;
        flowCounts[i] = 0;
        attachInterrupt(digitalPinToInterrupt(flowPins[i]), flowInterrupts[i], RISING);
    }
}

float SensorManager::readTemperature() {
    sensors.requestTemperatures();
    return sensors.getTempCByIndex(0);
}

bool SensorManager::isTemperatureSensorConnected() {
    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);
    return (temp != DEVICE_DISCONNECTED_C && temp > -55 && temp < 125);
}

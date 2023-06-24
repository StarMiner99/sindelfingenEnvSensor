//
// Created by thijs on 6/24/23.
//

#ifndef SINDELFINGENENVSENSOR_ENVSENSOR_H
#define SINDELFINGENENVSENSOR_ENVSENSOR_H


#include <cstdint>
#include "Adafruit_BME280.h"
#include "OneWire.h"
#include <DallasTemperature.h>

class EnvSensor {
private:
    Adafruit_BME280* bme;
    DallasTemperature* tempSensor;

    uint8_t bmeAddr;
    int moisturePin;
public:
    EnvSensor(int owTempPin, int moistureSensorPin, uint8_t bmeAddr);
    bool begin();

    float getOWTemperature();

    int getMoisture();

    float getTemperature();
    float getPressure();
    float getHumidity();


};


#endif //SINDELFINGENENVSENSOR_ENVSENSOR_H

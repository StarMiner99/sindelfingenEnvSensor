//
// Created by thijs on 6/24/23.
//

#ifndef SINDELFINGENENVSENSOR_ENVSENSOR_H
#define SINDELFINGENENVSENSOR_ENVSENSOR_H


#include <cstdint>
#include "Adafruit_BME280.h"
#include "OneWire.h"
#include <DallasTemperature.h>
#include "Config.h"

class EnvSensor {
private:
    Adafruit_BME280* bme;
    DallasTemperature* tempSensor;

public:
    EnvSensor();
    bool begin();

    float getOWTemperature();

    int getMoisture();

    float getTemperature();
    float getPressure();
    float getHumidity();


};


#endif //SINDELFINGENENVSENSOR_ENVSENSOR_H

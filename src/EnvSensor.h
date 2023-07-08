//
// Created by thijs on 6/24/23.
//

#ifndef SINDELFINGENENVSENSOR_ENVSENSOR_H
#define SINDELFINGENENVSENSOR_ENVSENSOR_H


#include <cstdint>

#include <OneWire.h>
#include <DallasTemperature.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "Config.h"

class EnvSensor {
private:
    Adafruit_BME280* bme;
    MbedI2C *i2c;
    //DallasTemperature* tempSensor;

public:
    EnvSensor();
    bool begin();

   // float getOWTemperature();

    int getMoisture();

    float getTemperature();
    float getPressure();
    float getHumidity();


};


#endif //SINDELFINGENENVSENSOR_ENVSENSOR_H

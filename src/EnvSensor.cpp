//
// Created by thijs on 6/24/23.
//

#include "EnvSensor.h"

EnvSensor::EnvSensor() {
   // OneWire oneWireTemp(OW_TEMP_PIN);
   // EnvSensor::tempSensor = new DallasTemperature(&oneWireTemp);

    EnvSensor::bme = new Adafruit_BME280();
    i2c = new MbedI2C(I2C_SDA_PIN,I2C_SCL_PIN);
}

bool EnvSensor::begin() {
    bool success;
    //tempSensor->begin();
    success = bme->begin(BME_ADDR, i2c);

    return success;
}
/*
float EnvSensor::getOWTemperature() {
    tempSensor->requestTemperatures();
    return tempSensor->getTempCByIndex(0);
}
 */

int EnvSensor::getMoisture() {
    return analogRead(MOISTURE_SENSOR_PIN);
}

float EnvSensor::getTemperature() {
    return bme->readTemperature();
}

float EnvSensor::getPressure() {
    return bme->readPressure();
}

float EnvSensor::getHumidity() {
    return bme->readHumidity();
}

void EnvSensor::sleep() {
    bme->setSampling(Adafruit_BME280::MODE_SLEEP);
}

void EnvSensor::wakeUp() {
    bme->setSampling(Adafruit_BME280::MODE_NORMAL);
}

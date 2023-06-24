//
// Created by thijs on 6/24/23.
//

#include "EnvSensor.h"

EnvSensor::EnvSensor(int owTempPin, int moistureSensorPin, uint8_t bmeAddr) {
    OneWire oneWireTemp(owTempPin);
    EnvSensor::tempSensor = new DallasTemperature(&oneWireTemp);

    EnvSensor::bme = new Adafruit_BME280();
    EnvSensor::bmeAddr = bmeAddr;

    EnvSensor::moisturePin = moistureSensorPin;
}

bool EnvSensor::begin() {
    bool success;
    tempSensor->begin();
    success = bme->begin(bmeAddr);

    return success;
}

float EnvSensor::getOWTemperature() {
    tempSensor->requestTemperatures();
    return tempSensor->getTempCByIndex(0);
}

int EnvSensor::getMoisture() {
    return analogRead(moisturePin);
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

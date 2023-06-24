#include <Arduino.h>
#include "EnvSensor.h"

EnvSensor sensor;

OneWire tempSensorOW(OW_TEMP_PIN);
DallasTemperature dTemp(&tempSensorOW);

void setup() {
    Serial.begin(115200);
    while(!Serial);


    dTemp.begin();
    sensor.begin();
}

void loop() {
    Serial.println("-------------------------------------------------");
    dTemp.requestTemperatures();
    Serial.println(dTemp.getTempCByIndex(0));

    Serial.println(sensor.getMoisture());
    Serial.println();
    Serial.println(sensor.getTemperature());
    Serial.println(sensor.getHumidity());
    Serial.println(sensor.getPressure());


    delay(1000);
}
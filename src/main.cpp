#include <Arduino.h>
#include "sleep.h"
#include "hardware/rtc.h"

#include "OneWireNg_CurrentPlatform.h"
#include "drivers/DSTherm.h"
#include "utils/Placeholder.h"

#include "EnvSensor.h"
#include "SPI.h"

#include <lmic.h>
#include <hal/hal.h>

#define Serial Serial1

MbedSPI spi(8,15,14);

EnvSensor sensor;

Placeholder<OneWireNg_CurrentPlatform> ow;




//LoRaWan config
static const u1_t PROGMEM APPEUI[8] = APP_EUI;

void os_getArtEui(u1_t *buf) { memcpy_P(buf, APPEUI, 8); }

static const u1_t PROGMEM DEVEUI[8] = DEV_EUI;

void os_getDevEui(u1_t *buf) { memcpy_P(buf, DEVEUI, 8); }

static const u1_t PROGMEM APPKEY[16] = APP_KEY;

void os_getDevKey(u1_t *buf) { memcpy_P(buf, APPKEY, 16); }



static uint8_t payload[11];
static osjob_t sendjob;
static osjob_t retryJoinCallback;

uint8_t LMIC_DataBits;

// send interval in seconds
#ifdef DEBUG_MODE
const unsigned TX_INTERVAL = 60;
#else
const unsigned TX_INTERVAL = SEND_OFFSET;
#endif

// Pin mapping
const lmic_pinmap lmic_pins = {
        .nss = 16,
        .rxtx = LMIC_UNUSED_PIN,
        .rst = 17,
        .dio = {21, 22, 23},
};

void printHex2(unsigned v) {
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

float owTemperature;
int moisture;
float bmeTemperature;
float bmePressure;
float bmeHumidity;

datetime_t t = {
        .year = 0,
        .month = 1,
        .day = 1,
        .dotw = 0,
        .hour = 1,
        .min = 1,
        .sec = 1
};


void updateSensorValues() {
    sensor.wakeUp();
    //dTemp.requestTemperatures();
    //owTemperature = dTemp.getTempCByIndex(0);

    DSTherm owTemp(ow);
    owTemp.convertTempAll(DSTherm::MAX_CONV_TIME, false);
    static Placeholder<DSTherm::Scratchpad> scrpd;
    OneWireNg::ErrorCode ec = owTemp.readScratchpadSingle(scrpd);
    if (ec == OneWireNg::EC_SUCCESS) {
        owTemperature =  (float)(scrpd->getTemp()) / 1000;
    } else if (ec == OneWireNg::EC_CRC_ERROR)
        owTemperature = -500;


    moisture = sensor.getMoisture();

    bmeTemperature = sensor.getTemperature();
    bmeHumidity = sensor.getHumidity();
    bmePressure = sensor.getPressure();
    sensor.sleep();
}


void do_send(osjob_t *j) {


    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {


        Serial.println("Preparing Send");
        // update values from sensors

        updateSensorValues();

        // load Data into array
        // adjust for the f2sflt16 range (-1 to 1)
        float fOwTemp = owTemperature / 200; // temp will not go under or above -200 / 200 °C (range of sensor -55 to +125 °C)
        float fMoisture = (float) moisture / 4096; // adc resolution of 12 bits
        float fBmeTemperature = bmeTemperature / 200; // see above (sensor range: -40 to +85 °C)
        float fBmePressure = bmePressure / 110000; // max pressure measured is 1100 hPa or 110000 Pa
        float fBmeHumidity = bmeHumidity / 100; // max humidity is 100%

        uint16_t payloadOwTemp = LMIC_f2sflt16(fOwTemp);
        uint16_t payloadMoisture = LMIC_f2sflt16(fMoisture);
        uint16_t payloadBmeTemperature = LMIC_f2sflt16(fBmeTemperature);
        uint16_t payloadBmePressure = LMIC_f2sflt16(fBmePressure);
        uint16_t payloadBmeHumidity = LMIC_f2sflt16(fBmeHumidity);

        byte owTempLow = lowByte(payloadOwTemp);
        byte owTempHigh = highByte(payloadOwTemp);
        payload[0] = owTempLow;
        payload[1] = owTempHigh;

        byte moistLow = lowByte(payloadMoisture);
        byte moistHigh = highByte(payloadMoisture);
        payload[2] = moistLow;
        payload[3] = moistHigh;

        byte bmeTempLow = lowByte(payloadBmeTemperature);
        byte bmeTempHigh = highByte(payloadBmeTemperature);
        payload[4] = bmeTempLow;
        payload[5] = bmeTempHigh;

        byte bmePressLow = lowByte(payloadBmePressure);
        byte bmePressHigh = highByte(payloadBmePressure);
        payload[6] = bmePressLow;
        payload[7] = bmePressHigh;

        byte bmeHumidLow = lowByte(payloadBmeHumidity);
        byte bmeHumidHigh = highByte(payloadBmeHumidity);
        payload[8] = bmeHumidLow;
        payload[9] = bmeHumidHigh;





        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, payload, sizeof(payload) - 1, 0);
        Serial.println(F("Packet queued"));

    }
    // Next TX is scheduled after TX_COMPLETE event.
}

static void sleepFinished() {


    //do_send(&sendjob);
    os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(1), do_send);
}

void retryJoin(osjob_t *j) {
    LMIC_startJoining();
}

void goSleep() {

    Serial.println("Going to sleep");
    Serial.flush();
    sleep_goto_sleep_for(TX_INTERVAL, &sleepFinished, true);

}


void onEvent(ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch (ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            {
                u4_t netid = 0;
                devaddr_t devaddr = 0;
                u1_t nwkKey[16];
                u1_t artKey[16];
                LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
                Serial.print("netid: ");
                Serial.println(netid, DEC);
                Serial.print("devaddr: ");
                Serial.println(devaddr, HEX);
                Serial.print("AppSKey: ");
                for (size_t i = 0; i < sizeof(artKey); ++i) {
                    if (i != 0)
                        Serial.print("-");
                    printHex2(artKey[i]);
                }
                Serial.println("");
                Serial.print("NwkSKey: ");
                for (size_t i = 0; i < sizeof(nwkKey); ++i) {
                    if (i != 0)
                        Serial.print("-");
                    printHex2(nwkKey[i]);
                }
                Serial.println();
            }
            // Disable link check validation (automatically enabled)
            // during join, but because slow data rates change max TX
            // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            Serial.println("Connected");
            do_send(&sendjob);
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));

        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
                Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
                Serial.print(F("Received "));
                Serial.print(LMIC.dataLen);
                Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            //os_setTimedCallback(&goSleep, os_getTime() + sec2osticks(5), goSleep2);
            goSleep();

            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;
        case EV_TXCANCELED:
            Serial.println(F("EV_TXCANCELED"));
            break;
        case EV_RXSTART:
            /* do not print anything -- it wrecks timing */
            break;
        case EV_JOIN_TXCOMPLETE:
            Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
            Serial.println("No Connection... trying to reconnect in 5 Seconds");
            os_setTimedCallback(&retryJoinCallback, os_getTime() + sec2osticks(5), retryJoin);
            break;

        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}




void setup() {

    Serial.begin(115200);
    Serial.println("Starting Up");


    //ow sensor
    new (&ow) OneWireNg_CurrentPlatform(OW_TEMP_PIN, false);
    DSTherm owTemp(ow);
    owTemp.filterSupportedSlaves();



    sensor.begin();

    pinMode(13, OUTPUT);
    digitalWrite(13, false);


    _rtc_init();
    rtc_set_datetime(&t);


    //LMIC init
    hal_set_spi(&spi);
    os_init();


    delay(10);

    LMIC_reset();
    delay(10);

    LMIC.datarate = SF9;
    LMIC.txpow = 14;

    LMIC_startJoining();

    Serial.println("Joining...");


    do_send(&sendjob);


}


void loop() {
    os_runloop_once();
}
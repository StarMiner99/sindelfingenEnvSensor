#include <Arduino.h>
#include "EnvSensor.h"

#define DISABLE_PING
#define DISABLE_BEACONS

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#define BLINKING 1
#define LED_ON 2
#define LED_OFF 0

int led_state = BLINKING;

EnvSensor sensor;

OneWire tempSensorOW(OW_TEMP_PIN);
DallasTemperature dTemp(&tempSensorOW);

//LoRaWan config
static const u1_t PROGMEM APPEUI[8] = APP_EUI;
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

static const u1_t PROGMEM DEVEUI[8] = DEV_EUI;
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

static const u1_t PROGMEM APPKEY[16] = APP_KEY;
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

static uint8_t payload[11];
//static uint8_t mydata[] = "Hello, world!";
static osjob_t sendjob;

// send interval in seconds
const unsigned TX_INTERVAL = 60;

// Pin mapping
const lmic_pinmap lmic_pins = {
        .nss = PIN_RFM_CS,
        .rxtx = LMIC_UNUSED_PIN,
        .rst = PIN_RFM_RST,
        .dio = {PIN_RFM_DIO0, PIN_RFM_DIO1, PIN_RFM_DIO2},
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

void updateSensorValues() {
    dTemp.requestTemperatures();
    owTemperature = dTemp.getTempCByIndex(0);

    moisture = sensor.getMoisture();

    bmeTemperature = sensor.getTemperature();
    bmeHumidity = sensor.getHumidity();
    bmePressure = sensor.getPressure();
}


void do_send(osjob_t* j){
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
        LMIC_setTxData2(1, payload, sizeof(payload)-1, 0);
        Serial.println(F("Packet queued"));


        //LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        //Serial.println(F("Packet queued"));

    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
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
                for (size_t i=0; i<sizeof(artKey); ++i) {
                    if (i != 0)
                        Serial.print("-");
                    printHex2(artKey[i]);
                }
                Serial.println("");
                Serial.print("NwkSKey: ");
                for (size_t i=0; i<sizeof(nwkKey); ++i) {
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
            led_state = LED_ON;
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
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
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
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
            led_state = BLINKING;
            break;

        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}


ulong lastBlink;
void setup() {
    Serial.begin(115200);
    //while(!Serial);
    delay(5000); // wait for serial monitors to register device

    Serial.println("Starting Up");

    dTemp.begin();
    sensor.begin();

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, false);

    //LMIC init
    os_init();

    LMIC_reset();
    LMIC_setDrTxpow(SF12, 14);

    do_send(&sendjob);

    lastBlink = millis();
}


bool led_current = false;
void loop() {
#ifdef DEBUG_MODE
    switch (led_state) {
        case BLINKING:
            if (millis() - lastBlink >= 200) {
                digitalWrite(LED_BUILTIN, !led_current);
                led_current = !led_current;
                lastBlink = millis();
            }
            break;
        case LED_OFF:
            digitalWrite(LED_BUILTIN, false);
            break;
        case LED_ON:
            digitalWrite(LED_BUILTIN, true);
            break;
        default:
            break;

    }
#endif
    os_runloop_once();
}
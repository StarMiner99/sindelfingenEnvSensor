//
// Created by thijs on 6/24/23.
//

#ifndef SINDELFINGENENVSENSOR_CONFIG_H
#define SINDELFINGENENVSENSOR_CONFIG_H


// define if debuging (uses more energy due to led)
#define DEBUG_MODE

#define SEND_OFFSET 3600 // time in seconds how often to send data (in debug mode this is 60)

#define I2C_SDA_PIN 2
#define I2C_SCL_PIN 3

#define BME_ADDR 0x76
#define MOISTURE_SENSOR_PIN A0
#define MOISTURE_SENSOR_POWER_PIN A1
#define OW_TEMP_PIN 5



// the following values should not be published
// and therefore placed in a seperate file that is not version Controlled
#include "ConfigPrivate.h"

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
//#define APP_EUI { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
// This should also be in little endian format, see above.
//#define DEV_EUI { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
//#define APP_KEY { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

#endif //SINDELFINGENENVSENSOR_CONFIG_H

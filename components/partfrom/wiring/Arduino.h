#ifndef _ARDUINO_H_
#define _ARDUINO_H_

#include "digital_io.h"
#include "wiring_time.h"
#include "HardwareSerial.h"

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitToggle(value, bit) ((value) ^= (1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

typedef uint8_t byte;

#define MOSI            PA7
#define MISO            PA6
#define SCK             PA5
#define SS              PA2
#define SDA             
#define SCL
#define PIN_SERIAL_RX   PA9
#define PIN_SERIAL_TX   PA10

#endif // _ARDUINO_H_
#ifndef _DIGITAL_IO_H_
#define _DIGITAL_IO_H_

#include "main.h"
#include "partfrom.h"
#include <stdint.h>

/*----------------------------------------------------------------------------
 *        STM32 pins number
 *----------------------------------------------------------------------------*/
#define PA0 0
#define PA1 1
#define PA2 2
#define PA3 3
#define PA4 4
#define PA5 5
#define PA6 6
#define PA7 7
#define PA8 8
#define PA9 9
#define PA10 10
#define PA11 11
#define PA12 12
#define PA13 13
#define PA14 14
#define PA15 15
#define PB0 16
#define PB1 17
#define PB2 18
#define PB3 19
#define PB4 20
#define PB5 21
#define PB6 22
#define PB7 23
#define PB8 24
#define PB9 25
#define PB10 26
#define PB11 27
#define PB12 28
#define PB13 29
#define PB14 30
#define PB15 31
#define PC13 32
#define PC14 33
#define PC15 34
#define PD0 35
#define PD1 36

#define LOW 0x0
#define HIGH 0x1

// GPIO FUNCTIONS
#define INPUT 0x01
// Changed OUTPUT from 0x02 to behave the same as Arduino pinMode(pin,OUTPUT)
// where you can read the state of pin even when it is set as OUTPUT
#define OUTPUT 0x03
#define PULLUP 0x04
#define INPUT_PULLUP 0x05
#define PULLDOWN 0x08
#define INPUT_PULLDOWN 0x09
#define OPEN_DRAIN 0x10
#define OUTPUT_OPEN_DRAIN 0x12
#define ANALOG 0xC0

#ifdef __cplusplus
extern "C" {
#endif

void digitalWrite(uint32_t ulPin, uint32_t ulVal);
int digitalRead(uint32_t ulPin);
void digitalToggle(uint32_t ulPin);
void pinMode(uint32_t ulPin, uint32_t mode);

#ifdef __cplusplus
}
#endif

#endif // _DIGITAL_IO_H_

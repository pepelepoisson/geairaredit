// 5x3 monochrome images representing numbers
// Image Size     : 5x3 pixels

#if defined(__AVR__)
#include <avr/pgmspace.h>
#elif defined(__PIC32MX__)
#define PROGMEM
#elif defined(__arm__)
#define PROGMEM
#endif

static const uint8_t PROGMEM
small_font[][5] =
{
  { // 0
    B11100000,
    B10100000,
    B10100000,
    B10100000,
    B11100000
  },
  { // 1
    B01000000,
    B11000000,
    B01000000,
    B01000000,
    B01000000
  },
  { // 2
    B11100000,
    B00100000,
    B11100000,
    B10000000,
    B11100000
  },
  { // 3
    B11100000,
    B00100000,
    B11100000,
    B00100000,
    B11100000
  },
  { // 4
    B10000000,
    B10000000,
    B10100000,
    B11100000,
    B00100000
  },
  { // 5
    B11100000,
    B10000000,
    B11100000,
    B00100000,
    B11100000
  },
  { // 6
    B11100000,
    B10000000,
    B11100000,
    B10100000,
    B11100000
  },
  { // 7
    B11100000,
    B00100000,
    B00100000,
    B00100000,
    B00100000
  },
  { // 8
    B11100000,
    B10100000,
    B11100000,
    B10100000,
    B11100000
  },
  { // 9
    B11100000,
    B10100000,
    B11100000,
    B00100000,
    B11100000
  },
  { // Minus
    B00000000,
    B00000000,
    B00000000,
    B10000000,
    B00000000
  },
};

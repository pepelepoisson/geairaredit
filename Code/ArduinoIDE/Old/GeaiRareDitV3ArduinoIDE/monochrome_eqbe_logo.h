// 32x8 monochrome images
// Image Size     : 8x8 pixels

#if defined(__AVR__)
    #include <avr/pgmspace.h>
#elif defined(__PIC32MX__)
    #define PROGMEM
#elif defined(__arm__)
    #define PROGMEM
#endif

static const uint8_t PROGMEM
    mono_bmp_32[][32] =
    {
	{   // 0: eqbe 64 logo
	    B00000000,B00010000,B00000000,B00000000,
      B00000000,B00010000,B00000000,B00000000,
      B01111011,B11010110,B11110011,B11010000,
      B00001010,B01010010,B10110010,B00010100,
      B01101010,B01010010,B10000010,B11011110,
      B01111011,B01011110,B11110011,B11000100,
      B00000000,B01000000,B00000000,B00000000,
      B00000000,B01000000,B00000000,B00000000
        },
    };

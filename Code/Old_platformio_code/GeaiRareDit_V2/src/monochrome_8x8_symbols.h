// 8x8 monochrome images
// Image Size     : 8x8 pixels

#if defined(__AVR__)
    #include <avr/pgmspace.h>
#elif defined(__PIC32MX__)
    #define PROGMEM
#elif defined(__arm__)
    #define PROGMEM
#endif

static const uint8_t PROGMEM
    mono_bmp[][8] =
    {
	{    // 0: Battery 0%
      B00000000,
	    B00011000,
	    B00100100,
	    B00100100,
	    B00100100,
	    B00100100,
	    B00111100,
	    B00000000
			},

	{   // 1: Battery 25%
      B00000000,
      B00011000,
      B00100100,
      B00100100,
      B00100100,
      B00111100,
      B00111100,
      B00000000
			},

	{   // 2: Battery 50%
      B00000000,
      B00011000,
      B00100100,
      B00100100,
      B00111100,
      B00111100,
      B00111100,
      B00000000
      },

	{   // 3: Battery 75%
      B00000000,
      B00011000,
      B00100100,
      B00111100,
      B00111100,
      B00111100,
      B00111100,
      B00000000
      },

	{   // 4; Battery 100%
      B00000000,
      B00011000,
      B00111100,
      B00111100,
      B00111100,
      B00111100,
      B00111100,
      B00000000
      },

	{   // 5; Wifi 1
	    B01111110,
	    B10000001,
	    B00111100,
	    B01000010,
	    B10011001,
	    B00100100,
	    B00000000,
	    B00011000
      },

	{   // 6; Wifi 2
      B00000000,
      B01111110,
      B10000001,
      B00111100,
      B01000010,
      B00000000,
      B00011000,
      B00000000
      },

  {   // 7; Symbol 1
      B10000000,
      B10000000,
      B10011000,
      B10111100,
      B10111100,
      B10011000,
      B10000000,
      B10000000
      },

	{   // 8; Symbol 2
      B00000000,
      B00000000,
      B00100100,
      B00011000,
      B00011000,
      B00100100,
      B00000000,
      B11111111
      },
    {   // 9; Symbol 3
      B00000001,
      B00000001,
      B00111101,
      B00100101,
      B00100101,
      B00111101,
      B00000001,
      B00000001
      },

	{   // 10; Symbol 4
      B11111111,
      B00000000,
      B00000100,
      B00000100,
      B00000100,
      B00111100,
      B00000000,
      B00000000
      },
  };

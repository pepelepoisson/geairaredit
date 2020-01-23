/*

Inspired from:
- Simon Says code in SparkFun Inventor's Kit Example sketch 16
- Vcc read from ???
- Sleep code from ???
  */

/*************************************************
* Libraries
*************************************************/
#include <Arduino.h>
#include <Adafruit_GFX.h>            // version=1.2.2 Author=Adafruit
#include <Adafruit_NeoMatrix.h>      // version=1.1.2 Author=Adafruit
#include <Adafruit_NeoPixel.h>       // version=1.1.2 Author=Adafruit
#include "monochrome_eqbe_logo.h"    // Monochrome eqbe 64 image used in splash screen
#include "monochrome_5x3_symbols.h"  // Monochrome bitmaps for 5x3 numbers and symbols
#include "monochrome_8x8_symbols.h"  // Monochrome bitmaps for 8x8 symbols
#include "small_font_5x3-4pts.h"     // Definition of alternate 5x3-4pts font
#include <avr/interrupt.h> // Librairies for sleep mode
#include <avr/power.h> // Librairies for sleep mode
#include <avr/sleep.h> // Librairies for sleep mode
#include <avr/io.h> // Librairies for sleep mode

/*************************************************
* Public Constants
*************************************************/
// USER-DEFINED CONFIG SETTINGS AND TUNABLES
#define BRIGHTNESS 10                 // 0-255 LEDs brightness - default setting
#define SHOW_SPLASH_SCREEN 1          // Set to 1 to show splash screen at startup, 0 otherwise
#define USE_5X3_FONT 1                // Set to 1 to use small 5pts font instead of standard 7pts
#define MAX_REST_TIME 600000           //max waiting time before going to sleep
#define MAX_IDLE_TIME 30000           //max waiting time before going to sleep

// Notes used for sounds and tones
#define NOTE_B0 31
#define NOTE_C1 33
#define NOTE_CS1 35
#define NOTE_D1 37
#define NOTE_DS1 39
#define NOTE_E1 41
#define NOTE_F1 44
#define NOTE_FS1 46
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_FS6 1480
#define NOTE_G6 1568
#define NOTE_GS6 1661
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093
#define NOTE_CS7 2217
#define NOTE_D7 2349
#define NOTE_DS7 2489
#define NOTE_E7 2637
#define NOTE_F7 2794
#define NOTE_FS7 2960
#define NOTE_G7 3136
#define NOTE_GS7 3322
#define NOTE_A7 3520
#define NOTE_AS7 3729
#define NOTE_B7 3951
#define NOTE_C8 4186
#define NOTE_CS8 4435
#define NOTE_D8 4699
#define NOTE_DS8 4978

#define CHOICE_OFF      0 //Used to control LEDs
#define CHOICE_NONE     0 //Used to check buttons
#define CHOICE_RED  (1 << 0)
#define CHOICE_GREEN    (1 << 1)
#define CHOICE_BLUE (1 << 2)
#define CHOICE_ORANGE   (1 << 3)

// Hardware pins
#define LEDS_PIN 7        // Pin connected to WS2812B data in
#define MOSFET_PIN 6      // Pin connected to MOSFET - Must be active to supply power to LEDs
#define BUTTON_RED    3
#define BUTTON_GREEN  4
#define BUTTON_BLUE   2  // Is also used to wake-up board from sleep mode
#define BUTTON_ORANGE 5
#define BUZZER1  9  // Buzzer pin definitions (the other pin goes to ground)

// Define game parameters
#define ROUNDS_TO_WIN      13 //Number of rounds to succesfully remember before you win. 13 is do-able.
#define ENTRY_TIME_LIMIT   3000 //Amount of time to press a button before game times out. 3000ms = 3 sec
#define MODE_MEMORY  0
#define MODE_BATTLE  1
#define MODE_BEEGEES 2

// Game state variables
byte gameMode = MODE_MEMORY; //By default, let's play the memory game
byte gameBoard[32]; //Contains the combination of buttons as we advance
byte gameRound = 0; //Counts the number of succesful rounds the player has made it through

// LED MATRIX VARIABLES DECLARATION OR DEFINITION
#define mw 8    // Define matrix width
#define mh 8    // Define matrix height

// Mapping of matrix rotation setting to be used depending on orientation. Index @ position = orientation. Value @ position = set rotation setting (0, 1,2 0r 3). Default = 2 for Lumivelo2019
int8_t matrix_set_rotation[7]={ 2, 2, 1, 0, 3, 2, 2 };

Adafruit_NeoMatrix *matrix = new Adafruit_NeoMatrix(mw, mh, LEDS_PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);  // MATRIX DECLARATION:

// Color definitions
// This could also be defined as matrix->color(255,0,0) but those defines
// are meant to work for adafruit_gfx backends that are lacking color()
#define LED_BLACK    0

#define LED_RED_VERYLOW   (3 <<  11)
#define LED_RED_LOW     (7 <<  11)
#define LED_RED_MEDIUM    (15 << 11)
#define LED_RED_HIGH    (31 << 11)

#define LED_GREEN_VERYLOW (1 <<  5)
#define LED_GREEN_LOW     (15 << 5)
#define LED_GREEN_MEDIUM  (31 << 5)
#define LED_GREEN_HIGH    (63 << 5)

#define LED_BLUE_VERYLOW  3
#define LED_BLUE_LOW    7
#define LED_BLUE_MEDIUM   15
#define LED_BLUE_HIGH     31

#define LED_ORANGE_VERYLOW  (LED_RED_VERYLOW + LED_GREEN_VERYLOW)
#define LED_ORANGE_LOW    (LED_RED_LOW     + LED_GREEN_LOW)
#define LED_ORANGE_MEDIUM (LED_RED_MEDIUM  + LED_GREEN_MEDIUM)
#define LED_ORANGE_HIGH   (LED_RED_HIGH    + LED_GREEN_HIGH)

#define LED_PURPLE_VERYLOW  (LED_RED_VERYLOW + LED_BLUE_VERYLOW)
#define LED_PURPLE_LOW    (LED_RED_LOW     + LED_BLUE_LOW)
#define LED_PURPLE_MEDIUM (LED_RED_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_PURPLE_HIGH   (LED_RED_HIGH    + LED_BLUE_HIGH)

#define LED_CYAN_VERYLOW  (LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_CYAN_LOW    (LED_GREEN_LOW     + LED_BLUE_LOW)
#define LED_CYAN_MEDIUM   (LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_CYAN_HIGH   (LED_GREEN_HIGH    + LED_BLUE_HIGH)

#define LED_WHITE_VERYLOW (LED_RED_VERYLOW + LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_WHITE_LOW   (LED_RED_LOW     + LED_GREEN_LOW     + LED_BLUE_LOW)
#define LED_WHITE_MEDIUM  (LED_RED_MEDIUM  + LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_WHITE_HIGH    (LED_RED_HIGH    + LED_GREEN_HIGH    + LED_BLUE_HIGH)

//Definition of various other variables
uint16_t bmpcolor[] = { LED_GREEN_HIGH, LED_BLUE_HIGH, LED_RED_HIGH };
uint16_t morebmpcolor[] = { LED_RED_VERYLOW,LED_RED_LOW,LED_RED_MEDIUM,LED_RED_HIGH,LED_GREEN_VERYLOW,LED_GREEN_LOW,LED_GREEN_MEDIUM,LED_GREEN_HIGH,
                            LED_BLUE_VERYLOW,LED_BLUE_LOW,LED_BLUE_MEDIUM,LED_BLUE_HIGH,LED_ORANGE_VERYLOW,LED_ORANGE_LOW,LED_ORANGE_MEDIUM,LED_ORANGE_HIGH,
                            LED_PURPLE_VERYLOW,LED_PURPLE_LOW,LED_PURPLE_MEDIUM,LED_PURPLE_HIGH,LED_CYAN_VERYLOW,LED_CYAN_LOW,LED_CYAN_MEDIUM,LED_CYAN_HIGH,
                            LED_WHITE_VERYLOW,LED_WHITE_LOW,LED_WHITE_MEDIUM,LED_WHITE_HIGH};
uint8_t y_text_ref= 0;  // Reference position used when displaying text on the matrix
int Vcc=0;  // Variable holding battery voltage in mV
unsigned long start_time=0, current_time=0, elapsed_time=0;  // Time references and counters
uint16_t menucolor[] = { LED_RED_MEDIUM, LED_BLUE_MEDIUM,  LED_ORANGE_MEDIUM, LED_GREEN_MEDIUM};  // Sequence of colors in menu cycles
uint16_t menusymbol[] = { 7,8,9,10 };  // Sequence of symbols from monochrome_8x8_symbols.h in menu cycles

/*************************************************
* Subroutines
*************************************************/

void display_bitmap(uint8_t bmp_num, uint16_t color) {   // Displays 8x8 monochrome images from mono_bmp table
    matrix->fillRect(0,0, 8,8, LED_BLACK);  // Clear the space under the bitmap that will be drawn
    matrix->drawBitmap(0, 0, mono_bmp[bmp_num], 8, 8, color);
    matrix->show();
}

void display_5x3_font(uint8_t bmp_num, uint16_t color,uint8_t xref, uint8_t yref) {   // Displays 5x3 monochrome images or symbols from small_font table
    matrix->fillRect(xref, yref, xref+3,yref+5, LED_BLACK);  // Clear the space under the bitmap that will be drawn
    matrix->drawBitmap(xref, yref, small_font[bmp_num], 3, 5, color);
    matrix->show();
}

void display_5x3_integer(int number){   // Handles display of numbers made of up to 2 5x3 symbols
  int tenth=0,unit=0;
  int color=LED_RED_MEDIUM;   // Default color which is used for positive numbers
  if (number<0){
    tenth=-number/10;
    unit=-number%10;
    color=LED_BLUE_MEDIUM;   // Switch to blue for negative numbers
    display_5x3_font(10, color,0,0);   // Display minus symbol
  }
  else {
    matrix->fillRect(0,0, 1,5, LED_BLACK);   // Display black pixels ahead of positive number
    tenth=number/10;
    unit=number%10;
  }
  display_5x3_font(tenth, color,1,0);
  display_5x3_font(unit, color,5,0);
}

void display_scrolling_text(String message, uint8_t rotation,uint8_t text_color, uint8_t y_pos) {
    //matrix->clear();
    matrix->setTextWrap(false);  // we don't wrap text so it scrolls nicely
    matrix->setTextSize(1);
    matrix->setRotation(rotation);
    uint16_t message_length=message.length();
    uint16_t font_width;
    #if USE_5X3_FONT
      font_width=4;
    #else
      font_width=5
    #endif

    for (int16_t x=7; x>=-font_width*message_length; x--) {
       matrix->clear();
       //matrix->fillRect(0,0, 8,y_pos+1, LED_BLACK);
       matrix->setCursor(x,y_pos);
       matrix->setTextColor(morebmpcolor[text_color]);
       matrix->print(message);
       matrix->show();
       delay(100);  // This delay sets the speed of scrolling text
    }
    matrix->fillRect(0,0, 8,8, LED_BLACK);   // Matrix is black after text has scrolled completely
}

void pinInterrupt(void) {  // Used to allow board wake up by button press
     detachInterrupt(0);
 }

void sleepNow(void){  // Routine to put board to sleep and control actions on waking-up
    // Set pin 2 as interrupt and attach handler:
    attachInterrupt(0, pinInterrupt, LOW);
    delay(100);
    //
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    //
    // Set sleep enable (SE) bit:
    sleep_enable();
    //
    // Put the device to sleep:
    matrix->fillRect(0,0, 8,8, LED_BLACK);  // Clear the matrix
    matrix->show();

    digitalWrite(MOSFET_PIN,LOW);   // turn LEDs off to indicate sleep
    Serial.println("Going to sleep... ");
    sleep_mode();
    //
    // Upon waking up, sketch continues from this point.
    sleep_disable();
    digitalWrite(MOSFET_PIN,HIGH);   // turn LED on to indicate awake
    delay(100);

    Serial.println("Restarting... ");
    start_time=millis();
}


long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

void setLEDs(byte leds){  // Lights a given LEDs - Pass in a byte that is made up from CHOICE_RED, CHOICE_ORANGE, etc
  if ((leds & CHOICE_RED) != 0){
    matrix->fillTriangle(0,1, 0,5, 2,3,LED_RED_HIGH);
    matrix->fillTriangle(0,2, 0,6, 2,4,LED_RED_HIGH);
  }
  else {
    matrix->fillTriangle(0,1, 0,5, 2,3,LED_BLACK);
    matrix->fillTriangle(0,2, 0,6, 2,4,LED_BLACK);
  }
  if ((leds & CHOICE_GREEN) != 0){
    matrix->fillTriangle(1,0, 5,0, 3,2,LED_GREEN_HIGH);
    matrix->fillTriangle(2,0, 6,0, 4,2,LED_GREEN_HIGH);
  }
  else {
    matrix->fillTriangle(1,0, 5,0, 3,2,LED_BLACK);
    matrix->fillTriangle(2,0, 6,0, 4,2,LED_BLACK);
  }
  if ((leds & CHOICE_BLUE) != 0){
    matrix->fillTriangle(1,7, 5,7, 3,5,LED_BLUE_HIGH);
    matrix->fillTriangle(2,7, 6,7, 4,5,LED_BLUE_HIGH);
  }
  else {
    matrix->fillTriangle(1,7, 5,7, 3,5,LED_BLACK);
    matrix->fillTriangle(2,7, 6,7, 4,5,LED_BLACK);
  }
  if ((leds & CHOICE_ORANGE) != 0){
    matrix->fillTriangle(7,1, 7,5, 5,3,LED_ORANGE_HIGH);
    matrix->fillTriangle(7,2, 7,6, 5,4,LED_ORANGE_HIGH);
  }
  else {
    matrix->fillTriangle(7,1, 7,5, 5,3,LED_BLACK);
    matrix->fillTriangle(7,2, 7,6, 5,4,LED_BLACK);
  }
  matrix->show();
}

byte checkButton(void) {  // Returns a '1' bit in the position corresponding to CHOICE_RED, CHOICE_GREEN, etc.
  if (digitalRead(BUTTON_RED) == 0) return(CHOICE_RED);
  else if (digitalRead(BUTTON_GREEN) == 0) return(CHOICE_GREEN);
  else if (digitalRead(BUTTON_BLUE) == 0) return(CHOICE_BLUE);
  else if (digitalRead(BUTTON_ORANGE) == 0) return(CHOICE_ORANGE);

  return(CHOICE_NONE); // If no button is pressed, return none
}

void buzz_sound(int buzz_length_ms, int buzz_delay_us){  // Toggle buzzer every buzz_delay_us, for a duration of buzz_length_ms.
  // Convert total play time from milliseconds to microseconds
  long buzz_length_us = buzz_length_ms * (long)1000;

  // Loop until the remaining play time is less than a single buzz_delay_us
  while (buzz_length_us > (buzz_delay_us * 2))
  {
    buzz_length_us -= buzz_delay_us * 2; //Decrease the remaining play time

    // Toggle the buzzer at various speeds
    digitalWrite(BUZZER1, LOW);
    delayMicroseconds(buzz_delay_us);


    digitalWrite(BUZZER1, HIGH);
    delayMicroseconds(buzz_delay_us);
  }
}


void toner(byte which, int buzz_length_ms)
{
  // Light an LED and play tone
  // Red, upper left:     440Hz - 2.272ms - 1.136ms pulse
  // Green, upper right:  880Hz - 1.136ms - 0.568ms pulse
  // Blue, lower left:    587.33Hz - 1.702ms - 0.851ms pulse
  // ORANGE, lower right: 784Hz - 1.276ms - 0.638ms pulse
  setLEDs(which); //Turn on a given LED

  //Play the sound associated with the given LED
  switch(which)
  {
  case CHOICE_RED:
    buzz_sound(buzz_length_ms, 1136);
    break;
  case CHOICE_GREEN:
    buzz_sound(buzz_length_ms, 568);
    break;
  case CHOICE_BLUE:
    buzz_sound(buzz_length_ms, 851);
    break;
  case CHOICE_ORANGE:
    buzz_sound(buzz_length_ms, 638);
    break;
  }

  setLEDs(CHOICE_OFF); // Turn off all LEDs
}

byte wait_for_button(void)
{
  // Wait for a button to be pressed.
  // Returns one of LED colors (LED_RED, etc.) if successful, 0 if timed out
  long startTime = millis(); // Remember the time we started the this loop

  while ( (millis() - startTime) < ENTRY_TIME_LIMIT) // Loop until too much time has passed
  {
    byte button = checkButton();

    if (button != CHOICE_NONE)
    {
      toner(button, 150); // Play the button the user just pressed

      while(checkButton() != CHOICE_NONE) ;  // Now let's wait for user to release button

      delay(10); // This helps with debouncing and accidental double taps

      return button;
    }

  }

  return CHOICE_NONE; // If we get here, we've timed out!
}

void winner_sound(void){  // Play the winner sound. Toggle the buzzer at various speeds.
  for (byte x = 250 ; x > 70 ; x--)
  {
    for (byte y = 0 ; y < 3 ; y++)
    {
      digitalWrite(BUZZER1, LOW);
      delayMicroseconds(x);

      digitalWrite(BUZZER1, HIGH);
      delayMicroseconds(x);
    }
  }
}

void play_winner(void){  // Play the winner sound and lights
  setLEDs(CHOICE_GREEN | CHOICE_BLUE);
  winner_sound();
  setLEDs(CHOICE_RED | CHOICE_ORANGE);
  winner_sound();
  setLEDs(CHOICE_GREEN | CHOICE_BLUE);
  winner_sound();
  setLEDs(CHOICE_RED | CHOICE_ORANGE);
  winner_sound();
}

void play_loser(void){  // Play the loser sound/lights
  setLEDs(CHOICE_RED | CHOICE_GREEN);
  buzz_sound(255, 1500);

  setLEDs(CHOICE_BLUE | CHOICE_ORANGE);
  buzz_sound(255, 1500);

  setLEDs(CHOICE_RED | CHOICE_GREEN);
  buzz_sound(255, 1500);

  setLEDs(CHOICE_BLUE | CHOICE_ORANGE);
  buzz_sound(255, 1500);

  start_time=millis();  // Reset counter before game goes to sleep mode
}

void RestMode(void){  // Turn off LEDs to save battery but dont cut LED power supply yet to avoid white flash if user returns before sleep. Send to sleep if wait time is too long.
  matrix->fillRect(0,0, 8,8, LED_BLACK);  // Clear the matrix
  matrix->show();
  while(1)
    {
    if (millis()-start_time>MAX_REST_TIME){sleepNow();}
    if (checkButton() != CHOICE_NONE){start_time=millis();return;}
  }
}

void attractMode(void){  // Show an "attract mode" display while waiting for user to press button. Send to sleep if wait time is too long.
  while(1)
  {

    if (millis()-start_time>MAX_IDLE_TIME){start_time=millis();RestMode();}

    while(checkButton() != CHOICE_NONE){delay(50);}  // Wait that button is released (used if returning from a button-induced wake-up)

    for (uint8_t i=0; i<4;i++){
      display_bitmap(menusymbol[i], menucolor[i]);
      delay(300);
      matrix->fillRect(0,0, 8,8, LED_BLACK);   // Matrix is black
      if (checkButton() != CHOICE_NONE) return;
    }

  }
}

// Plays the current contents of the game moves
void playMoves(void)
{
  for (byte currentMove = 0 ; currentMove < gameRound ; currentMove++)
  {
    toner(gameBoard[currentMove], 150);

    // Wait some amount of time between button playback
    // Shorten this to make game harder
    delay(150); // 150 works well. 75 gets fast.
  }
}

// Adds a new random button to the game sequence, by sampling the timer
void add_to_moves(void)
{
  byte newButton = random(0, 4); //min (included), max (exluded)

  // We have to convert this number, 0 to 3, to CHOICEs
  if(newButton == 0) newButton = CHOICE_RED;
  else if(newButton == 1) newButton = CHOICE_GREEN;
  else if(newButton == 2) newButton = CHOICE_BLUE;
  else if(newButton == 3) newButton = CHOICE_ORANGE;

  gameBoard[gameRound++] = newButton; // Add this new button to the game array
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//The following functions are related to game play only

// Play the regular memory game
// Returns 0 if player loses, or 1 if player wins
boolean play_memory(void)
{
  randomSeed(millis()); // Seed the random generator with random amount of millis()

  gameRound = 0; // Reset the game to the beginning

  while (gameRound < ROUNDS_TO_WIN)
  {
    add_to_moves(); // Add a button to the current moves, then play them back

    playMoves(); // Play back the current game board

    // Then require the player to repeat the sequence.
    for (byte currentMove = 0 ; currentMove < gameRound ; currentMove++)
    {
      byte choice = wait_for_button(); // See what button the user presses

      if (choice == 0) return false; // If wait timed out, player loses

      if (choice != gameBoard[currentMove]) return false; // If the choice is incorect, player loses
    }

    delay(1000); // Player was correct, delay before playing moves
  }

  return true; // Player made it through all the rounds to win!
}

// Play the special 2 player battle mode
// A player begins by pressing a button then handing it to the other player
// That player repeats the button and adds one, then passes back.
// This function returns when someone loses
boolean play_battle(void)
{
  gameRound = 0; // Reset the game frame back to one frame

  while (1) // Loop until someone fails
  {
    byte newButton = wait_for_button(); // Wait for user to input next move
    gameBoard[gameRound++] = newButton; // Add this new button to the game array

    // Then require the player to repeat the sequence.
    for (byte currentMove = 0 ; currentMove < gameRound ; currentMove++)
    {
      byte choice = wait_for_button();

      if (choice == 0) return false; // If wait timed out, player loses.

      if (choice != gameBoard[currentMove]) return false; // If the choice is incorect, player loses.
    }

    delay(100); // Give the user an extra 100ms to hand the game to the other player
  }

  return true; // We should never get here
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// The following functions are related to Beegees Easter Egg only

int melody[] = {  // Notes in the melody. Each note is about an 1/8th note, "0"s are rests.
  NOTE_G4, NOTE_A4, 0, NOTE_C5, 0, 0, NOTE_G4, 0, 0, 0,
  NOTE_E4, 0, NOTE_D4, NOTE_E4, NOTE_G4, 0,
  NOTE_D4, NOTE_E4, 0, NOTE_G4, 0, 0,
  NOTE_D4, 0, NOTE_E4, 0, NOTE_G4, 0, NOTE_A4, 0, NOTE_C5, 0};

int noteDuration = 115; // This essentially sets the tempo, 115 is just about right for a disco groove :)
int LEDnumber = 0; // Keeps track of which LED we are on during the beegees loop

void changeLED(void){  // Each time this function is called the board moves to the next LED
  setLEDs(1 << LEDnumber); // Change the LED

  LEDnumber++; // Goto the next LED
  if(LEDnumber > 3) LEDnumber = 0; // Wrap the counter if needed
}

void play_beegees(){  // Do nothing but play bad beegees music. This function is activated when user holds bottom right button during power up.
  //Turn on the bottom right (ORANGE) LED
  setLEDs(CHOICE_ORANGE);
  toner(CHOICE_ORANGE, 150);

  setLEDs(CHOICE_RED | CHOICE_GREEN | CHOICE_BLUE); // Turn on the other LEDs until you release button

  while(checkButton() != CHOICE_NONE) ; // Wait for user to stop pressing button

  setLEDs(CHOICE_NONE); // Turn off LEDs

  delay(1000); // Wait a second before playing song

  //digitalWrite(BUZZER1, LOW); // setup the "BUZZER1" side of the buzzer to stay low, while we play the tone on the other pin.

  while(checkButton() == CHOICE_NONE) //Play song until you press a button
  {
    // iterate over the notes of the melody:
    for (int thisNote = 0; thisNote < 32; thisNote++) {
      changeLED();
      tone(BUZZER1, melody[thisNote],noteDuration);
      // to distinguish the notes, set a minimum time between them.
      // the note's duration + 30% seems to work well:
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      // stop the tone playing:
      noTone(BUZZER1);
    }
  }
}


void setup()
{
  Serial.begin(9600);  // Establish serial connection for debugging

  //Setup hardware inputs/outputs. These pins are defined in the hardware_versions header file

  //Enable pull ups on inputs to save on resistors!
  pinMode(BUTTON_RED, INPUT_PULLUP);
  pinMode(BUTTON_GREEN, INPUT_PULLUP);
  pinMode(BUTTON_BLUE, INPUT_PULLUP);
  pinMode(BUTTON_ORANGE, INPUT_PULLUP);

  pinMode(MOSFET_PIN, OUTPUT);
  digitalWrite(MOSFET_PIN, HIGH);  // Turn on LED matrix 5V supply

  //Setup outputs for buzzer
  pinMode(BUZZER1, OUTPUT);

  // Check battery state and print to serial
  Vcc=int(readVcc());
  Serial.print("VCC=");Serial.print(Vcc);Serial.println("mV");

  // Initialize LED matrix
  matrix->setRotation(matrix_set_rotation[0]);   // Rotate display accordinly 0, 1, 2 or 3
  matrix->begin();
  matrix->setTextWrap(false);
  matrix->setBrightness(BRIGHTNESS);

  # if SHOW_SPLASH_SCREEN
    for (int8_t x=7; x>=-32; x--) {
       matrix->fillRect(0,0, 8,8, LED_BLACK);
       matrix->drawBitmap(x, 0, mono_bmp_32[0], 32, 8, LED_PURPLE_MEDIUM);
       matrix->show();
       delay(200);
    }
  //  display_scrolling_text("GeaiRareDit    ", 0,LED_GREEN_MEDIUM, 0);
  //  matrix->fillRect(0,0, 8,8, LED_BLACK);
  //  matrix->show();
  # endif

  matrix->clear();
  delay(500);

  #if USE_5X3_FONT
    matrix->setFont(&small_5x34pt7b);  // Change from default 7pts font to custom 5x3 font
    y_text_ref =6;
  # endif

  //Mode checking
  gameMode = MODE_MEMORY; // By default, we're going to play the memory game

  // Check to see if the lower right button is pressed
  if (checkButton() == CHOICE_ORANGE) play_beegees();

  // Check to see if upper right button is pressed
  if (checkButton() == CHOICE_GREEN)
  {
    gameMode = MODE_BATTLE; //Put game into battle mode

    //Turn on the upper right (green) LED
    setLEDs(CHOICE_GREEN);
    toner(CHOICE_GREEN, 150);

    setLEDs(CHOICE_RED | CHOICE_BLUE | CHOICE_ORANGE); // Turn on the other LEDs until you release button

    while(checkButton() != CHOICE_NONE) ; // Wait for user to stop pressing button

    //Now do nothing. Battle mode will be serviced in the main routine
  }

  play_winner(); // After setup is complete, say hello to the world
}

void loop()
{
  attractMode(); // Blink lights while waiting for user to press a button

  //display_bitmap(5, bmpcolor[2]);
  // Indicate the start of game play
  //setLEDs(CHOICE_RED | CHOICE_GREEN | CHOICE_BLUE | CHOICE_ORANGE); // Turn all LEDs on
  delay(1000);
  setLEDs(CHOICE_OFF); // Turn off LEDs
  delay(250);

  //if (checkButton() == CHOICE_ORANGE) play_beegees();

  if (checkButton() == CHOICE_GREEN)
  {
    gameMode = MODE_BATTLE; //Put game into battle mode

    //Turn on the upper right (green) LED

    matrix->fillRect(0,0, 8,8, LED_PURPLE_MEDIUM);
    matrix->show();

    toner(CHOICE_GREEN, 150);
    delay(1000);
    //matrix->fillRect(0,0, 8,8, LED_BLACK);

    setLEDs(CHOICE_RED | CHOICE_BLUE | CHOICE_ORANGE); // Turn on the other LEDs until you release button

    while(checkButton() != CHOICE_NONE) ; // Wait for user to stop pressing button

    //Now do nothing. Battle mode will be serviced in the main routine
  }
  else {gameMode = MODE_MEMORY;}

  if (gameMode == MODE_MEMORY)
  {
    matrix->fillRect(0,0, 8,8, LED_BLACK);
    // Play memory game and handle result
    if (play_memory() == true)
      play_winner(); // Player won, play winner tones
    else
      play_loser(); // Player lost, play loser tones
      matrix->fillRect(0,0, 8,8, LED_BLACK);  // Clear the matrix
      display_5x3_integer(gameRound-1); // Show number of rounds completed successfully
      delay(2000);
      matrix->fillRect(0,0, 8,8, LED_BLACK);  // Clear the matrix
      matrix->show();
  }

  if (gameMode == MODE_BATTLE)
  {
    play_battle(); // Play game until someone loses

    play_loser(); // Player lost, play loser tones
  }
}

// Libraries required
#include "I2Cdev.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal.h>
#include <Adafruit_INA219.h>

// A simple data logger for the INA219 current sensor

// how many milliseconds between grabbing data and logging it. 1000 ms is once a second
#define LOG_INTERVAL  5000 // mills between entries (reduce to take more/faster data)

// how many milliseconds before writing the logged data permanently to disk
// set it to the LOG_INTERVAL to write each time (safest)
// set it to 10*LOG_INTERVAL to write all data every 10 datareads, you could lose up to 
// the last 10 reads if power is lost but it uses less power and is much faster!
#define SYNC_INTERVAL 5000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()

#define ECHO_TO_SERIAL   1// echo data to serial port
#define ECHO_TO_LCD   1// echo data to lcd screen

// the digital pins that connect to the LEDs
#define redLEDpin 2
#define greenLEDpin 3

// macros from DateTime.h
/* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)  
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)  

#define VT_PIN A2 
#define AT_PIN A1

RTC_DS1307 RTC; // define the Real Time Clock object

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);  // initialize the LCD library with the numbers of the interface pins

float integrated_charge_mAh_ina219 = 0;
uint32_t previous_time=0;

int a0_level = 0;
int key_level = 1023;
String key_pressed = "";



// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// the logging file
File logfile;

Adafruit_INA219 ina219;

#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
   #define Serial SerialUSB
#endif

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  // red LED indicates error
  digitalWrite(redLEDpin, HIGH);

  while(1);
}

void setup(void)
{
  #ifndef ESP8266
    while (!Serial);     // will pause Zero, Leonardo, etc until serial console opens
  #endif
  uint32_t currentFrequency;
  
  Serial.begin(57600);
  Serial.println();

  // Initialize the INA219.
  // By default the initialization will use the largest range (32V, 2A).  However
  // you can call a setCalibration function to change this range (see comments).
  ina219.begin();
  // To use a slightly lower 32V, 1A range (higher precision on amps):
  //ina219.setCalibration_32V_1A();
  // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
  //ina219.setCalibration_16V_400mA();

  Serial.println("Measuring voltage and current with INA219 ...");  
  Serial.print("Millis (ms) ");
  Serial.print("Bus Voltage (V) ");
  Serial.print("Shunt Voltage (mV) ");
  Serial.print("Load Voltage (V) ");
  Serial.println("Current (mA)");
  
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.clear();    
  lcd.setCursor(0,0);
  lcd.print("*PowerLog 1.0*");
  lcd.setCursor(0,1);
  lcd.print("Laketanou");
  delay(1000);
  lcd.clear();
  
  #if ECHO_TO_LCD
  lcd.setCursor(0,0);  
  lcd.println("Write header...    ");
  delay(2000);
  lcd.clear();
  #endif //ECHO_TO_LCD
  //#if ECHO_TO_SERIAL
  //Serial.println("Calibration...    ");
  //#endif //ECHO_TO_SERIAL
  
  // use debugging LEDs
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);
  
  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    error("Card failed, or not present");
  }
  Serial.println("card initialized.");
  
  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
  if (! logfile) {
    error("couldnt create file");
  }

  #if ECHO_TO_LCD
  lcd.setCursor(0,0); 
  lcd.print("Logging to: ");
  lcd.setCursor(0,1);
  lcd.print(filename);
  #endif //ECHO_TO_LCD 
  #if ECHO_TO_SERIAL
  Serial.print("Logging to: ");
  Serial.println(filename);
  delay(1000);
  #endif //ECHO_TO_SERIAL

  // connect to RTC
  //Wire.begin();  
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_LCD
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("RTC failed");
#endif  //ECHO_TO_LCD    
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL
  }
  logfile.print("Logging to: ");
  logfile.println(filename);
}

void loop(void)
{
  a0_level = analogRead(A0); // the buttons are read from the analog0 pin
  
  // Check if a0_level has changed
  if ((a0_level != 1023) && (a0_level != key_level)){
    key_level = a0_level;
    if (key_level > 700 && key_level < 725){
       key_pressed="Select";
    } else if (key_level > 470 && key_level < 490){
      key_pressed="Left";
    } else if (key_level < 10){
      key_pressed="Right";
    } else if (key_level > 120 && key_level < 150){
      key_pressed="Up";
    } else if (key_level > 290 && key_level < 350){
      key_pressed="Down";
    }
      //update button pressed
      //lcd.setCursor(10,1);
      //lcd.print("        ");
      //lcd.setCursor(10,1);
      //lcd.print(btnStr);
  }
  else {key_pressed="";}

  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
  
  digitalWrite(greenLEDpin, HIGH);
  
  // log milliseconds since starting
  uint32_t m = millis();
  logfile.print(m);           // milliseconds since start
  logfile.print(", ");    

  DateTime now;
  
  // fetch the time
  now = RTC.now();
  // log time
  //logfile.print(now.unixtime()); // seconds since 1/1/1970
  //logfile.print(", ");
  logfile.print('"');
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
  logfile.print('"');  

  float shuntvoltage_ina219 = 0;
  float busvoltage_ina219 = 0;
  float current_mA_ina219 = 0;
  float loadvoltage_ina219 = 0;

  shuntvoltage_ina219 = ina219.getShuntVoltage_mV();
  busvoltage_ina219 = ina219.getBusVoltage_V();
  current_mA_ina219 = ina219.getCurrent_mA();
  loadvoltage_ina219 = busvoltage_ina219 + (shuntvoltage_ina219 / 1000);
  integrated_charge_mAh_ina219=integrated_charge_mAh_ina219+current_mA_ina219*((m-previous_time)/3600000);
  previous_time=m;

  // Read inputs from MAX471 sensor
  int vt_read = analogRead(VT_PIN);
  int at_read = analogRead(AT_PIN);

  float voltage_max471 = vt_read * (5.0 / 1024.0) * 5.0;
  float current_max471 = at_read * (5.0 / 1024.0)*1000;
  float watts_max471 = voltage_max471 * current_max471/1000.0;
  
  Serial.print("Volts: "); 
  Serial.print(voltage_max471, 3);
  Serial.print("\tmilliAmps: ");
  Serial.print(current_max471,3);
  Serial.print("\tWatts: ");
  Serial.println(watts_max471,3);
  Serial.println();
  
  // log data
  logfile.print(", ");    
  logfile.print(shuntvoltage_ina219);
  logfile.print(", ");    
  logfile.print(busvoltage_ina219);
  logfile.print(", ");
  logfile.print(current_mA_ina219);  
  logfile.print(", ");
  logfile.print(loadvoltage_ina219);
  logfile.print(", ");
  logfile.print(key_pressed);
  logfile.print(", ");
  logfile.print(voltage_max471);  
  logfile.print(", ");
  logfile.print(current_max471);
  logfile.print(", ");
  logfile.print(watts_max471);  
  logfile.println();
#if ECHO_TO_SERIAL
  //Serial.print(now.unixtime()); // seconds since 1/1/1970
  //Serial.print(", ");
  Serial.print('"');
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.print('"');
  Serial.print(", ");    
  Serial.print(shuntvoltage_ina219);
  Serial.print(", ");    
  Serial.print(busvoltage_ina219);
  Serial.print(", ");
  Serial.print(current_mA_ina219);
  Serial.print(", ");
  Serial.print(loadvoltage_ina219);
  Serial.print(", ");  
  Serial.print(key_pressed);
  Serial.println();
#endif //ECHO_TO_SERIAL

#if ECHO_TO_LCD    
    time(millis() / 1000);
    lcd.setCursor(0,1);
    lcd.print("U=");
    lcd.print(loadvoltage_ina219);
    lcd.print("V I=");
    lcd.print(current_mA_ina219);
    lcd.print("mA        ");
#endif  //ECHO_TO_LCD    


  digitalWrite(greenLEDpin, LOW);

  // Now we write data to disk! Don't sync too often - requires 2048 bytes of I/O to SD card
  // which uses a bunch of power and takes time
  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();
  
  // blink LED to show we are syncing data to the card & updating FAT!
  digitalWrite(redLEDpin, HIGH);
  logfile.flush();
  digitalWrite(redLEDpin, LOW);

}

void time(long val){  
int days = elapsedDays(val);
int hours = numberOfHours(val);
int minutes = numberOfMinutes(val);
int seconds = numberOfSeconds(val);

 // digital clock display of current time
 lcd.setCursor(0,0);
 lcd.print("Time ");
 lcd.print(days,DEC);  
 printDigits(hours);  
 printDigits(minutes);
 printDigits(seconds);
 Serial.println();
}

void printDigits(byte digits){
 // utility function for digital clock display: prints colon and leading 0
 lcd.print(":");
 if(digits < 10)
   lcd.print('0');
 lcd.print(digits,DEC);  
}


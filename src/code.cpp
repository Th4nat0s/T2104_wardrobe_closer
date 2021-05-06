/*
  __      __                  ._____________      ___.
 /  \    /  \_____ _______  __| _/\______   \ ____\_ |__   ____
 \   \/\/   /\__  \\_  __ \/ __ |  |       _//  _ \| __ \_/ __ \
  \        /  / __ \|  | \/ /_/ |  |    |   (  <_> ) \_\ \  ___/
   \__/\  /  (____  /__|  \____ |  |____|_  /\____/|___  /\___  >
        \/  ______\/.  __      \/         \/           \/     \/
            \_   ___ \|  |   ____  ______ ___________
            /    \  \/|  |  /  _ \/  ___// __ \_  __ \
            \     \___|  |_(  <_> )___ \\  ___/|  | \/
             \______  /____/\____/____  >\___  >__|
                    \/                \/     \/
                          Copyleft Thanat0s Since april 2021

  For all the kids who did not close their fuck**g WardRobes !!


  Code for Attiny25 microcontroller
  https://github.com/Th4nat0s/T2104_wardrobe_closer.

                               ATMEL Attiny25 / (ARDUINO)

                                         +-\/-+
        PCINT5/RESET/ADC0/dW        PB5 1|*   |8 VCC
 PCINT3/XTAL1/CLKI/OC1B/ADC3 (A3/3) PB3 2|    |7 PB2 (A1/2) SCK/SCL/USKC/TO/PCINT2
 PCINT4/XTAL2/CLKO/OC1B/ADC2 (A2/4) PB4 3|    |6 PB1 (1)    MISO/DO/AIN1/OC0B/OC1A/PCINT1
                                    GND 4|    |5 PB0 (0)    MOSI/DI/SDA/AIN0/OC0A/OC1A/AREF/PCINT0
                                         +----+
What does it do...
 Read door status of Wardrobe every minutes
 If open more than 5min
 Beep Until a kid close the Wardrobe !

https://enavarro.eu/exploration-des-sleep-mode-du-attiny85.html

*/

#include <Arduino.h>
#include <avr/sleep.h>

// Connectique
// Connect to input of TRCS5000
#define DET A3

// Connect to LED of TRCS5000
// In Attiny25 #define EMT 1
#define EMT 2

// To Buzzer
#define BEEP 4

// To Battery Led
#define LED 1

// Variables
int view;  // DAC read for sensor
int warning;
bool door = false;  // Door status, True is open
bool alert = false;
#define maxcount 5 // number of try before buzzing
int f_wdt = 0;  // out of sleep

// Define
#define TH_OPEN 210


// time base is 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
// time_base x time_mult = work loop delay
#define time_mult 8
#define time_base 9


// Routines to set and clear bits (used in the sleep code)
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// WDTCR in atmega is named WDTCSR
#ifndef WDTCR
#define WDTCR WDTCSR
#endif

// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(int ii) {
  byte bb;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);

  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCR = bb;
  WDTCR |= _BV(WDIE);
}

// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  f_wdt++;  // Increment timer
}

void setup()
{
  // initialize LED digital pin as an output.
  #ifdef __PLATFORMIO_BUILD_DEBUG__
  Serial.begin(9600);
  Serial.println("System Start");
  Serial.flush();
  #endif
  pinMode(LED,OUTPUT);
  setup_watchdog(time_base); // how many ~ Sec for 1 sleep loop.
  for (int i = 1; i <= 15; i++) {
    digitalWrite(LED,HIGH);
    delay(500);
    digitalWrite(LED,LOW);
    delay(500);
    }
}

void read_sensor()
// Read the sensor twice with or 
// Without led on to remove noise
{
  digitalWrite(EMT,HIGH);
  delayMicroseconds(500);
  view=analogRead(DET);  // 1ere lecture avec Led ir on
  digitalWrite(EMT,LOW);
  delayMicroseconds(500);
  view=analogRead(DET) - view;  // 2eme lecture avec Led ir off
  #ifdef __PLATFORMIO_BUILD_DEBUG__
  Serial.print("Score Read:");
  Serial.println(view);
  Serial.flush();
  #endif
  if (view > TH_OPEN){
    door = false;
    alert = false;
    warning = 0;
  } else {
    door = true;
  }
}

void main_loop()
{
  pinMode(EMT,OUTPUT);  // Reactivate out port
  pinMode(LED,OUTPUT);  // Reactivate out port
  delayMicroseconds(500);
  read_sensor();
  #ifdef __PLATFORMIO_BUILD_DEBUG__
  Serial.print("Main Loop, Door Status:");
  Serial.println(door);
  Serial.print("Main Loop, Warning Status:");
  Serial.println(warning);
  Serial.flush();
  #endif
  if (door == true){
    warning++;
    if (warning >= maxcount) {
      alert = true;
    }
  }
  while (alert == true) {
     // If more than N minutes
     #ifdef __PLATFORMIO_BUILD_DEBUG__
     Serial.println("Alert Door Open");
     Serial.flush();
     #endif
     tone(BEEP, 2600, 500);  // Tone 1, 0,5 Sec.
     delay(500);
     tone(BEEP, 1500, 500); // Tone 2, 0,5 Sec
     delay(1500);          // Silence 1 Sec.
     read_sensor();
    }
  pinMode(EMT,INPUT);  // Activate port In to lower consumpion
  pinMode(LED,INPUT);  // Activate port In to lower consumpion
}

// set system into the sleep state
// system wakes up when wtchdog is timed out
void system_sleep() {
  cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();                      // activate sleep
  sleep_mode();                        // System actually sleeps here
  sleep_disable();                     // System continues execution here when watchdog timed out
  sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON
}

void fake_sleep()
{
  delay(1000);
  f_wdt++;
  #ifdef __PLATFORMIO_BUILD_DEBUG__
  Serial.print("Wdt timer");
  Serial.println(f_wdt);
  Serial.flush();
  #endif
}


void loop()
{
  // 8 Sec x 8 = around 64 seconds
  if (f_wdt>=time_mult) {  // wait for timed out watchdog / flag is set when a watchdog timeout occurs
    #ifdef __PLATFORMIO_BUILD_DEBUG__
    Serial.println("Wdt timer Match count");
    Serial.flush();
    #endif
    main_loop();  // call real loop
    f_wdt=0;      // reset timing loop flag
  }
  //fake_sleep();
  #ifdef __PLATFORMIO_BUILD_DEBUG__
  Serial.println("Sleeping Loop");
  Serial.flush();
  #endif
  system_sleep();  // Send the unit to sleep
}

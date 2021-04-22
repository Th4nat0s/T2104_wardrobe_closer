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

  For all the kids who did not close they're WardRobe !!



  Code for Attiny25 microcontroller
  https://github.com/Th4nat0s/T2104_wardrobe_closer.

                               ATMEL Attiny25 / (ARDUINO)

                                         +-\/-+
        PCINT5/RESET/ADC0/dW        PB5 1|    |8 VCC
 PCINT3/XTAL1/CLKI/OC1B/ADC3 (A3/3) PB3 2|    |7 PB2 (A1/2) MISO/DO/AIN1/OC0B/OC1A/PCINT1
 PCINT4/XTAL2/CLKO/OC1B/ADC2 (A2/4) PB4 3|    |6 PB1 (1)    MISO/DO/AIN1/OC0B/OC1A/PCINT1
                                    GND 4|    |5 PB0 (0)    MOSI/DI/SDA/AIN0/OC0A/OC1A/AREF/PCINT0
                                         +----+
What does it do...
 Read door status of Wardrobe every minutes
 If open more than 5min
 Beep Until a kid close the Wardrobe !

*/

#include <Arduino.h>

// Connectique
// Connect to input of TRCS5000
#define DET A3

// Connect to LED of TRCS5000
// In Attiny25 #define EMT 1
#define EMT 2

// To Buzzer
#define BEEP 4


// Variables
int view;  // DAC read for sensor
int warning;
bool door = false;  // Door status, True is open
bool alert = false;
#define maxcount 5 // number of try before buzzing
#define loop_delay 2000 // Seconds between tests

// Define
#define TH_OPEN 350


void setup()
{
  // initialize LED digital pin as an output.
  #ifdef __PLATFORMIO_BUILD_DEBUG__
  Serial.begin(9600);
  #endif
  pinMode(EMT,OUTPUT); 
}

void read_sensor()
// Read the sensor twice with or 
// Without led on to remove noise
{
  digitalWrite(EMT,HIGH);
  delayMicroseconds(500);
  view=analogRead(DET);
  digitalWrite(EMT,LOW);
  delayMicroseconds(500);
  view=analogRead(DET) - view;
  if (view > TH_OPEN){
    door = false;
    alert = false;
    warning = 0;
  } else {
    door = true;
  }
}

void loop()
{
  read_sensor();
  #ifdef __PLATFORMIO_BUILD_DEBUG__
  Serial.println(door);
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
     #endif
     tone(BEEP, 600, 500);
     delay(500);
     tone(BEEP, 400, 500);
     delay(1000);
     read_sensor();
    }
  delay(loop_delay);
}

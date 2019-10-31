#include "MultiFuncShield.h"
#include <SoftwareSerial.h>

#define TRIG_PIN 5
#define ECHO_PIN 6

#define INC_LED LED_1
#define DEC_LED LED_4
#define BEEP_LED LED_2

#define MAX_BEEP_COUNT 10

//Application modes
enum AppModeValues{
  TEMP,
  BEEP,
  COUNT
};

//Modes in countdown application
enum CountDownModeValues{
  COUNTING_STOPPED,
  COUNTING,
  UP_S,
  DOWN_S,
  UP_M,
  DOWN_M,
  UP_10S,
  DOWN_10S,
  UP_10M,
  DOWN_10M,
  COUNT_RESET,
  COUNT_BEEP
};

//Modes in beeping application
enum BeepModeValues{
  INCREASING,
  DECREASING,
  BEEPING
};

byte appMode = TEMP;
byte countDownMode = COUNTING_STOPPED;
byte beepMode = BEEPING;
byte tenths = 0;
byte seconds = 0;
byte minutes = 0;

byte beepCount = 1;

char txtBuffer[5]; //Buffer of 4 characters (plus 'null character' or '\0' at the end, for the display) 

TimerOne t1;

void setup(){
  Serial.begin(9600);
  t1.initialize(); //Initialize TimerOne with default period (1s)
  
  //MFS is a declared instance of MultiFuncShield in MultiFuncShield.h
  MFS.initialize(&t1); //Enable timer interrupt with TimerOne's configuration

  MFS.initLM35(SMOOTHING_MODERATE);
}

void loop(){
  byte btn = MFS.getButton();

  //--------------------Temperature application-------------------------------
  if(appMode == TEMP){
    //Do something for temperature application
  }

  //---------------------Beeping application------------------------------
  else if(appMode == BEEP){
    //Do something for beeping application
  }

  //-------------------Countdown application--------------------------------
  else if(appMode == COUNT){ 
    //Do something for countdown application
  }
}

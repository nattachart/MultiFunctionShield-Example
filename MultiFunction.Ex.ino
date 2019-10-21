#include "MultiFuncShield.h"

#define TRIG_PIN 5
#define ECHO_PIN 6

#define BEEP_INC_LED LED_1
#define BEEP_DEC_LED LED_4
#define BEEP_LED LED_2

#define MAX_BEEP_COUNT 10

//Application modes
enum AppModeValues{
  COUNT,
  TEMP,
  BEEP
};

//Modes in countdown application
enum CountDownModeValues{
  COUNTING_STOPPED,
  COUNTING 
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
char seconds = 0;
char minutes = 0;

byte beepCount = 1;

TimerOne t1;

void setup(){
  Serial.begin(9600);
  t1.initialize(); //Initialize TimerOne with default period (1s)
  
  //MFS is a declared instance of MultiFuncShield in MultiFuncShield.h
  MFS.initialize(&t1); //Enable timer interrupt with TimerOne's configuration

  MFS.initLM35(SMOOTHING_NONE);
}

void loop(){
  byte btn = MFS.getButton();
  
  switch(appMode){
    //////////////////////////////////////////////////////
    case TEMP: //Temperature application
      if(btn == BUTTON_1_SHORT_RELEASE){
        appMode = BEEP;
        break;
      }
      else{
        int tempCentigrade = MFS.getLM35Data(); // get centigrade in 1/10 of degree.
        MFS.write((float)tempCentigrade / 10, 1);  // display temp to 1 decimal place.
      }
      break;
    //////////////////////////////////////////////////////
    case BEEP: //Beeping application
      MFS.write(beepCount); //Display the beep count
      switch(beepMode){
        case INCREASING:
          MFS.writeLeds(LED_ALL, OFF);
          MFS.writeLeds(BEEP_INC_LED, ON);
          MFS.blinkLeds(BEEP_INC_LED, ON);
          break;
        case DECREASING:
          MFS.writeLeds(LED_ALL, OFF);
          MFS.writeLeds(BEEP_DEC_LED, ON);
          MFS.blinkLeds(BEEP_DEC_LED, ON);
          break;
        case BEEPING:
          MFS.writeLeds(LED_ALL, OFF);
          MFS.writeLeds(BEEP_LED, ON);
          break;
      }
      
      if(btn == BUTTON_1_SHORT_RELEASE){
        appMode = COUNT;
        MFS.writeLeds(LED_ALL, OFF); //Turn off all LEDs before leaving the beep mode
        break;
      }
      else if(btn == BUTTON_2_SHORT_RELEASE){
        switch(beepMode){
          case INCREASING:
            beepMode = DECREASING;
            break;
          case DECREASING:
            beepMode = BEEPING;
            break;
          case BEEPING:
            beepMode = INCREASING;
            break;
        }
      }
      else if(btn == BUTTON_3_SHORT_RELEASE){
        switch(beepMode){
          case INCREASING:
            if(beepCount < MAX_BEEP_COUNT) 
              beepCount++;
            break;
          case DECREASING:
            if(beepCount > 1) //One as the minimum beep count
              beepCount--;
            break;
          case BEEPING:
            MFS.beep(5,    // beep for 50 milliseconds
                5,    // silent for 50 milliseconds
                beepCount,    // repeat above cycle beepCount times
                1,    // loop 1 time
                50    // wait 500 milliseconds between loop
                );
            break;
        }
      }
      break;
    //////////////////////////////////////////////////////
    case COUNT: //Countdown application
      if(btn == BUTTON_1_SHORT_RELEASE){
        appMode = TEMP;
        break;
      }
      else{
        switch(countDownMode){
          case COUNTING_STOPPED:
            if (btn == BUTTON_2_SHORT_RELEASE && (minutes + seconds) > 0){
              // start the timer
              countDownMode = COUNTING;
            }
            else if(btn == BUTTON_1_LONG_PRESSED){
              // reset the timer
              tenths = 0;
              seconds = 0;
              minutes = 0;
              MFS.write(minutes*100 + seconds);
            }
            else if(btn == BUTTON_2_PRESSED || btn == BUTTON_2_LONG_PRESSED){
              minutes++;
              if (minutes > 60)
                minutes = 0;
              MFS.write(minutes*100 + seconds);
            }
            else if(btn == BUTTON_3_PRESSED || btn == BUTTON_3_LONG_PRESSED){
              seconds += 10;
              if (seconds >= 60)
                seconds = 0;
              MFS.write(minutes*100 + seconds);
            }
            break;
          case COUNTING:
            if (btn == BUTTON_2_SHORT_RELEASE || btn == BUTTON_2_LONG_RELEASE){
              // stop the timer
              countDownMode = COUNTING_STOPPED;
            }
            else{
              // continue counting down
              tenths++;
              if (tenths == 10){
                 tenths = 0;
                 seconds--;
                 if (seconds < 0 && minutes > 0){
                  seconds = 59;
                  minutes--; 
                 }
                 if (minutes == 0 && seconds == 0){
                   // timer has reached 0, so sound the alarm
                   MFS.beep(50, 50, 3);  // beep 3 times, 500 milliseconds on / 500 off
                   countDownMode = COUNTING_STOPPED;
                 }
                 MFS.write(minutes*100 + seconds);
               }
               delay(100);
            }
            break;
        }
      }
      break;
  }
}

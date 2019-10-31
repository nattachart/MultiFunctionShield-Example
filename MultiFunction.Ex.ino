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

char txtBuffer[5];

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
      if(btn == BUTTON_1_SHORT_RELEASE){
        appMode = BEEP;
        beepMode = INCREASING; //Initial state of the beep application
      }
      else{
        int tempCentigrade = MFS.getLM35Data(); // get centigrade in 1/10 of degree.
        MFS.write((float)tempCentigrade / 10, 1);  // display temp to 1 decimal place.
      }
  }

  //---------------------Beeping application------------------------------
  else if(appMode == BEEP){
      MFS.write(beepCount); //Display the beep count
      switch(beepMode){
        case INCREASING:
          MFS.writeLeds(LED_ALL, OFF);
          MFS.writeLeds(INC_LED, ON);
          MFS.blinkLeds(INC_LED, ON);
          break;
        case DECREASING:
          MFS.writeLeds(LED_ALL, OFF);
          MFS.writeLeds(DEC_LED, ON);
          MFS.blinkLeds(DEC_LED, ON);
          break;
        case BEEPING:
          MFS.writeLeds(LED_ALL, OFF);
          MFS.writeLeds(BEEP_LED, ON);
          break;
      }
      
      if(btn == BUTTON_1_SHORT_RELEASE){
        appMode = COUNT;
        countDownMode = COUNT_RESET; //Initial state of the counting application
        MFS.writeLeds(LED_ALL, OFF); //Turn off all LEDs before leaving the beep mode
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
            MFS.beep(50,    // beep for 500 milliseconds
                10,    // silent for 100 milliseconds
                beepCount,    // repeat above cycle beepCount times
                1,    // loop 1 time
                30    // wait 300 milliseconds between loop (if loop 1 time, this does not mean anything)
                );
            break;
        }
      }
  }

  //-------------------Countdown application--------------------------------
  else if(appMode == COUNT){ 
      if(btn == BUTTON_1_SHORT_RELEASE){
        appMode = TEMP;
      }
      else{
        switch(countDownMode){
          case COUNT_RESET: //count reset state
            MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
            MFS.writeLeds(LED_1 | LED_2 | LED_3 | LED_4, OFF);
            // reset the timer
            tenths = 0;
            seconds = 0;
            minutes = 0;
            sprintf(txtBuffer, "%04d", minutes*100 + seconds);
            MFS.write(txtBuffer);
            countDownMode = COUNTING_STOPPED; //After finishing resetting all values, transition to the stop state
            break;
          //--------------------------------------------------- 
          case COUNT_BEEP: //count beep state
            MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
            MFS.writeLeds(LED_1 | LED_2 | LED_3 | LED_4, OFF);
            //sound the alarm
            MFS.beep(50, 50, 3);  // beep 3 times, 500 milliseconds on / 500 off
            countDownMode = COUNTING_STOPPED; //After time passed, finishing the beep, transition to the stop state
            break;
          //--------------------------------------------------- 
          case COUNTING: //counting state
            MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
            MFS.writeLeds(LED_1 | LED_2 | LED_3 | LED_4, OFF);
            if(btn == BUTTON_2_SHORT_RELEASE){
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
                   // timer has reached 0, transition to the count beep state
                   countDownMode = COUNT_BEEP;
                 }
                 sprintf(txtBuffer, "%04d", minutes*100 + seconds);
                 MFS.write(txtBuffer);
               }
               delay(100);
            }
            break;
          //---------------------------------------------------
          case COUNTING_STOPPED: //counting stop state
            MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
            MFS.writeLeds(LED_1 | LED_2 | LED_3 | LED_4, OFF);
            MFS.writeLeds(LED_2 | LED_3, ON);
            if (btn == BUTTON_2_LONG_PRESSED && seconds + minutes > 0){
              // start the timer
              countDownMode = COUNTING;
            }
            else if(btn == BUTTON_2_SHORT_RELEASE){
              countDownMode = UP_S;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_4, ON);
            }
            else if(btn == BUTTON_1_LONG_PRESSED){
              countDownMode = COUNT_RESET;
            }
            break;
          //---------------------------------------------------
          case UP_S: //1 second increment state
            MFS.writeLeds(LED_1 | LED_2 | LED_3 | LED_4, OFF);
            MFS.writeLeds(INC_LED, ON);
            MFS.blinkLeds(INC_LED, ON);
            if(btn == BUTTON_3_SHORT_RELEASE){
              //countDownMode = UP_S; //this can be omitted because it stays at the same state
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_4, ON);
              seconds = seconds + 1;
              if(seconds >= 60){
                seconds = 0;
              }
              sprintf(txtBuffer, "%04d", minutes*100 + seconds);
              MFS.write(txtBuffer);
            }
            else if(btn == BUTTON_3_LONG_RELEASE){
              countDownMode = UP_10S;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_3, ON);
            }
            else if(btn == BUTTON_2_SHORT_RELEASE){
              countDownMode = DOWN_S;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_4, ON);
            }
            else if (btn == BUTTON_2_LONG_PRESSED && seconds + minutes > 0){
              // start the timer
              countDownMode = COUNTING;
            }
            else if(btn == BUTTON_1_LONG_PRESSED){
              countDownMode = COUNT_RESET;
            }
            break;
          //---------------------------------------------------
          case UP_10S: //10 seconds increment state
            MFS.writeLeds(LED_1 | LED_2 | LED_3 | LED_4, OFF);
            MFS.writeLeds(INC_LED, ON);
            MFS.blinkLeds(INC_LED, ON);
            if(btn == BUTTON_3_SHORT_RELEASE){
              countDownMode = UP_S;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_4, ON);
            }
            else if(btn == BUTTON_3_LONG_RELEASE){
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_3, ON);
              seconds = seconds + 10;
              if(seconds >= 60){
                seconds = 0;
              }
              sprintf(txtBuffer, "%04d", minutes*100 + seconds);
              MFS.write(txtBuffer);
            }
            else if(btn == BUTTON_2_SHORT_RELEASE){
              countDownMode = DOWN_S;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_4, ON);
            }
            else if (btn == BUTTON_2_LONG_PRESSED && seconds + minutes > 0){
              // start the timer
              countDownMode = COUNTING;
            }
            else if(btn == BUTTON_1_LONG_PRESSED){
              countDownMode = COUNT_RESET;
            }
            break;
          //---------------------------------------------------
          case DOWN_S: //1 second decrement state
            MFS.writeLeds(LED_1 | LED_2 | LED_3 | LED_4, OFF);
            MFS.writeLeds(DEC_LED, ON);
            MFS.blinkLeds(DEC_LED, ON);
            if(btn == BUTTON_3_SHORT_RELEASE){
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_4, ON);
              //countDownMode = DOWN_S;
              if(seconds > 0)
                seconds = seconds - 1;
              sprintf(txtBuffer, "%04d", minutes*100 + seconds);
              MFS.write(txtBuffer);
            }
            else if(btn == BUTTON_3_LONG_RELEASE){
              countDownMode = DOWN_10S;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_3, ON);
            }
            else if(btn == BUTTON_2_SHORT_RELEASE){
              countDownMode = UP_M;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_2, ON);
            }
            else if (btn == BUTTON_2_LONG_PRESSED && seconds + minutes > 0){
              // start the timer
              countDownMode = COUNTING;
            }
            else if(btn == BUTTON_1_LONG_PRESSED){
              countDownMode = COUNT_RESET;
            }
            break;
          //---------------------------------------------------
          case DOWN_10S: //10 seconds decrement state
            MFS.writeLeds(LED_1 | LED_2 | LED_3 | LED_4, OFF);
            MFS.writeLeds(DEC_LED, ON);
            MFS.blinkLeds(DEC_LED, ON);
            if(btn == BUTTON_3_SHORT_RELEASE){
              countDownMode = DOWN_S;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_4, ON);
            }
            else if(btn == BUTTON_3_LONG_RELEASE){
              //countDownMode = DOWN_10S;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_3, ON);
              if(seconds >= 10)
                seconds = seconds - 10;
              sprintf(txtBuffer, "%04d", minutes*100 + seconds);
              MFS.write(txtBuffer);
            }
            else if(btn == BUTTON_2_SHORT_RELEASE){
              countDownMode = UP_M;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_2, ON);
            }
            else if (btn == BUTTON_2_LONG_PRESSED && seconds + minutes > 0){
              // start the timer
              countDownMode = COUNTING;
            }
            else if(btn == BUTTON_1_LONG_PRESSED){
              countDownMode = COUNT_RESET;
            }
            break;
          //---------------------------------------------------
          case UP_M: //1 minute increment state
            MFS.writeLeds(LED_1 | LED_2 | LED_3 | LED_4, OFF);
            MFS.writeLeds(INC_LED, ON);
            MFS.blinkLeds(INC_LED, ON);
            if(btn == BUTTON_3_SHORT_RELEASE){
              //countDownMode = UP_M;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_2, ON);
              minutes = minutes + 1;
              if(minutes >= 60){
                minutes = 0;
              }
              sprintf(txtBuffer, "%04d", minutes*100 + seconds);
              MFS.write(txtBuffer);
            }
            else if(btn == BUTTON_3_LONG_RELEASE){
              countDownMode = UP_10M;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_1, ON);
            }
            else if(btn == BUTTON_2_SHORT_RELEASE){
              countDownMode = DOWN_M;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_2, ON);
            }
            else if (btn == BUTTON_2_LONG_PRESSED && seconds + minutes > 0){
              // start the timer
              countDownMode = COUNTING;
            }
            else if(btn == BUTTON_1_LONG_PRESSED){
              countDownMode = COUNT_RESET;
            }
            break;
          //---------------------------------------------------
          case UP_10M: //10 minutes increment state
            MFS.writeLeds(LED_1 | LED_2 | LED_3 | LED_4, OFF);
            MFS.writeLeds(INC_LED, ON);
            MFS.blinkLeds(INC_LED, ON);
            if(btn == BUTTON_3_SHORT_RELEASE){
              countDownMode = UP_M;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_2, ON);
            }
            else if(btn == BUTTON_3_LONG_RELEASE){
              //countDownMode = UP_10M;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_1, ON);
              minutes = minutes + 10;
              if(minutes >= 60){
                minutes = 0;
              }
              sprintf(txtBuffer, "%04d", minutes*100 + seconds);
              MFS.write(txtBuffer);
            }
            else if(btn == BUTTON_2_SHORT_RELEASE){
              countDownMode = DOWN_M;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_2, ON);
            }
            else if (btn == BUTTON_2_LONG_PRESSED && seconds + minutes > 0){
              // start the timer
              countDownMode = COUNTING;
            }
            else if(btn == BUTTON_1_LONG_PRESSED){
              countDownMode = COUNT_RESET;
            }
            break;
          //---------------------------------------------------
          case DOWN_M: //1 minute decrement state
            MFS.writeLeds(LED_1 | LED_2 | LED_3 | LED_4, OFF);
            MFS.writeLeds(DEC_LED, ON);
            MFS.blinkLeds(DEC_LED, ON);
            if(btn == BUTTON_3_SHORT_RELEASE){
              //countDownMode = DOWN_M;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_2, ON);
              if(minutes > 0)
                minutes = minutes - 1;
              sprintf(txtBuffer, "%04d", minutes*100 + seconds);
              MFS.write(txtBuffer);
            }
            else if(btn == BUTTON_3_LONG_RELEASE){
              countDownMode = DOWN_10M;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_1, ON);
            }
            else if(btn == BUTTON_2_SHORT_RELEASE){
              countDownMode = COUNTING_STOPPED;
            }
            else if (btn == BUTTON_2_LONG_PRESSED && seconds + minutes > 0){
              // start the timer
              countDownMode = COUNTING;
            }
            else if(btn == BUTTON_1_LONG_PRESSED){
              countDownMode = COUNT_RESET;
            }
            break;
          //---------------------------------------------------
          case DOWN_10M: //10 minutes decrement state
            MFS.writeLeds(LED_1 | LED_2 | LED_3 | LED_4, OFF);
            MFS.writeLeds(DEC_LED, ON);
            MFS.blinkLeds(DEC_LED, ON);
            if(btn == BUTTON_3_SHORT_RELEASE){
              countDownMode = DOWN_M;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_2, ON);
            }
            else if(btn == BUTTON_3_LONG_RELEASE){
              //countDownMode = DOWN_10M;
              MFS.blinkDisplay(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4, OFF);
              MFS.blinkDisplay(DIGIT_1, ON);
              if(minutes >= 10)
                minutes = minutes - 10;
              sprintf(txtBuffer, "%04d", minutes*100 + seconds);
              MFS.write(txtBuffer);
            }
            else if(btn == BUTTON_2_SHORT_RELEASE){
              countDownMode = COUNTING_STOPPED;
            }
            else if (btn == BUTTON_2_LONG_PRESSED && seconds + minutes > 0){
              // start the timer
              countDownMode = COUNTING;
            }
            else if(btn == BUTTON_1_LONG_PRESSED){
              countDownMode = COUNT_RESET;
            }
            break;
        }
      }
  }
}

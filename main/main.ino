#include <avr/sleep.h>
#include "TimerOne.h"
#include "EnableInterrupt.h"

#define BUTTON_1 2
#define BUTTON_2 3
#define BUTTON_3 5
#define BUTTON_4 6
#define LED_1 7
#define LED_2 8
#define LED_3 10
#define LED_4 11
#define LED_RED 13
#define Potenziometer A0

int T1 = 2000; //pause between match
int T2 = 5000; //time show pattern
int T3 = 5000; //response time

long initStateTimeout = 10000; //10 sec
bool initState = true;
bool printInitMessage = true;
bool blinkFlag = false;
bool systemOnPause = false;
long startingTime;
long currentTime;
int life=3;
int points=0;
int factor=500;

bool setPattern=true;
bool showPattern=false;
bool pausePhase=false;
long showPatternInitialTime;
long responseInitialTime;
long pausePhaseInitialTime;
long errorInitialTime;
bool responsePhase=false;
bool notifyError=false;
const int errorNotificationTime=1000;
bool led_1_pattern = false;
bool led_2_pattern = false;
bool led_3_pattern = false;
bool led_4_pattern = false;
bool led_1_on = false;
bool led_2_on = false;
bool led_3_on = false;
bool led_4_on = false;


void shutDown(){
  digitalWrite(LED_RED,LOW);
  systemOnPause=true;
  Timer1.stop();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();
}

void wakeUp(){
  sleep_disable();
  Timer1.restart();
  startingTime=millis();
  printInitMessage=true;
  systemOnPause=false;
}

void turnOff(){
  digitalWrite(LED_1,LOW);
  digitalWrite(LED_2,LOW);
  digitalWrite(LED_3,LOW);
  digitalWrite(LED_4,LOW);
  led_1_on = false;
  led_2_on = false;
  led_3_on = false;
  led_4_on = false;
}

//must becomr fade not blick
void blinky(){
 if (!blinkFlag){
 digitalWrite(LED_RED, HIGH);
 } else {
 digitalWrite(LED_RED, LOW);
 }
 blinkFlag = !blinkFlag;
}

void startGame(){
  initState=false;
  setPattern=true;
  Timer1.stop();
  digitalWrite(LED_RED, LOW);
  Serial.println("Go!");
  int level = analogRead(Potenziometer);

  if(level<250){
    factor=62;
  } else if (level<500) {
    factor=125;
  } else if (level<750) {
    factor=250;
  } else if (level<1024) {
    factor=500;
  };

}

void endResponsePhase(){
  responsePhase=false;
  pausePhase=true;
  pausePhaseInitialTime=millis();
  turnOff();
}

void generatePattern(){
  led_1_pattern = (random(10)%2==0);
  led_2_pattern = (random(10)%2==0);
  led_3_pattern = (random(10)%2==0);
  led_4_pattern = (random(10)%2==0);
  setPattern=false;
  showPattern=true;
  showPatternInitialTime=millis();
  if(!led_1_pattern && !led_2_pattern && !led_3_pattern && !led_4_pattern){
    generatePattern();
  }
}

void displayPattern(){
  digitalWrite(LED_1,led_1_pattern?HIGH:LOW);
  digitalWrite(LED_2,led_2_pattern?HIGH:LOW);
  digitalWrite(LED_3,led_3_pattern?HIGH:LOW);
  digitalWrite(LED_4,led_4_pattern?HIGH:LOW);
}

void displayReponse(){
  digitalWrite(LED_1,led_1_on?HIGH:LOW);
  digitalWrite(LED_2,led_2_on?HIGH:LOW);
  digitalWrite(LED_3,led_3_on?HIGH:LOW);
  digitalWrite(LED_4,led_4_on?HIGH:LOW);
}

void initStateButton(){
    if(systemOnPause){  
      wakeUp();
    }else{
      startGame();
    }
}

void gameOver(){
  Serial.println("Game Over. Final Score: ");
  Serial.println(points);
  initState=true;
  pausePhase=false;
  notifyError=false;
  printInitMessage=true;
  startingTime=millis();
  Timer1.restart();
  life=3;
  points=0;
  turnOff();
}

void onError(){
  notifyError=true;
  errorInitialTime=millis();
  life=life-1;
  Serial.println("Penalty!");
  if(life<=0){
    gameOver();
  }
}

void hidePattern(){
   turnOff();
   responsePhase=true;
   showPattern=false;
   responseInitialTime=millis();
}

void earlyResponseCheck(){
  if(showPattern){
    hidePattern();
    endResponsePhase();
    onError();
  }
}

void onButton1Press(){
  if(initState){
    initStateButton();
  }else{
    if(responsePhase){
      led_1_on=true;
    } 
    earlyResponseCheck();
  }  
}

void onButton2Press(){
  if(initState){
    initStateButton();
  }else{
    if(responsePhase){
      led_2_on=true;
    }
    earlyResponseCheck();
  }  
}

void onButton3Press(){
  if(initState){
    initStateButton();
  }else{
    if(responsePhase){
      led_3_on=true;
    }
    earlyResponseCheck();
  }  
}

void onButton4Press(){
  if(initState){
    initStateButton();
  }else{ 
    if(responsePhase){
      led_4_on=true;
    }
    earlyResponseCheck();
  }  
}

void onCorrectAnswer(){
  Serial.println("New point! Score:");
  Serial.println(points);
  T2=T2-factor;
  T3=T3-factor;
  points=points+1;
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(blinky);
  startingTime=millis();
  randomSeed(analogRead(0));
  enableInterrupt(BUTTON_1, onButton1Press, RISING);
  enableInterrupt(BUTTON_2, onButton2Press, RISING);
  enableInterrupt(BUTTON_3, onButton3Press, RISING);
  enableInterrupt(BUTTON_4, onButton4Press, RISING);
}

void loop() {
  long currentTime = millis();
  if(initState){
    if(printInitMessage){
      Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
      printInitMessage=false;
    }
    if((currentTime-startingTime) > initStateTimeout){
      shutDown();
    }
  } else {
    if(setPattern){
      generatePattern();
    }
    if(showPattern){
      if((currentTime-showPatternInitialTime) < T2 ){
        displayPattern();
      } else {
        hidePattern();
      }
    }
    if(responsePhase){
      displayReponse();
    }
    if(responsePhase && (currentTime-responseInitialTime) > T3){
      if(
        led_1_pattern == led_1_on &&
        led_2_pattern == led_2_on &&
        led_3_pattern == led_3_on &&
        led_4_pattern == led_4_on 
      ){
        onCorrectAnswer();
      }else{
        onError();
      }
      endResponsePhase();
    }
    if(pausePhase){
       if((currentTime-pausePhaseInitialTime) > T1 ){
         setPattern=true;
         pausePhase=false;
       } 
       if(notifyError) {
         if((currentTime-errorInitialTime) > errorNotificationTime){
           digitalWrite(LED_RED, LOW);
           notifyError=false;
         } else {
           digitalWrite(LED_RED, HIGH);
         }
       }
    }
  }
}

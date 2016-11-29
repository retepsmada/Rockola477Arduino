#include "../Utilities/records.h"

//Count pulses from opto encoder on carousel until the correct record is reached
#define controlPulse 3
//Pulse to stop carosel spinning
#define controlStopSpin 4
//Take high untill home is low to start spin
#define controlStartSpin 5
//Take high to just rotate the carousel for record loading
#define controlScan 6
//Is high when the carousel is in the home position
#define controlHome 7
//Is low for side A and high for side B.
//If you want B and it's A, just do one more full rotation
#define controlSide 8
//High when a record is playing
#define pin 9

int Home = 0;
int incorrectSelection = 0;//goes to one if selection is incorrect
int pulseCount = 0;//Amount of pulses on the pulse pin in one selection cycle
int currentSelection = 0;//Current record displayed on screen
int currentState = 0; //Curent state of pulse pin
int lastState = 0;//Last state of pulse pin
int errorValue = 0;//used to detect if there has been a mechanical error
//Pin Variables

void recordSelect(int id){
  record recordFromId = makeRecord(id);
  digitalWrite(controlStartSpin, HIGH);
  delay(250);
  if (((recordFromId.side == 0) && (digitalRead(controlSide) == HIGH)) || ((recordFromId.side == 1) && (digitalRead(controlSide) == LOW))){
    
     while(digitalRead(controlHome) == 1);{
      Serial.print("1");
     }
      digitalWrite(controlStartSpin, LOW);
      Serial.print(digitalRead(controlHome));
     while(digitalRead(controlHome) == 0);{
        Serial.print("2");
      }
  }
  digitalWrite(controlStartSpin, HIGH);
  Serial.print("3");
     while(digitalRead(controlHome) == 1);{                        //need to change so that you can select a record at any time and so that you can add money at any time
        delay(10);
        ++errorValue;
        Serial.print("4");
     }
       digitalWrite(controlStartSpin, LOW);
       errorValue = 0;
  while(recordFromId.num != pulseCount){
    currentState = digitalRead(controlPulse);
    if((currentState != lastState) && (currentState == 1)){
      ++pulseCount;
      delay(10);
      Serial.print(pulseCount);
    }
    lastState = currentState;
  }
  digitalWrite(controlStopSpin, HIGH);
  delay(50);
  digitalWrite(controlStopSpin, LOW);
  pulseCount = 0;
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
 pinMode(controlSide, INPUT);
  pinMode(controlHome, INPUT);
  pinMode(controlScan, OUTPUT);
  pinMode(controlStartSpin, OUTPUT);
  pinMode(controlStopSpin, OUTPUT);
  pinMode(controlPulse, INPUT);
  pinMode(pin, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(digitalRead(pin) == 0){
    recordSelect(291);
  }
}

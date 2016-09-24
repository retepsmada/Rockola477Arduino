#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>


//Other Variables
#define unitsPerPlay //Pricing for a single song

//Set equal to the amount of money that has been put in, subtract minimumMoney when a selection is succesfully made
int unitMemory;
//Don't know how to impliment bonus options, where if one play is 25 cents then 2 is 45

//Pin Variables

//This is defined as the smallest coin that can be deposited,
//so a nickel-may change this later
#define buttonAddUnit 1
//TBD-Probably reset
#define buttonClear 2
//Subtracts one play if there are any in the buffer
#define buttonSubPlay 3
//Adds one play that can be used
#define buttonAddPlay 4
//TBD
#define buttonTest 5

//Count pulses from opto encoder on carousel until the correct record is reached
#define controlPulse 6
//Pulse to stop carosel spinning
#define controlStopSpin 7
//Take high untill home is low to start spin
#define controlStartSpin 8
//Take high to just rotate the carousel for record loading
#define controlScan 9
//Is high when the carousel is in the home position
#define controlHome 10
//Is low for side A and high for side B.
//If you want B and it's A, just do one more full rotation
#define controlSide 11

//Pulses when half dollar is inserted
#define halfDollar 12
//Pulses when quarter is inserted
#define quarter 13
//Pulses when nickel is inserted
#define nickel 14
//Pulses when dime is inserted
#define dime 15

//Pins A4 and A5 are automaticaly selected for use with the I2C displays

//Take high when the selection display is showing the selection just made
#define ledYourSelection 16
//Take high when the selection display is showing the record that is currently playing
#define ledRecordPlaying 17
//Take high for a certain amount of time if a button is pressed and unitMemory is less than unitsPerPlay
#define ledAddCoins 18
//Take high when the reset button is pressed and keep high until another button is pressed
#define ledResetReselect 19

//will probobly get multiplexing board for 7 pin keyboard

void setup(){
  pinMode(ledResetReselect, OUTPUT);
  pinMode(ledAddCoins, OUTPUT);
  pinMode(ledRecordPlaying, OUTPUT);
  pinMode(ledYourSelection, OUTPUT);
  pinMode(dime, INPUT);
  pinMode(nickel, INPUT);
  pinMode(quarter, INPUT);
  pinMode(halfDollar, INPUT);
  pinMode(controlSide, INPUT);
  pinMode(controlHome, INPUT);
  pinMode(controlScan, OUTPUT);
  pinMode(controlStartSpin, OUTPUT);
  pinMode(controlStopSpin, OUTPUT);
  pinMode(controlPulse, INPUT);
  pinMode(buttonTest, INPUT);
  pinMode(buttonAddPlay, INPUT);
  pinMode(buttonSubPlay, INPUT);
  pinMode(buttonClear, INPUT);
  pinMode(buttonAddUnit, INPUT);
}

void loop(){
  
}

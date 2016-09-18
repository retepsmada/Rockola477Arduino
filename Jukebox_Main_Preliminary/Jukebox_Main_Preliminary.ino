#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>


//Other Variables
int 
int 
int unitsPerPlay //Pricing for a single song
int unitMemory //Set equal to the amount of money that has been put in, subtract minimumMoney when a selection is succesfully made
int 
//Don't know how to impliment bonus options, where if one play is 25 cents then 2 is 45

//Pin Variables
int buttonAddUnit = 1//This is defined as the smallest coin that can be deposited, so a nickel-may change this later
int buttonClear = 2//TBD-Probably reset
int buttonSubPlay = 3//Subtracts one play if there are any in the buffer
int buttonAddPlay = 4//Adds one play that can be used
int buttonTest = 5//TBD
int controlPulse = 6//Count pulses from opto encoder on carousel untill the correct record is reached
int controlStopSpin = 7//Pulse to stop carosel spinning
int controlStartSpin = 8//Take high untill home is low to start spin
int controlScan = 9//Take high to just rotate the carousel for record loading
int controlHome = 10//Is high when the carousel is in the home position
int controlSide = 11//Is low for side A and high for side B. If you want B and it's A, just do one more full rotation
int halfDollar = 12//Pulses when half dollar is inserted
int quarter = 13//Pulses when quarter is inserted
int nickel = 14//Pulses when nickel is inserted
int dime = 15//Pulses when dime is inserted
//Pins A4 and A5 are automaticaly selected for use with the I2C displays
int ledYourSelection = 16//Take high when the selection display is showing the selection just made
int ledRecordPlaying = 17//Take high when the selection display is showing the record that is currently playing
int ledAddCoins = 18//Take high if a button is pressed and creditIn is less than minimumMoney
int ledResetReselect = 19//Take high when the reset button is pressed and keep high until annother button is pressed

//will probobly get multiplexing board for 7 pin keyboard


void setup(){
  pinMode(ledResetReselect, OUTPUT)
  pinMode(ledAddCoins, OUTPUT)
  pinMode(ledRecordPlaying, OUTPUT)
  pinMode(ledYourSelection, OUTPUT)
  pinMode(dime, INPUT)
  pinMode(nickel, INPUT)
  pinMode(quarter, INPUT)
  pinMode(halfDollar, INPUT)
  pinMode(controlSide, INPUT)
  pinMode(controlHome, INPUT)
  pinMode(controlScan, OUTPUT)
  pinMode(controlStartSpin, OUTPUT)
  pinMode(controlStopSpin. OUTPUT)
  pinMode(controlPulse, INPUT)
  pinMode(buttonTest, INPUT)
  pinMode(buttonAddPlay, INPUT)
  pinMode(buttonSubPlay, INPUT)
  pinMode(buttonClear, INPUT)
  pinMode(buttonAddUnit, INPUT)
  pinMode(
}


void loop(){
  if(

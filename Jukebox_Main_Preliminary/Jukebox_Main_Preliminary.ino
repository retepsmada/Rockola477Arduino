#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <SparkFunSX1509.h>
#include "../Utilities/records.h"

int incorrectSelection = 0;//goes to one if selection is incorrect
int pulseCount = 0;//Amount of pulses on the pulse pin in one selection cycle
int currentSelection = 0;//Current record displayed on screen
int currentState = 0; //Curent state of pulse pin
int lastState = 0;//Last state of pulse pin
int errorValue = 0;//used to detect if there has been a mechanical error
//Pin Variables

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
#define recordPlaying 9

//Pins A4 and A5 are automaticaly selected for use with the I2C displays and the multiplex board

//Take high when the selection display is showing the selection just made
#define ledYourSelection 10
//Take high when the selection display is showing the record that is currently playing
#define ledRecordPlaying 11
//Take high for a certain amount of time if a button is pressed and unitMemory is less than unitsPerPlay
#define ledAddCoins 12
//Take high when the reset button is pressed and keep high until another button is pressed
#define ledResetReselect 13

// SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default):
const byte SX1509_ADDRESS = 0x3E;  // SX1509 I2C address
SX1509 io; // Create an SX1509 object to be used throughout

#define KEY_ROWS 4 // Number of rows in the keypad matrix
#define KEY_COLS 4 // Number of columns in the keypad matrix

// keyMap maps row/column combinations to characters:
int keyMap[KEY_ROWS][KEY_COLS] = {
{ 2, 10, 7, 11},
{ 1, 4, 6, 9},
{ 0, 3, 5, 8},
{ 12, 13, 14, 15}};

const byte ARDUINO_INTERRUPT_PIN = 2;

Adafruit_7segment creditDisplay = Adafruit_7segment();
Adafruit_7segment selectionDisplay = Adafruit_7segment();

void clearCreditDisplay(){
  creditDisplay.writeDigitRaw(0,0);
  creditDisplay.writeDigitRaw(1,0);
  creditDisplay.writeDigitRaw(2,0);
  creditDisplay.writeDigitRaw(3,0);
  creditDisplay.writeDigitRaw(4,0);
  creditDisplay.writeDisplay();
}

void clearSelectionDisplay(){
  selectionDisplay.writeDigitRaw(0,0);
  selectionDisplay.writeDigitRaw(1,0);
  selectionDisplay.writeDigitRaw(2,0);
  selectionDisplay.writeDigitRaw(3,0);
  selectionDisplay.writeDigitRaw(4,0);
  selectionDisplay.writeDisplay();
}

void recordSelect(int id){
  record recordFromId = makeRecord(id);
  if (((recordFromId.side == 0) && (digitalRead(controlSide) == 1)) || ((recordFromId.side == 1) && (digitalRead(controlSide) == 0))){
     digitalWrite(controlStartSpin, HIGH);
     while(digitalRead(controlHome) != 0);
      digitalWrite(controlStartSpin, LOW);
      while(digitalRead(controlHome) != 1);
  }
  digitalWrite(controlStartSpin, HIGH);
    while(digitalRead(controlHome) != 0);
    digitalWrite(controlStartSpin, LOW);
  while(recordFromId.num != pulseCount){
    currentState = digitalRead(controlPulse);// this works
    if(currentState != lastState && currentState == HIGH){
      ++pulseCount;
    }
    lastState = currentState;
  }
  digitalWrite(controlStopSpin, HIGH);
  delay(50);
  digitalWrite(controlStopSpin, LOW);
}

void creditCounter(int coin){
  
}

void setup(){
  Serial.begin(9600);
  Serial.print("Tast.mp3");
  
  pinMode(ledResetReselect, OUTPUT);
  pinMode(ledAddCoins, OUTPUT);
  pinMode(ledRecordPlaying, OUTPUT);
  pinMode(ledYourSelection, OUTPUT);
  pinMode(controlSide, INPUT);
  pinMode(controlHome, INPUT);
  pinMode(controlScan, OUTPUT);
  pinMode(controlStartSpin, OUTPUT);
  pinMode(controlStopSpin, OUTPUT);
  pinMode(controlPulse, INPUT);
  pinMode(recordPlaying, INPUT);
  
  creditDisplay.begin(0x71);
  selectionDisplay.begin(0x70);


  if (!io.begin(SX1509_ADDRESS))
  {
    while (1); // If we fail to communicate, loop forever.
  }
  
  // Scan time range: 1-128 ms, powers of 2
  byte scanTime = 8; // Scan time per row, in ms
  // Debounce time range: 0.5 - 64 ms (powers of 2)
  byte debounceTime = 1; // Debounce time
  // Sleep time range: 128 ms - 8192 ms (powers of 2) 0=OFF
  byte sleepTime = 0;
  // Scan time must be greater than debounce time!
  io.keypad(KEY_ROWS, KEY_COLS, 
            sleepTime, scanTime, debounceTime);

  // Set up the Arduino interrupt pin as an input w/ 
  // internal pull-up. (The SX1509 interrupt is active-low.)
  pinMode(ARDUINO_INTERRUPT_PIN, INPUT_PULLUP);

  clearSelectionDisplay();
  clearCreditDisplay();
  digitalWrite(ledResetReselect, HIGH);
  digitalWrite(ledAddCoins, HIGH);
  digitalWrite(ledRecordPlaying, HIGH);
  digitalWrite(ledYourSelection, HIGH);
}

// Compared to the keypad in keypad.ino, this keypad example
// is a bit more advanced. We'll use these variables to check
// if a key is being held down, or has been released. Then we
// can kind of emulate the operation of a computer keyboard.
unsigned int previousKeyData = 0; // Stores last key pressed
unsigned int holdCount, releaseCount = 0; // Count durations
const unsigned int holdCountMax = 5; // Key hold limit
const unsigned int releaseCountMax = 100; // Release limit
int selectionDisplayCount = 1; //What display digit we're currently on

void loop(){

    // If the SX1509 INT pin goes low, a keypad button has
  // been pressed:
  if (digitalRead(ARDUINO_INTERRUPT_PIN) == LOW)
  {
    // Use io.readKeypad() to get the raw keypad row/column
    unsigned int keyData = io.readKeypad();
  // Then use io.getRow() and io.getCol() to parse that
  // data into row and column values.
    byte row = io.getRow(keyData);
    byte col = io.getCol(keyData);
  // Then plug row and column into keyMap to get which
  // key was pressed.
    int key = keyMap[row][col];
    
  // If it's a new key pressed
    if (keyData != previousKeyData)
    {
      holdCount = 0;
      if(key == 11){
        clearSelectionDisplay();
        selectionDisplayCount = 1;
        currentSelection = 0;
        incorrectSelection = 0;
        digitalWrite(ledResetReselect, HIGH);
      }
      else if(incorrectSelection == 0){
      if (key < 10 && key >= 0 && selectionDisplayCount <= 4){
       if((selectionDisplayCount == 1)&&(key >= 1 && key <= 2)){
          currentSelection = key * 100;
        selectionDisplay.writeDigitNum(selectionDisplayCount, key);
        selectionDisplay.writeDisplay();
        Serial.print(key);
       }
       else if((selectionDisplayCount == 3)&&(key >= 0 && key <= 9)){ //There's probably a better way to have the keys go into a three digit variable but I don't know it
        currentSelection = currentSelection + (key * 10);
        selectionDisplay.writeDigitNum(selectionDisplayCount, key);
        selectionDisplay.writeDisplay();
        Serial.print(key);
       }
       else if((selectionDisplayCount == 4)&&(key >= 0 && key <= 7)){
        currentSelection = key + currentSelection;
        selectionDisplay.writeDigitNum(selectionDisplayCount, key);
        selectionDisplay.writeDisplay();
        Serial.print(key);
        Serial.print(currentSelection);
       }
       else{
        digitalWrite(ledResetReselect, LOW);
        incorrectSelection = 1;
       }
        
        ++selectionDisplayCount;
        if(selectionDisplayCount == 2){++selectionDisplayCount;}
        delay(250);
      }
    }
    }
    else // If the button's beging held down:
    {
      holdCount++; // Increment holdCount
      if (holdCount > holdCountMax) // If it exceeds threshold
        Serial.println(key); // Print the key
    }
    releaseCount = 0; // Clear the releaseCount variable
    previousKeyData = keyData; // Update previousKeyData
  }
  
  // If no keys have been pressed we'll continuously increment
  //  releaseCount. Eventually creating a release, once the 
  // count hits the max.
  releaseCount++;
  if (releaseCount >= releaseCountMax)
  {
    releaseCount = 0;
    previousKeyData = 0;
  }
}


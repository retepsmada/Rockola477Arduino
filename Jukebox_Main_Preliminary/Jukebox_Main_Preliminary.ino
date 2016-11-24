#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>

//Set equal to the amount of money that has been put in, subtract minimumMoney when a selection is succesfully made
//int unitMemory;
//Don't know how to impliment bonus options, where if one play is 25 cents then 2 is 45

//Pin Variables

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

//Pins A4 and A5 are automaticaly selected for use with the I2C displays and the multiplex board

//Take high when the selection display is showing the selection just made
#define ledYourSelection 16
//Take high when the selection display is showing the record that is currently playing
#define ledRecordPlaying 17
//Take high for a certain amount of time if a button is pressed and unitMemory is less than unitsPerPlay
#define ledAddCoins 18
//Take high when the reset button is pressed and keep high until another button is pressed
#define ledResetReselect 19

// SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default):
const byte SX1509_ADDRESS = 0x3E;  // SX1509 I2C address
SX1509 io; // Create an SX1509 object to be used throughout

#define KEY_ROWS 4 // Number of rows in the keypad matrix
#define KEY_COLS 4 // Number of columns in the keypad matrix

// keyMap maps row/column combinations to characters:
char keyMap[KEY_ROWS][KEY_COLS] = {
{ '2', ' ', '7', 'R'},
{ '1', '4', '6', '9'},
{ '0', '3', '5', '8'},
{ 'N', 'D', 'Q', 'F'}};

const byte ARDUINO_INTERRUPT_PIN = 2;

Adafruit_7segment creditDisplay = Adafruit_7segment();
Adafruit_7segment selectionDisplay = Adafruit_7segment();

void setup(){
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
  
  creditDisplay.begin();
  selectionDisplay.begin();

 
  if (!io.begin(SX1509_ADDRESS))
  {
    while (1) ; // If we fail to communicate, loop forever.
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
}

// Compared to the keypad in keypad.ino, this keypad example
// is a bit more advanced. We'll use these varaibles to check
// if a key is being held down, or has been released. Then we
// can kind of emulate the operation of a computer keyboard.
unsigned int previousKeyData = 0; // Stores last key pressed
unsigned int holdCount, releaseCount = 0; // Count durations
const unsigned int holdCountMax = 15; // Key hold limit
const unsigned int releaseCountMax = 100; // Release limit
}

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
    char key = keyMap[row][col];
    
  // If it's a new key pressed
    if (keyData != previousKeyData)
    {
      holdCount = 0; // Reset hold-down count
      Serial.println(String(key)); // Print the key
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

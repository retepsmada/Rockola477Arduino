#include <Key.h>
#include <Keypad.h>
#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>

int moneyIn = 0;
int creditsIn = 0;
#define creditAmount 25
int incorrectSelection = 0;//goes to one if selection is incorrect
int pulseCount = 0;//Amount of pulses on the pulse pin in one selection cycle
int currentSelection = 0;//Current record displayed on screen
int currentPlaying = 123;//The record that is currently playing
int currentState = 0; //Curent state of pulse pin
int lastState = 0;//Last state of pulse pin
int errorValue = 0;//used to detect if there has been a mechanical error
//Pin Variables

//Count pulses from opto encoder on carousel until the correct record is reached
#define controlPulse 0
//Pulse to stop carosel spinning
#define controlStopSpin A0
//Take high untill home is low to start spin
#define controlStartSpin A1
//Take high to just rotate the carousel for record loading
#define controlScan A2
//Is high when the carousel is in the home position
#define controlHome A3
//Is low for side A and high for side B.
//If you want B and it's A, just do one more full rotation
#define controlSide A6
//High when a record is playing
#define recordPlaying A7

//Pins A4 and A5 are automaticaly selected for use with the I2C displays and the multiplex board

//Take high when the selection display is showing the selection just made
#define ledYourSelection 10
//Take high when the selection display is showing the record that is currently playing
#define ledRecordPlaying 11
//Take high for a certain amount of time if a button is pressed and unitMemory is less than unitsPerPlay
#define ledAddCoins 12
//Take high when the reset button is pressed and keep high until another button is pressed
#define ledResetReselect 13



//This is the keymap for the multiplex of all the buttons in the jukebox

const byte rows = 4;
const byte cols = 5;

char keys[rows][cols] = {
  {'R','9','8','N',' '},
  {'7','6','5','D',' '},
  {' ','4','3','Q',' '},
  {'2','1','0','F',' '}
};
byte rowPins[rows] = {4, 5, 6, 7}; //connect to the row pinouts of the keypad
byte colPins[cols] = {8, 9, 10, 11, 12}; //connect to the column pinouts of the keypad
Keypad keys2 = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

int previousKey = 0; // Stores last key pressed in int format
char charKey; //Stores the current key in char format
int key; //Stores the current key in int format
unsigned int releaseCount = 0; // Count durations
#define releaseCountMax 2000// Release limit



//These are the functions and definitions that control the two displays

int selectionDisplayCount = 1; //What display digit we're currently on
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

void updateCredit(){
  creditDisplay.print(creditsIn * 100);
  creditDisplay.writeDigitRaw(3,0);
  creditDisplay.writeDigitRaw(4,0);
  creditDisplay.writeDisplay();
}

//These functions write specific numbers to the displays and also modify some variables
void clearSelection(){
  clearSelectionDisplay();
  selectionDisplayCount = 1;
  currentSelection = 0;
  incorrectSelection = 0;
  digitalWrite(ledResetReselect, HIGH);
}

void updateCurrentSelection() {
  //Add key as a digit to currentSelection:
  currentSelection *= 10;
  currentSelection += key;
  //Write key to selectionDisplay:
  selectionDisplay.writeDigitNum(selectionDisplayCount, key);
  selectionDisplay.writeDisplay();
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

  //Here we make sure that everything is blank at startup
  creditDisplay.begin(0x71);
  selectionDisplay.begin(0x70);

  clearSelectionDisplay();
  clearCreditDisplay();
  
  digitalWrite(ledResetReselect, HIGH);
  digitalWrite(ledAddCoins, HIGH);
  digitalWrite(ledRecordPlaying, HIGH);
  digitalWrite(ledYourSelection, HIGH);
}

//This is the code that selects records, it converts the input to be usable and then selects the record

//This is a datatype that can be either "A" or "B".
enum side { A, B };
typedef enum side side;
//This is a datatype representing a record.
//It has both a side and a record number.
typedef struct {
  side side;
  int num;
} record;

void recordSelect(int id){
 /**
   * This function takes in a number assuming that:
   * * It has three digits.
   * * Its hundreds digit is 1 or 2.
   * * Its tens digit is 0-9.
   * * Its ones digit is 0-7.
   * It then returns a record corresponding to that id number.
  */
  //First, compute the digits in id:
  int digits[3];
  for (int i = 0; i < 3; i++) {
    //i % 10 always represents the ones digit of a number:
    digits[i] = id % 10;
    //Now, divide i by 10 to get rid of the last digit:
    id /= 10;
    /* This means that the next digit we add will be the digit right before the one we just added.
       Thus, digits[0] is the ones digit,
             digits[1] is the tens digit,
         and digits[2] is the hundreds digit. */
  }

  record recordFromId;
  //This sets the side to A if the hundreds digit is 1 and B otherwise.
  recordFromId.side = (digits[2] == 1) ? A : B;
  //This sets the record number to the ones digit times 10 plus the tens digit.
  recordFromId.num = digits[0]*10+digits[1]+1;
  if (((recordFromId.side == 0) && (digitalRead(controlSide) == 1)) || ((recordFromId.side == 1) && (digitalRead(controlSide) == 0))) {
  if (digitalRead(controlSide) ^ recordFromId.side) {
    digitalWrite(controlStartSpin, HIGH);
    while(digitalRead(controlHome) != LOW);
    digitalWrite(controlStartSpin, LOW);
    while(digitalRead(controlHome) != HIGH);
  }
  digitalWrite(controlStartSpin, HIGH);
  while(digitalRead(controlHome) != LOW);
  digitalWrite(controlStartSpin, LOW);
  while(recordFromId.num != pulseCount){
    currentState = digitalRead(controlPulse);// this works
    if (currentState != lastState && currentState == HIGH) {
      ++pulseCount;
    }
    lastState = currentState;
  }
  digitalWrite(controlStopSpin, HIGH);
  delay(50);
  digitalWrite(controlStopSpin, LOW);
  selectionDisplay.print(currentPlaying); //just testing this here
  selectionDisplay.writeDisplay();
  digitalWrite(ledRecordPlaying, LOW);
  pulseCount = 0;
  lastState = 0;
}

//This is the queue code

#define queueSize 100
//This is our queue of records that the user has inputted:
int queue[queueSize];
//This is the index of the first element in queue:
int start = 0;
//This is the number of records in the queue:
int numRecords = 0;

inline int isempty() { return (numRecords == 0); }
   
inline int isfull() { return (numRecords == queueSize); }
    
int pop() {
  //Get the first element in the queue:
  int data = queue[start];
  //Increment start to mark that the first element has been popped from the queue:
  start += 1;
  //If start is queueSize, then it is no longer a valid index, so make it 0:
  start %= queueSize;
  //Decrement numRecords:
  numRecords--;
  //Debugging:
  Serial.println("Popped");
  Serial.println(data);
  //Finally, return the popped element:
  return data;
}

int push(int data) {
  //Get the index of where the new element should be:
  int index = start+numRecords;
  //Now, if index >= queueSize, then it is no longer a valid index, so subtract it by queueSize:
  index %= queueSize;
  //Set the new element:
  queue[index] = data;
  //Increment numRecords:
  numRecords++;
  //Debugging:
  Serial.println("Pushed");
  Serial.println(queue[index]);
  Serial.println(data);
}





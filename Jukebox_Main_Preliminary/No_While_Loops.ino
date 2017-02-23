#include <Key.h>
#include <Keypad.h>
#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>

int moneyIn = 0;//In cents how much money is in the machine if there is less than one credit
int creditsIn = 0;//Credits, or plays that the user has
#define creditAmount 25//The amount of cents it takes to make one credit
int incorrectSelection = 0;//goes to one if selection is incorrect
int pulseCount = 0;//Amount of pulses on the pulse pin in one selection cycle
int currentSelection = 0;//Current record displayed on screen
int currentPlaying = 999;//The record that is currently playing
int currentState = 0; //Curent state of pulse pin
int lastState = 0;//Last state of pulse pin
int errorValue = 0;//used to detect if there has been a mechanical error
//Pin Variables

//Count pulses from opto encoder on carousel until the correct record is reached
#define controlPulse 13
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
#define ledYourSelection 1
//Take high when the selection display is showing the record that is currently playing
#define ledRecordPlaying 0
//Take high for a certain amount of time if a button is pressed and unitMemory is less than unitsPerPlay
#define ledAddCoins 2
//Take high when the reset button is pressed and keep high until another button is pressed
#define ledResetReselect 3


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
byte colPins[cols] = {10, 9, 8, 11, 12}; //connect to the column pinouts of the keypad
Keypad keys2 = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

int previousKey = 0; // Stores last key pressed in int format
char charKey; //Stores the current key in char format
int key; //Stores the current key in int format
unsigned int releaseCount = 0; // Count durations
#define releaseCountMax 500// Release limit
unsigned int holdCount = 0;//maximum hold time



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
  //Serial.begin(9600);
  //Serial.print("Tast.mp3");
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
//This is the record object associated with the last thing passed into recordSelect:
record recordFromId;
//The curRecordSelectState int tells us what state we are currently in for the recordSelect function:
enum state { BEGIN, DSIDE_HOME_LOW, DSIDE_HOME_HIGH, HOME_LOW, PULSE_COUNT };
int curRecordSelectState = BEGIN;

void recordSelect(int id){
 /**
   * This function takes in a number assuming that:
   * * It has three digits.
   * * Its hundreds digit is 1 or 2.
   * * Its tens digit is 0-9.
   * * Its ones digit is 0-7.
   * It then starts playing record corresponding to that id number.
  */
  //Do different things based off the state:
  switch (curRecordSelectState) {
    case BEGIN:
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
      //This sets the side to A if the hundreds digit is 1 and B otherwise.
      recordFromId.side = (digits[2] == 1) ? A : B;
      //This sets the record number to the ones digit times 10 plus the tens digit.
      recordFromId.num = digits[0]*10+digits[1]+1;

      digitalWrite(controlStartSpin, HIGH);
      delay(30);
      //If controlSide is different from the record's side, then we need to wait until controlHome is LOW, then HIGH, then LOW:
      if (digitalRead(controlSide) ^ recordFromId.side) curRecordSelectState = DSIDE_HOME_LOW;
      //Otherwise, we only need to wait until controlHome is LOW once:
      else curRecordSelectState = HOME_LOW;
      break;
      
    case DSIDE_HOME_LOW:
      //If controlHome is low, write to controlStartPin and wait for controlHome to become HIGH:
      if (digitalRead(controlHome) == LOW) {
        digitalWrite(controlStartSpin, LOW);
        curRecordSelectState = DSIDE_HOME_HIGH;
      }
      break;
      
    case DSIDE_HOME_HIGH:
      //If controlHome is high, write to controlStartPin and wait for controlHome to become LOW:
      if (digitalRead(controlHome) == HIGH) {
        digitalWrite(controlStartSpin, HIGH);
        curRecordSelectState = HOME_LOW;
      }
      break;
      
    case HOME_LOW:
      //If controlHome is low, write to controlStartPin and start incrementing pulseCount:
      if (digitalRead(controlHome) == LOW) {
        digitalWrite(controlStartSpin, LOW);
        curRecordSelectState = PULSE_COUNT;
      }
      break;
      
    case PULSE_COUNT:
      //If we're in the middle of incrementing pulseCount:
      if (recordFromId.num != pulseCount) {
        currentState = digitalRead(controlPulse);// this works
        if (currentState != lastState && currentState == HIGH) {
          ++pulseCount;
        }
        lastState = currentState;
      }
      //Once we're done:
      else {
        digitalWrite(controlStopSpin, HIGH);
        delay(50);
        digitalWrite(controlStopSpin, LOW);
        selectionDisplay.print(currentPlaying); //just testing this here
        selectionDisplay.writeDisplay();
        digitalWrite(ledRecordPlaying, LOW);
        pulseCount = 0;
        lastState = 0;
        //Stop calling recordSelect:
        curRecordSelectState = BEGIN;
      }
      break;
  }
}
//This is the queue code

#define queueSize 100
//This is our queue of records that the user has inputted:
int queue[queueSize];
//This is the index of the first element in queue:
int start = 0;
//This is the number of records in the queue:
int numRecords = 0;

static inline int isempty() { return (numRecords == 0); }
   
static inline int isfull() { return (numRecords == queueSize); }
    
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



void loop(){
  //If we're not calling recordSelect right now:
  if (curRecordSelectState == BEGIN) {
    keyboardRead();
    if ((digitalRead(recordPlaying) == 0) && (digitalRead(controlHome) == 1) && !isempty()){
      Serial.println("recorded");
      recordSelect(pop());
    }
  }
  //Otherwise, call recordSelect:
  //(What you pass in here doesn't matter since id is only used in the BEGIN state.)
  else recordSelect(0);
}

//This fucntion reads the keyboard, coin switches, and the PCB buttons
//It adds and subtracts credits accordingly and stores the current selection
//When a valid selection is made it stores it as currentSelection and subtracts one credit from creditsIn
void keyboardRead(){
  charKey = keys2.getKey();
  key = (int)charKey - '0';  //the char is just the raw ascii value so if we subtract '0' it is the original number
  if (key != previousKey) {
    //Clear releaseCount and set previousKey:
    previousKey = key;
    releaseCount = 0;
    if ((charKey == 'N')||(charKey == 'D')||(charKey == 'Q')||(charKey == 'F')){
      switch(charKey){
        case 'N':
          moneyIn += 5;
          break;
        case 'D':
          moneyIn += 10;
          break;
        case 'Q':
          moneyIn += 25;
          break;
        case 'F':
          moneyIn +=50;
          break;
      }
      Serial.print("Money: ");
      Serial.println(moneyIn);
      while (moneyIn >= creditAmount){
        creditsIn += 1;
        moneyIn -= creditAmount;
        updateCredit();
        Serial.print("Credits: ");
        Serial.println(creditsIn);
      }
    }
    else if (charKey == 'R') {
      clearSelection();
      digitalWrite(ledAddCoins, HIGH);
      digitalWrite(ledYourSelection, HIGH);
      digitalWrite(ledResetReselect, HIGH);
      return;
    }
    else if(incorrectSelection == 0) {
      if (key < 10 && key >= 0 && selectionDisplayCount <= 4) {
        //Let the first digit be either a 1 or a 2:
        if ((selectionDisplayCount == 1)&&(key >= 1 && key <= 2)) {
          updateCurrentSelection();
          digitalWrite(ledYourSelection, LOW);
          Serial.println(key);
        }
        //Let the second digit be 0-9:
        else if ((selectionDisplayCount == 3)&&(key >= 0 && key <= 9)){
          updateCurrentSelection();
        }
        //Let the third digit be 0-7:
        else if((selectionDisplayCount == 4)&&(key >= 0 && key <= 7)){
          updateCurrentSelection();
          //Selection is valid
          Serial.print(currentSelection);
          //Push the new selection in only if they have a credit and the queue is not full:
          if (creditsIn){
            if (!isfull()) {
              push(currentSelection);
              currentPlaying = currentSelection;
              clearSelection();
              creditsIn -= 1;
              updateCredit();
              currentSelection = 0;
              digitalWrite(ledYourSelection, HIGH);
              selectionDisplay.print(currentPlaying);
              selectionDisplay.writeDisplay();
              digitalWrite(ledRecordPlaying, LOW);
              Serial.println("Removed");
              return;
            }
          }
          //If they do not have enough credits, then tell them to add more coins:
          else{
            digitalWrite(ledAddCoins, LOW);
          }
        }
        //If one of the digits does not match the given criteria, set incorrectSelection:
        else{
          digitalWrite(ledResetReselect, LOW);
          incorrectSelection = 1;
        }

        ++selectionDisplayCount;
        if(selectionDisplayCount == 2){++selectionDisplayCount;}
      }
    }
  }
  //If there is no key or we are still pressing the same key:
  else {
    //Increment releaseCount:
    releaseCount++;
    //If releaseCount reaches the max, reset so that the key can be pressed again:
    if (releaseCount >= releaseCountMax) {
      previousKey = 0;
      releaseCount = 0;
    }
  }
}
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
int currentSelection = 0;//Current record displayed on screen
int currentPlaying = 999;//The record that is currently playing //In both files


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


void loop(){
  keyboardRead();
  if ((digitalRead(recordPlaying) == 0) && (digitalRead(controlHome) == 1) && !isempty()){
    Serial.println("recorded");
    recordSelect(pop());
  }
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

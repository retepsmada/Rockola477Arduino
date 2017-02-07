#include <Key.h>
#include <Keypad.h>
#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>


int incorrectSelection = 0;//goes to one if selection is incorrect
int pulseCount = 0;//Amount of pulses on the pulse pin in one selection cycle
int currentSelection = 0;//Current record displayed on screen
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

int moneyIn = 0;
int creditsIn = 0;
#define creditAmount 25

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

const byte rows = 4;
const byte cols = 5;

int keys[rows][cols] = {
  {11,9,8,12,16},
  {7,6,5,13,16},
  {10,4,3,14,16},
  {2,1,0,15,16}
};
byte rowPins[rows] = {1, 0, 2, A7}; //connect to the row pinouts of the keypad
byte colPins[cols] = {A0, A1, A2, A3, A6}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

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

  clearSelectionDisplay();
  clearCreditDisplay();
  
  digitalWrite(ledResetReselect, HIGH);
  digitalWrite(ledAddCoins, HIGH);
  digitalWrite(ledRecordPlaying, HIGH);
  digitalWrite(ledYourSelection, HIGH);
}


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

  //if (((recordFromId.side == 0) && (digitalRead(controlSide) == 1)) || ((recordFromId.side == 1) && (digitalRead(controlSide) == 0))) {
  if (digitalRead(controlSide) == 1-recordFromId.side) {
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
    if (currentState != lastState && currentState == HIGH) {
      ++pulseCount;
    }
    lastState = currentState;
  }
  digitalWrite(controlStopSpin, HIGH);
  delay(50);
  digitalWrite(controlStopSpin, LOW);
}

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
}


// Compared to the keypad in keypad.ino, this keypad example
// is a bit more advanced. We'll use these variables to check
// if a key is being held down, or has been released. Then we
// can kind of emulate the operation of a computer keyboard.
unsigned int previousKeyData = 0; // Stores last key pressed
unsigned int releaseCount = 0; // Count durations
// Release limit
#define releaseCountMax 1000
int selectionDisplayCount = 1; //What display digit we're currently on

void loop(){
  keyboardRead();
  if (digitalRead(recordPlaying) == LOW && digitalRead(controlHome) == HIGH && !isempty()){
    recordSelect(pop());
  }

}

void clearSelection(){
  clearSelectionDisplay();
  selectionDisplayCount = 1;
  currentSelection = 0;
  incorrectSelection = 0;
  digitalWrite(ledResetReselect, HIGH);
}

unsigned int keyData;
int key;

void updateCurrentSelection() {
  //Add key as a digit to currentSelection:
  currentSelection *= 10;
  currentSelection += key;
  //Write key to selectionDisplay and Serial:
  selectionDisplay.writeDigitNum(selectionDisplayCount, key);
  selectionDisplay.writeDisplay();
  Serial.println("UCS");
  Serial.print(key);
}

void keyboardRead(){

    if ((keyData != previousKeyData) && (key != NO_KEY)) {
      previousKeyData = keyData;
      if (key > 11 && key < 16){
        switch(key){
          case 12:
            moneyIn += 5;
            break;
          case 13:
            moneyIn += 10;
            break;
          case 14:
            moneyIn += 25;
            break;
          case 15:
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
      else if (key == 11) {
        clearSelection();
        digitalWrite(ledAddCoins, HIGH);
        digitalWrite(ledYourSelection, HIGH);
        return;
      }
      else if(incorrectSelection == 0) {
        if (key < 10 && key >= 0 && selectionDisplayCount <= 4) {
          //Let the first digit be either a 1 or a 2:
          if ((selectionDisplayCount == 1)&&(key >= 1 && key <= 2)) {
            updateCurrentSelection();
            digitalWrite(ledYourSelection, LOW);
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
                clearSelection();
                push(currentSelection);
                creditsIn -= 1;
                updateCredit();
                currentSelection = 0;
                digitalWrite(ledYourSelection, HIGH);
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
}

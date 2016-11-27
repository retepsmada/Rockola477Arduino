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
#define pin 9
//This is a datatype that can be either "A" or "B".
enum side { A, B };
typedef enum side side;
//This is a datatype representing a record.
//It has both a side and a record number.
typedef struct {
    side side;
    int num;
} record;

int incorrectSelection = 0;//goes to one if selection is incorrect
int pulseCount = 0;//Amount of pulses on the pulse pin in one selection cycle
int currentSelection = 0;//Current record displayed on screen
int currentState = 0; //Curent state of pulse pin
int lastState = 0;//Last state of pulse pin
int errorValue = 0;//used to detect if there has been a mechanical error
//Pin Variables


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
    recordFromId.num = digits[0]*10+digits[1];
    Serial.print(recordFromId.side);
    Serial.print(recordFromId.num);
  if (((recordFromId.side == 0) && (digitalRead(controlSide) == 1)) || ((recordFromId.side == 1) && (digitalRead(controlSide) == 0))){
     digitalWrite(controlStartSpin, HIGH);
     while(digitalRead(controlHome) == 1);{
      Serial.print("1");
     }
      digitalWrite(controlStartSpin, LOW);
      errorValue = 0;
      Serial.print(digitalRead(controlHome));
     while(digitalRead(controlHome) == 0);{
        Serial.print("2");
      }
      errorValue = 0;
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
  pinMode(recordPlaying, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(digitalRead(pin) == 1){
    recordSelect(167);
  }
}

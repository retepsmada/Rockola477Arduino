//Count pulses from opto encoder on carousel untill the correct record is reached
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
#define buttonPin 12

int pulseNumber = 0;
int sideNumber = 0;
#define recordNumber 123

void setup() {
  // put your setup code here, to run once:
  pinMode(controlSide, INPUT);
  pinMode(controlHome, INPUT);
  pinMode(controlScan, OUTPUT);
  pinMode(controlStartSpin, OUTPUT);
  pinMode(controlStopSpin, OUTPUT);
  pinMode(controlPulse, INPUT);
  pinMode(buttonPin, INPUT);
}

void loop() {
  if(((recordNumber - 97)<=100) && (digitalRead(controlSide)==HIGH))
  {
    
  }
  else
  {
    //if(
  }
}
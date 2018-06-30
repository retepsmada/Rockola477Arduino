int pulseCount = 0;//Amount of pulses on the pulse pin in one selection cycle
int currentState = 0; //Curent state of pulse pin
int lastState = 0;//Last state of pulse pin
int errorValue = 0;//used to detect if there has been a mechanical error
int currentPlaying = 999;//The record that is currently playing //In both files

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

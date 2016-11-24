/**
 * This is me trying to code the Jukebox.ino flow chart as best as I can.
 * @author Noble H. Mushtak
*/

#define unitsPerPlay 25
int unitsIn = 0;
int playCount = 0;

/**
 * Takes in a pin number and value of coin in cents.
 * Then, checks if pin is HIGH and if so, deals with the coin accordingly.
*/
void checkCoin(int pin, int value) {
    //Only do something if the pin is HIGH:
    if (digitalRead(pin) == HIGH) {
        //Increment unitsIn:
        unitsIn += value;
        //If there is enough coins for another play:
        if (unitsIn >= unitsPerPlay) {
            //Increment playCount and decrease unitsIn
            unitsIn -= unitsPerPlay, playCount++;
            //This method be defined later:
            print(playCount, 78);
        }
    }
}

void loop() {
    //Check all four coins:
    checkCoin(nickel, 5);
    checkCoin(dime, 10);
    checkCoin(quarter, 25);
    checkCoin(halfDollar, 50);
    //Now, I'm not exactly sure how the key pins are supposed to work, so I'll implement that later.
}
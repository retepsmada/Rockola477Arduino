#pragma once

//This is a datatype that can be either "A" or "B".
enum side { A, B };
typedef enum side side;
//This is a datatype representing a record.
//It has both a side and a record number.
typedef struct {
    side side;
    int num;
} record;

record makeRecord(int id) {
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
    return recordFromId;
}
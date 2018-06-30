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

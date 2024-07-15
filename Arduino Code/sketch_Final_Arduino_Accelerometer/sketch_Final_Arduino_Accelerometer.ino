#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

struct dataLine{
  unsigned long dataT;
  float dataX;
  float dataY;
  float dataZ;
};

unsigned long initialTime;
bool startDataCollection;
int dataCounter;
unsigned long dur;
bool dataCollected;
dataLine data[12];
sensors_event_t event;


/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

void setup(void) {
  Serial.begin(115200);
  while(!Serial);
  if(!accel.begin()) {
    while(1);
  }

  dataCounter = -10;
  startDataCollection = false;
  dataCollected = false;

  /* Set the range to whatever is appropriate for your project */
  accel.setRange(ADXL345_RANGE_16_G); // CHECK THIS HEHE
  // accel.setRange(ADXL345_RANGE_8_G);
  // accel.setRange(ADXL345_RANGE_4_G);
  // accel.setRange(ADXL345_RANGE_2_G);
}

void loop(void) {
  /* Get a new sensor event */
  accel.getEvent(&event);

  while (Serial.available() > 0 && dataCounter == -10) {
    String message = Serial.readStringUntil("\r"); // every time you call Serial.read() it will read a new message, so you only want to call it once per check
    // Serial.println("Received: " + message + " hehe"); // Debugging
    // Serial.println(message);

    if (message == "2500") { // i’m guessing duration is in ms
      dur = message.toInt(); // you will need to cast this as an int, not sure if this is the correct notation
      // Serial.println("made it past message");

      dataCounter = 0; // empty the array from previous times, check notation
      dataCollected = false; // to mark that you are not yet done with collection
      // Serial.println("Data collection started for duration: " + message + "ms"); Debugging
      //delay(500);
      startDataCollection = true; // to know it’s time to begin the process
    }
    
    else if (message == "2550") { // i’m guessing duration is in ms
      dur = message.toInt(); // you will need to cast this as an int, not sure if this is the correct notation
      // Serial.println("made it past message");

      dataCounter = 0; // empty the array from previous times, check notation
      dataCollected = false; // to mark that you are not yet done with collection
      // Serial.println("Data collection started for duration: " + message + "ms"); Debugging
      //delay(500);
      startDataCollection = true; // to know it’s time to begin the process
    }
  }

  if(startDataCollection) {
    if(dataCounter == 0) {
      // Serial.println("We got inside of if(sizeof..)")  ;
      initialTime = millis();
      dataCounter = 0; // assuming arduino goes from 0 (rather than 1), if not adjust here
    }

    if(millis() - initialTime <= dur) { // keep collecting data until it’s time
      if(dataCounter < 12) {
        dataLine dataTemp = {millis() - initialTime, event.acceleration.x, event.acceleration.y, event.acceleration.z};
        data[dataCounter] = dataTemp;
        dataCounter++;
      }
    }
    else { // we are done collecting data
      dataCollected = true;
      startDataCollection = false;
      // Serial.println("We got inside the else statement");
    }
  }

  // want to check that all the data during that duration has been saved
  if(dataCollected) {
    // Serial.println("Sending over the data");
    Serial.println("time,x,y,z");
    for (int i = 0; i < dataCounter; i++) {
      Serial.println(String(data[i].dataT) + "," + String(data[i].dataX) + "," + String(data[i].dataY) + "," + String(data[i].dataZ));
    }
    Serial.println("end");
    dataCollected = false; // to stop from resending
    dataCounter = -10;
  }
}


#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Vector.h>
#include <SPI.h>
#include <SD.h>

File mainFile;

int initialTime;
int dataCounter;
int dur;
int stringCount = 0;
float amplitude;
int frequency;
int modulation;
int pattern;
int iter = 0;

bool startDataCollection;
bool dataCollected;
bool nameFile;

String strs[10];
String duration;
String amp;

sensors_event_t event;

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

void setup(void) {
  Serial.begin(9600);
  while(!Serial);
  if(!accel.begin()) {
    while(1);
  }

  if (!SD.begin(10)) {
    Serial.println("Failed to initialize SD Card");
    return;
  }

  dataCounter = -10;
  startDataCollection = false;
  dataCollected = false;
  nameFile = false;

  /* Set the range to whatever is appropriate for your project */
  accel.setRange(ADXL345_RANGE_16_G);
  // accel.setRange(ADXL345_RANGE_8_G);
  // accel.setRange(ADXL345_RANGE_4_G);
  // accel.setRange(ADXL345_RANGE_2_G);
}

void loop(void) {
  /* Get a new sensor event */
  accel.getEvent(&event);

  while (Serial.available() > 0 && dataCounter == -10) {
    String message = Serial.readStringUntil("\r");
    stringCount = 0;
    while (message.length() > 0) {
      int index = message.indexOf(" ");
      if (index == -1) {
        strs[stringCount++] = message;
        break;
      }
      else {
        strs[stringCount++] = message.substring(0, index);
        message = message.substring(index+1);
      }
    }

    duration = strs[0];
    amplitude = (strs[2]).toFloat();
    modulation = (strs[3]).toInt();
    frequency = (strs[1]).toInt();
    pattern = (strs[4]).toInt();
    
    if (frequency == 250) {
      frequency = 25;
    }
    else {
      frequency = 8;
    }

    if (amplitude == 0.1) {
      amp = "L";
    }
    else {
      amp = "H";
    }
    
    Serial.println(message);
    Serial.println(duration);
    Serial.println(amplitude);
    Serial.println(modulation);
    Serial.println(frequency);
    Serial.println(pattern);

    if (duration == "2500" || duration == "2550") {
      dur = duration.toInt();
      dataCounter = 0; // empty the array from previous times
      dataCollected = false; // to mark that you are not yet done with collection
      startDataCollection = true; // to know it’s time to begin the process
      nameFile = true;
    }
  }

  if (nameFile) {
    mainFile = SD.open(String(frequency) + String(amp) + String(modulation) + ".csv", FILE_WRITE);
    iter++; //
    if (!mainFile) {
      Serial.println("Failed to open mainFile for writing");
      return;
    }
    nameFile = false;
  }

  if(startDataCollection) {
    if(dataCounter == 0) {
      initialTime = millis();
      dataCounter = 0;
    }
    if(millis() - initialTime <= dur) { // keep collecting data until it’s time
      dataCounter++;
      mainFile.print(millis() - initialTime);
      mainFile.print(",");
      mainFile.print(event.acceleration.x);
      mainFile.print(",");
      mainFile.print(event.acceleration.y);
      mainFile.print(",");
      mainFile.println(event.acceleration.z);
    }
    else { // we are done collecting data
      dataCollected = true;
      startDataCollection = false;
    }
  }

  // want to check that all the data during that duration has been saved
  if(dataCollected) {
    mainFile.close();
    Serial.println("end");
    dataCollected = false; // to stop from resending
    dataCounter = -10;

    //String(frequency) + String(amplitude) + String(modulation) + String(iter) +
  }
}
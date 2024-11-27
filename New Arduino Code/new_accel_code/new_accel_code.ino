// import libraries
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM6DS3TRC.h>
#include <Vector.h>
#include <SPI.h>
#include <SD.h>

// initializa variables
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

// Initialize constructor
sensors_event_t accelEvent;

/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM6DS3TRC adaf = Adafruit_LSM6DS3TRC();

void setup(void) {
  Serial.begin(115200);
  while(!Serial);

  if(!adaf.begin_I2C()) {
    while(1);
  }

  if (!SD.begin(10)) {
    Serial.println("Failed to initialize SD Card"); // check if sd card is there
    return;
  }

  dataCounter = -10;
  startDataCollection = false;
  dataCollected = false;
  nameFile = false;
  pinMode(3, OUTPUT);

  /* Set the range to whatever is appropriate for your project */
  // accel.setRange(ADXL345_RANGE_16_G);
  // accel.setRange(ADXL345_RANGE_8_G);
  // accel.setRange(ADXL345_RANGE_4_G);
  adaf.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
}

void loop(void) {
  /* Get a new sensor event */
  sensors_vec_t accel;

  adaf.readAcceleration(accel.x,accel.y,accel.z);

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

    if (amplitude == 0.177) {
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

    if (duration >= "3500") {
      dur = duration.toInt();
      dataCounter = 0; // empty the array from previous times
      dataCollected = false; // to mark that you are not yet done with collection
      startDataCollection = true; // to know it’s time to begin the process
      nameFile = true;
    }
  }

  if (nameFile) {
    mainFile = SD.open(String(frequency) + String(amp) + String(modulation) + "_" + String(pattern) + ".csv", FILE_WRITE);
    iter++; //
    if (!mainFile) {
      Serial.println("Failed to open mainFile for writing");
      return;
    }
    nameFile = false;
  }

  if(startDataCollection) {
    digitalWrite(3, HIGH);
    if(dataCounter == 0) {
      initialTime = millis();
      dataCounter = 0;
    }
    if(millis() - initialTime <= (dur + 1000)) { // keep collecting data until it’s time
      dataCounter++;
      mainFile.print(millis() - initialTime);
      mainFile.print(",");
      mainFile.print(accel.x);
      mainFile.print(",");
      mainFile.print(accel.y);
      mainFile.print(",");
      mainFile.println(accel.z);
    }
    else { // we are done collecting data
      dataCollected = true;
      startDataCollection = false;
      digitalWrite(3, LOW);
    }
  }

  // want to check that all the data during that duration has been saved
  if(dataCollected) {
    mainFile.close();
    Serial.println("end");
    dataCollected = false; // to stop from resending
    dataCounter = -10;
  }
}

// have arduino led blink when data saving occurs during VCM run for debugging
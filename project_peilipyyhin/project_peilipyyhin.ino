#include <Wire.h>

/*Example sketch to control a stepper motor with A4988/DRV8825 stepper motor driver and Arduino without a library. More info: https://www.makerguides.com */

#include <math.h>
#include "DHT20.h"
//#include "wire.h"

// Define stepper motor connections and steps per revolution:
#define dirPin 2
#define stepPin 3

static const uint8_t dataPin = 5;
static const uint8_t clockPin = 6;

// Globals for data reading
#define clockSpeed 400000
#define readDelay 1000

// Globals for motor stuff
#define stepsPerRevolution 50
#define stepDelay 6000

// Define the DHT20 and wire
DHT20 DHT;

// Data value holders
float hum, temp, dew;
float a = 17.27;
float b = 237.7;

// Offset for calibration
static const int hum_offset = 0;
static const int temp_offset = 0;

void setup() {
  
  // Declare pins as output:
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  
  // Setup the DHT
  Wire.begin();
  DHT.begin();
  Wire.setClock(400000);
  Serial.begin(115200);

  // Calibrating
  DHT.setHumOffset(hum_offset);
  DHT.setTempOffset(temp_offset);
  
}

void loop() {

  // READ every readDelay ms
  if (millis() - DHT.lastRead() >= readDelay) {
    // READ DATA
    uint32_t start = micros();
    int status = DHT.read();
    uint32_t stop = micros();
    switch (status){
      case DHT20_OK:
        Serial.print("OK,\t");
        break;
      case DHT20_ERROR_CHECKSUM:
        Serial.print("Checksum error,\t");
        break;
      case DHT20_ERROR_CONNECT:
        Serial.print("Connect error,\t");
        break;
      case DHT20_MISSING_BYTES:
        Serial.print("Missing bytes,\t");
        break;
      default:
        Serial.print("Unknown error,\t");
        break;
    }

    // Read the data values
    hum = DHT.getHumidity();
    temp = DHT.getTemperature();

    // Calculate the dew point
    dew = (b*(a*temp)/(b+temp)+log(hum))/(a-(a*temp)/(b+temp)+log(hum));

    bool conditional = temp < dew;
    
    // Print the data to serial
    Serial.print("DHT20, \t");
    Serial.print(hum, 1);
    Serial.print(",\t");
    Serial.print(temp, 1);
    Serial.print(",\t");
    Serial.print(dew, 1);
    Serial.print(",\t");
    Serial.print("condition met? ");
    Serial.print(conditional ? "Yup" : "Nup");
    Serial.print("\n");
  }

  
  
  // Set the spinning direction clockwise:
  digitalWrite(dirPin, HIGH);

  // Spin the stepper motor 1 revolution slowly:
  for (int i = 0; i < stepsPerRevolution; i++) {
    // These four lines result in 1 step:
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
  }

  delay(1000);
  
  // Set the spinning direction counterclockwise:
  digitalWrite(dirPin, LOW);

  // Spin the stepper motor 1 revolution quickly:
  for (int i = 0; i < stepsPerRevolution; i++) {
    // These four lines result in 1 step:
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
  }

  delay(1000);
  
}

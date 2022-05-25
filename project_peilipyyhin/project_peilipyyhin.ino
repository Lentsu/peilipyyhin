/*Example sketch to control a stepper motor with A4988/DRV8825 stepper motor driver and Arduino without a library. More info: https://www.makerguides.com */
#include <Wire.h>
#include <math.h>
#include "DHT20.h"

// Uncomment for debug
//#define DEBUG

// Offset for calibration if DEBUG is defined
#ifdef DEBUG
  #define hum_offset 100
  #define temp_offset 100
#else
  #define hum_offset 0
  #define temp_offset 0
#endif

// Define stepper motor pins
#define dirPin 2
#define stepPin 3

// Data reading
#define clockSpeed 400000
#define readDelay 1000

// Globals for stepper motor
//  Steps and direction
#define stepsPerRevolution 50
#define DIRECTION 1
//  wipe delays
#define stepDelay 12000
#define delayAmount 1000
#define middleDelay 400

// Define the DHT20 and wire
DHT20 DHT;

// Data value holders
float hum, temp;
bool conditional;

// The minimum environment condition for a wipe
static const int hum_min = 50;    // Wipe also whenever hum_min + 30
static const int temp_min = 35;

// Function to wipe once
void wipe();

void setup() {
  
  // Declare pins as output:
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  
  // Setup the DHT
  Wire.begin();
  Wire.setClock(400000);
  DHT.begin();
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

    // Calculate the conditional
    conditional = ((hum >= hum_min && temp >= temp_min) || (hum >= (hum_min + 10)));
    
    // Print the data to serial
    Serial.print("DHT20, \t");
    Serial.print(hum, 1);
    Serial.print("/("); Serial.print(hum_min, 1); Serial.print(")%");
    Serial.print(",\t");
    Serial.print(temp, 1);
    Serial.print("/("); Serial.print(temp_min, 1); Serial.print(")C");
    Serial.print(",\t");
    Serial.print("Wipe: \t");
    Serial.print(conditional ? "Yes" : "No");
    Serial.print("\n");
  }
  
  // Wipe once if condition is met
  if (conditional) 
    wipe();

  // Delay for delay amount
  delay(delayAmount);
    
}

// Move the motor a single step to direction
static void step_(int sPin, int dPin, int d) {

  // Set direction
  digitalWrite(dirPin, d);
  // Step
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(stepDelay/2);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(stepDelay/2);

}

// Move the motor n steps
static void n_step_(int sPin, int dPin, int d, int n) {
  for (int i = 0; i < n; ++i) {
    step_(sPin, dPin, d);
  }
}

// Wipe once
void wipe() {

  // Move the motor half a revolution TO direction
  n_step_(stepPin, dirPin, DIRECTION, stepsPerRevolution/2);

  // Delay for middleDelay
  delay(middleDelay);
  
  // Move the motor a revolution FROM direction
  n_step_(stepPin, dirPin, !DIRECTION, stepsPerRevolution);

  // Delay for middleDelay
  delay(middleDelay);

  // Move back to the middle
  n_step_(stepPin, dirPin, DIRECTION, stepsPerRevolution/2);

  // Delay for middleDelay
  delay(middleDelay);
}

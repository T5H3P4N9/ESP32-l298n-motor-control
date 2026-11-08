// ESP32-S3 N16R8 with an L298N motor driver.
// Potentiometer controls speed; button toggles direction.

const int analogPin = 7;          // Potentiometer wiper
const int motorControlPin = 6;    // L298N ENA (remove ENA jumper for PWM)
const int motorIn1Pin = 4;        // L298N IN1
const int motorIn2Pin = 5;        // L298N IN2
const int led1 = 15;
const int led2 = 16;
const int btnPin = 17;            // Button wired to 3.3 V; INPUT_PULLDOWN

const int sensorInputMinValue = 0;
const int sensorInputMaxValue = 4095;
const int pwmMinValue = 0;
const int pwmMaxValue = 255;
const int debounceTimeMs = 500;

volatile int mode = 0;
volatile unsigned long lastInterrupt = 0;

int previousMode = -1;
unsigned long lastPrint = 0;

void IRAM_ATTR interruptHandler() {
  const unsigned long now = millis();
  if (now - lastInterrupt > debounceTimeMs) {
    lastInterrupt = now;
    mode = (mode + 1) % 2;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(motorControlPin, OUTPUT);
  pinMode(motorIn1Pin, OUTPUT);
  pinMode(motorIn2Pin, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(btnPin, INPUT_PULLDOWN);

  attachInterrupt(digitalPinToInterrupt(btnPin), interruptHandler, RISING);
}

void loop() {
  const int sensorInputValue = analogRead(analogPin);
  const int boundedSensorValue = constrain(sensorInputValue,
                                           sensorInputMinValue,
                                           sensorInputMaxValue);
  const int motorOutputControl = map(boundedSensorValue,
                                     sensorInputMinValue, sensorInputMaxValue,
                                     pwmMinValue, pwmMaxValue);
  const int speedPercentage = map(boundedSensorValue,
                                  sensorInputMinValue, sensorInputMaxValue,
                                  0, 100);

  analogWrite(motorControlPin, motorOutputControl);

  // Copy the ISR-updated value before using it in the main loop.
  const int currentMode = mode;
  if (currentMode != previousMode) {
    if (currentMode == 0) {
      leftDirection();
      Serial.println("Direction: Left");
    } else {
      rightDirection();
      Serial.println("Direction: Right");
    }
    previousMode = currentMode;
  }

  if (millis() - lastPrint >= 200) {
    lastPrint = millis();
    Serial.print("Speed: ");
    Serial.print(speedPercentage);
    Serial.print("% | Sensor: ");
    Serial.print(sensorInputValue);
    Serial.print(" | PWM: ");
    Serial.println(motorOutputControl);
  }
}

void leftDirection() {
  digitalWrite(motorIn1Pin, HIGH);
  digitalWrite(motorIn2Pin, LOW);
  digitalWrite(led1, HIGH);
  digitalWrite(led2, LOW);
}

void rightDirection() {
  digitalWrite(motorIn1Pin, LOW);
  digitalWrite(motorIn2Pin, HIGH);
  digitalWrite(led1, LOW);
  digitalWrite(led2, HIGH);
}

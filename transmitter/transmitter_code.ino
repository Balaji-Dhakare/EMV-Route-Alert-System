#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include <Wire.h>
#include <Adafruit_QMC5883P.h>

#define CE_PIN   4
#define CSN_PIN  5

// --- 4 BUTTON PIN DEFINITIONS ---
#define B1         13  // Straight (val2 = heading + 0)
#define B2         14  // Right    (val2 = heading + 90)
#define B3         25  // U-Turn   (val2 = heading + 180)
#define B4         26  // Left     (val2 = heading + 270)
#define BUTTON_PIN 27  // Master Stop Button

// --- 4 LED PIN DEFINITIONS ---
#define LED1       12  // Turns on for B1 (Straight)
#define LED2       15  // Turns on for B2 (Right)
#define LED3       32  // Turns on for B3 (U-Turn)
#define LED4       33  // Turns on for B4 (Left)

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "12345"; 

Adafruit_QMC5883P qmc;

int16_t x_min = 32767;
int16_t x_max = -32768;
int16_t y_min = 32767;
int16_t y_max = -32768;

struct DataPacket {
  int val1; 
  int val2; 
};

DataPacket myData; 

unsigned long lastTransmitTime = 0;
const unsigned long transmitInterval = 1000; 

bool sendState = false;       
int activeSelection = 0;      

bool lastB1State = HIGH; 
bool lastB2State = HIGH;
bool lastB3State = HIGH;
bool lastB4State = HIGH;
bool lastPhysicalButtonState = HIGH;

// Helper function to turn on only the active LED and kill the rest
void updateIndicatorLEDs(int activeMode) {
  digitalWrite(LED1, (activeMode == 1) ? HIGH : LOW);
  digitalWrite(LED2, (activeMode == 2) ? HIGH : LOW);
  digitalWrite(LED3, (activeMode == 3) ? HIGH : LOW);
  digitalWrite(LED4, (activeMode == 4) ? HIGH : LOW);
}

void captureCompassSnapshot(int offsetAngle) {
  while (!qmc.isDataReady()) {
    delay(5); 
  }

  int16_t x, y, z;
  if (qmc.getRawMagnetic(&x, &y, &z)) {
    if (x < x_min) x_min = x;
    if (x > x_max) x_max = x;
    if (y < y_min) y_min = y;
    if (y > y_max) y_max = y;

    float x_offset = (x_max + x_min) / 2.0;
    float y_offset = (y_max + y_min) / 2.0;
    float x_cal = x - x_offset;
    float y_cal = y - y_offset;

    float heading = atan2(y_cal, x_cal) * 180.0 / PI;
    heading = 360 - heading;

    if (heading >= 360) heading -= 360;
    if (heading < 0)    heading += 360;

    int rawHeading = (int)heading;
    myData.val1 = rawHeading;
    myData.val2 = (rawHeading + offsetAngle) % 360;
    
    Serial.print(F("📸 [SNAPSHOT ENCODED] -> Mode: "));
    Serial.println(activeSelection);
    Serial.print(F("   Pure Raw (val1): ")); Serial.print(myData.val1); Serial.println(F("°"));
    Serial.print(F("   Turn Target (val2): ")); Serial.print(myData.val2); Serial.println(F("°"));
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  // Configure Buttons
  pinMode(B1,         INPUT_PULLUP); 
  pinMode(B2,         INPUT_PULLUP);
  pinMode(B3,         INPUT_PULLUP);
  pinMode(B4,         INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Configure LEDs as outputs
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  // Ensure all LEDs start in OFF state
  updateIndicatorLEDs(0);

  Wire.begin(21, 22);
  if (!qmc.begin()) {
    Serial.println(F("❌ QMC5883P NOT detected!"));
    while (1);
  }
  qmc.setMode(QMC5883P_MODE_CONTINUOUS);

  if (!radio.begin()) {
    Serial.println(F("❌ Transmitter NRF24L01 failed to start!"));
    while (1);
  }
  
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);  
  radio.stopListening();          

  myData.val1 = 0;
  myData.val2 = 0;
  
  Serial.println(F("==============================================="));
  Serial.println(F("LED Diagnostic Mode Initialized."));
  Serial.println(F("==============================================="));
}

void loop() {
  bool currentB1State = digitalRead(B1);
  bool currentB2State = digitalRead(B2);
  bool currentB3State = digitalRead(B3);
  bool currentB4State = digitalRead(B4);
  bool currentPhysicalButtonState = digitalRead(BUTTON_PIN);
  
  // --- BUTTON 1: STRAIGHT (PIN 13) ---
  if (currentB1State == LOW && lastB1State == HIGH) {
    sendState = true; 
    activeSelection = 1; 
    Serial.println(F("\n--> B1 Pressed: Straight Layout. LED 1 ON"));
    updateIndicatorLEDs(activeSelection); // Turn on LED 1, others off
    captureCompassSnapshot(0);
    delay(250); 
  }
  lastB1State = currentB1State;

  // --- BUTTON 2: RIGHT (PIN 14) ---
  if (currentB2State == LOW && lastB2State == HIGH) {
    sendState = true; 
    activeSelection = 2; 
    Serial.println(F("\n--> B2 Pressed: Right Turn Layout. LED 2 ON"));
    updateIndicatorLEDs(activeSelection); // Turn on LED 2, others off
    captureCompassSnapshot(90);
    delay(250); 
  }
  lastB2State = currentB2State;

  // --- BUTTON 3: U-TURN (PIN 25) ---
  if (currentB3State == LOW && lastB3State == HIGH) {
    sendState = true; 
    activeSelection = 3; 
    Serial.println(F("\n--> B3 Pressed: U-Turn Layout. LED 3 ON"));
    updateIndicatorLEDs(activeSelection); // Turn on LED 3, others off
    captureCompassSnapshot(180);
    delay(250); 
  }
  lastB3State = currentB3State;

  // --- BUTTON 4: LEFT (PIN 26) ---
  if (currentB4State == LOW && lastB4State == HIGH) {
    sendState = true; 
    activeSelection = 4; 
    Serial.println(F("\n--> B4 Pressed: Left Turn Layout. LED 4 ON"));
    updateIndicatorLEDs(activeSelection); // Turn on LED 4, others off
    captureCompassSnapshot(270);
    delay(250); 
  }
  lastB4State = currentB4State;

  // --- MASTER STOP BUTTON (PIN 27) ---
  if (currentPhysicalButtonState == LOW && lastPhysicalButtonState == HIGH) {
    if (sendState == true) {
      sendState = false; 
      activeSelection = 0;
      Serial.println(F("\n--> Pin 27 Pressed: Master Stop. All LEDs OFF"));
      updateIndicatorLEDs(activeSelection); // Turn all LEDs off
    }
    delay(250); 
  }
  lastPhysicalButtonState = currentPhysicalButtonState;

  // --- TRANSMISSION LOGIC TIMING CONTROL ---
  unsigned long currentTime = millis();
  if (currentTime - lastTransmitTime >= transmitInterval) {
    lastTransmitTime = currentTime;

    if (sendState == true) {
      radio.write(&myData, sizeof(myData));
    }
  }
}
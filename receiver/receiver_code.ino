#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RF24.h>

// --- Pin Definitions ---
#define NRF_CE_PIN  D3   // GPIO0  - NRF24L01 CE Pin
#define NRF_CSN_PIN D4   // GPIO2  - NRF24L01 CSN Pin
#define BUZZER_PIN  D0   // GPIO16 - Kept on D0 to leave D5 open for NRF SPI Clock (SCK)

// --- Initialize NRF24L01 ---
RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);
const byte pipeAddress[6] = "12345"; 

// Structure to hold the incoming data package
struct PacketData {
  int inAngle;
  int outAngle;
};
PacketData receivedData;

// --- Initialize 4 LCD instances ---
LiquidCrystal_I2C lcd1(0x27, 16, 2); 
LiquidCrystal_I2C lcd2(0x26, 16, 2); 
LiquidCrystal_I2C lcd3(0x25, 16, 2); 
LiquidCrystal_I2C lcd4(0x24, 16, 2); 

// --- Custom Character Pixel Maps ---
byte arrowUp[8]    = {0x04,0x0E,0x1F,0x04,0x04,0x04,0x04,0x00}; // ID: 0
byte arrowDown[8]  = {0x00,0x04,0x04,0x04,0x04,0x1F,0x0E,0x04}; // ID: 1
byte arrowLeft[8]  = {0x00,0x04,0x0C,0x1F,0x0C,0x04,0x00,0x00}; // ID: 2
byte arrowRight[8] = {0x00,0x04,0x06,0x1F,0x06,0x04,0x00,0x00}; // ID: 3

int ind = 0;      
int outd = 0;     
unsigned long previousMillis = 0;
unsigned long lastSignalMillis = 0; 

const long blinkInterval = 300; 
const unsigned long timeoutInterval = 2000; 
bool showArrowState = true;
bool signalActive = false;

void setup() {
  Serial.begin(115200); 
  Serial.println("\nESP8266 NRF Receiver Initializing...");

  // Configure Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH); 

  // Start I2C (SDA = D2, SCL = D1)
  Wire.begin(D2, D1); 

  // --- Setup All LCDs ---
  lcd1.init(); loadCustomCharacters(lcd1);
  lcd2.init(); loadCustomCharacters(lcd2);
  lcd3.init(); loadCustomCharacters(lcd3);
  lcd4.init(); loadCustomCharacters(lcd4);

  disableAllDisplays();

  // --- Setup NRF24L01 ---
  if (!radio.begin()) {
    Serial.println("NRF24L01 Hardware not responding!");
    lcd1.backlight();
    lcd1.setCursor(0,0); lcd1.print("NRF Error!    ");
    while (1); 
  }
  
  radio.openReadingPipe(1, pipeAddress);
  radio.setPALevel(RF24_PA_LOW); 
  radio.startListening();
  Serial.println("Listening for NRF signals...");
}

void loop() {
  if (radio.available()) {
    radio.read(&receivedData, sizeof(receivedData));
    
    if (receivedData.inAngle >= 0 && receivedData.inAngle <= 360 &&
        receivedData.outAngle >= 0 && receivedData.outAngle <= 360) {
      
      Serial.print("NRF Received Data -> Incoming Angle: ");
      Serial.print(receivedData.inAngle);
      Serial.print(" | Outgoing Angle: ");
      Serial.println(receivedData.outAngle);
      
      if (!signalActive) {
        enableAllBacklights();
        setupDisplayLabels();
      }

      ind = receivedData.inAngle;
      outd = receivedData.outAngle;
      lastSignalMillis = millis(); 
      signalActive = true;
    }
  }

  if (millis() - lastSignalMillis > timeoutInterval) {
    if (signalActive) { 
      signalActive = false;
      Serial.println("NRF Signal Lost / Timed Out.");
      digitalWrite(BUZZER_PIN, HIGH); 
      disableAllDisplays();           
    }
  }

  unsigned long currentMillis = millis();
  if (signalActive && (currentMillis - previousMillis >= blinkInterval)) {
    previousMillis = currentMillis;
    showArrowState = !showArrowState; 
    
    if (showArrowState) {
      digitalWrite(BUZZER_PIN, LOW);  
    } else {
      digitalWrite(BUZZER_PIN, HIGH); 
    }

    updateAllDisplays();
  }
}

void loadCustomCharacters(LiquidCrystal_I2C &lcd) {
  lcd.createChar(0, arrowUp);
  lcd.createChar(1, arrowDown);
  lcd.createChar(2, arrowLeft);
  lcd.createChar(3, arrowRight);
}

// Returns standard custom character ID based on raw input angle
int getArrowIDForAngle(int angle) {
  if (angle >= 315 || angle <= 45)       return 0; // Up
  else if (angle > 45 && angle <= 135)   return 3; // Right
  else if (angle > 135 && angle <= 225)  return 1; // Down
  else                                   return 2; // Left
}

// Rotates any standard arrow ID clockwise by degrees (90, 180, 270)
int getRotatedArrowID(int standardID, int rotationDegree) {
  int steps = rotationDegree / 90;
  int currentID = standardID;
  
  for (int s = 0; s < steps; s++) {
    switch (currentID) {
      case 0: currentID = 3; break; // Up -> Right
      case 3: currentID = 1; break; // Right -> Down
      case 1: currentID = 2; break; // Down -> Left
      case 2: currentID = 0; break; // Left -> Up
    }
  }
  return currentID;
}

// Calculates layout target column location based on the finalized character ID
int getColumnForArrowID(int arrowID) {
  switch(arrowID) {
    case 0: return 14; // Upward -> Column 14
    case 3: return 15; // Rightward -> Column 15
    case 1: return 15; // Downward -> Column 15
    case 2: return 13; // Leftward -> Column 13
    default: return 14;
  }
}

void enableAllBacklights() {
  lcd1.backlight();
  lcd2.backlight();
  lcd3.backlight();
  lcd4.backlight();
}

void disableAllDisplays() {
  lcd1.clear(); lcd1.noBacklight();
  lcd2.clear(); lcd2.noBacklight();
  lcd3.clear(); lcd3.noBacklight();
  lcd4.clear(); lcd4.noBacklight();
}

void setupDisplayLabels() {
  LiquidCrystal_I2C* displays[] = {&lcd1, &lcd2, &lcd3, &lcd4};
  for (int i = 0; i < 4; i++) {
    displays[i]->clear();
    displays[i]->setCursor(0, 0); displays[i]->print("EMV OUTGOING:"); 
    displays[i]->setCursor(0, 1); displays[i]->print("EMV INCOMING: "); 
  }
}

void updateAllDisplays() {
  LiquidCrystal_I2C* displays[] = {&lcd1, &lcd2, &lcd3, &lcd4};
  
  int standardOutID = getArrowIDForAngle(outd);
  int standardInID  = getArrowIDForAngle(ind);

  for (int i = 0; i < 4; i++) {
    // Clear display arrow tracks to prevent duplicate overlap ghosting
    displays[i]->setCursor(13, 0); displays[i]->print("   "); 
    displays[i]->setCursor(14, 1); displays[i]->print(" "); 
    
    if (showArrowState) {
      int localOutID = standardOutID;
      int localInID  = standardInID;
      
      // Apply the rotation offsets dynamically per screen configuration
      if (i == 1) { 
        // Display 2: +90 Degrees Rotation
        localOutID = getRotatedArrowID(standardOutID, 90);
        localInID  = getRotatedArrowID(standardInID, 90);
      } 
      else if (i == 2) { 
        // Display 3: +180 Degrees Rotation (Opposite of Display 1)
        localOutID = getRotatedArrowID(standardOutID, 180);
        localInID  = getRotatedArrowID(standardInID, 180);
      } 
      else if (i == 3) { 
        // Display 4: +270 Degrees Rotation (Opposite of Display 2)
        localOutID = getRotatedArrowID(standardOutID, 270);
        localInID  = getRotatedArrowID(standardInID, 270);
      }
      
      // Calculate layout positioning
      int outColumn = getColumnForArrowID(localOutID);

      // Print Outgoing Arrow on Row 0
      displays[i]->setCursor(outColumn, 0);
      displays[i]->write(localOutID);
      
      // Print Incoming Arrow on Row 1 (Fixed at layout column slot 14)
      displays[i]->setCursor(14, 1);
      displays[i]->write(localInID);
    }
  }
}
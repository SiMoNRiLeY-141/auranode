#include <Wire.h>
#include <AccelAndGyro.h>
#include <BarometricPressure.h>
#include <LightProximityAndGesture.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "BluetoothSerial.h" // --- NEW: ESP32 Native Bluetooth ---

// Check if Bluetooth is enabled in the ESP32 compiler
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define BUZZER_PIN 4
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

// --- HARDWARE OBJECTS ---
AccelAndGyro accelGyro;
BarometricPressure bmp;
LightProximityAndGesture apds;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
BluetoothSerial SerialBT; // --- NEW: Bluetooth Object ---

// --- INDUSTRIAL THRESHOLDS ---
float rhythmTolerance = 200.0;  
float tempThreshold = 33.0;      
int shadowThreshold = 10;         
int lightThreshold = 1000;       

// --- STATE VARIABLES ---
float baselineRhythm = 0.0;     
bool isFaultActive = false;
bool lastFaultState = false;   // --- NEW: Tracks when a fault STARTS or ENDS
bool isMuted = false;
bool shadowLatched = false;    
String faultReason = "";
unsigned long lastUpdate = 0;
unsigned long faultStartTime = 0; 

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  Wire.setClock(100000); 

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); 

  // Boot Bluetooth with the name the phone will see
  SerialBT.begin("AuraNode_EdgeAI"); 
  Serial.println("Bluetooth Started! Ready to pair.");

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(15, 20);
  display.println(F("AURANODE"));
  display.setTextSize(1);
  display.setCursor(10, 45);
  display.println(F("Booting Native Core"));
  display.display();
  delay(1000);

  if (!accelGyro.begin(false)) Serial.println("Vibration Sensor Failed!");
  if (!bmp.begin()) Serial.println("Thermal Sensor Failed!");
  if (!apds.begin()) Serial.println("Optical Sensor Failed!");
  
  apds.enableAmbientLightSensor();
  
  // --- DYNAMIC CALIBRATION PHASE (WITH ZERO-DROP FILTER) ---
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println(F("CALIBRATING..."));
  display.setCursor(0, 30);
  display.println(F("Sampling Data..."));
  display.display();

  delay(1000); // Let MEMS silicon stabilize

  double calibrationSum = 0; 
  int validSamples = 0;
  
  while (validSamples < 100) {
    float ax = accelGyro.getAccelX(false);
    float ay = accelGyro.getAccelY(false);
    float az = accelGyro.getAccelZ(false);
    float vector = sqrt(pow(ax, 2) + pow(ay, 2) + pow(az, 2));
    
    // Ignore zeros and impossible astronomical spikes
    if (vector > 500 && vector < 10000) { 
      calibrationSum += vector;
      validSamples++;
    }
    delay(10); 
  }
  
  baselineRhythm = calibrationSum / 100.0;
  
  display.fillRect(0, 30, 128, 20, BLACK);
  display.setCursor(0, 30);
  display.print(F("Base: "));
  display.println(baselineRhythm);
  display.display();
  
  delay(2000); 

  digitalWrite(BUZZER_PIN, HIGH); delay(100); digitalWrite(BUZZER_PIN, LOW); delay(100);
  digitalWrite(BUZZER_PIN, HIGH); delay(100); digitalWrite(BUZZER_PIN, LOW);
}

void loop() {
  // ---------------------------------------------------------
  // PHASE 1: HIGH-SPEED HMI CHECK (Touchless Shadow Toggle)
  // ---------------------------------------------------------
  uint16_t currentLight = apds.getAmbientLight(false);
  unsigned long duration = 0;
  if (isFaultActive) duration = millis() - faultStartTime;

  if (currentLight <= shadowThreshold) {
    if (!shadowLatched) { 
      if (isFaultActive) {
        if (duration >= 10000) {
          if (!isMuted) {
            isMuted = true; 
            SerialBT.println(">>> OPERATOR ACKNOWLEDGED FAULT (Muted)"); // Send to phone
          } else {
            isFaultActive = false;
            isMuted = false;
            faultReason = "";
          }
        }
      }
      shadowLatched = true; 
    }
  } else {
    shadowLatched = false; 
  }

  // ---------------------------------------------------------
  // PHASE 2: EDGE-AI DIAGNOSTICS (Runs every 150ms)
  // ---------------------------------------------------------
  if (millis() - lastUpdate > 150) {
    lastUpdate = millis();

    // 1. Fetch Real Data (Bypass Ghost Frames)
    float ax, ay, az, currentVibration = 0;
    bool isValidData = false;
    
    for (int i = 0; i < 5; i++) {
      ax = accelGyro.getAccelX(false); 
      ay = accelGyro.getAccelY(false);
      az = accelGyro.getAccelZ(false);
      currentVibration = sqrt(pow(ax, 2) + pow(ay, 2) + pow(az, 2));
      
      if (currentVibration > 500 && currentVibration < 10000) {
        isValidData = true;
        break; 
      }
      delay(10);
    }

    float currentTemp = bmp.getTempC(false);
    float rhythmDeviation = 0; 

    // 2. Data Validation Gate
    if (isValidData) {
      rhythmDeviation = abs(currentVibration - baselineRhythm);
      bool isRhythmBroken = rhythmDeviation > rhythmTolerance;
      bool isUnsafe = (isRhythmBroken || currentTemp > tempThreshold || currentLight > lightThreshold);

      if (isUnsafe && !isFaultActive) {
          isFaultActive = true;
          isMuted = false;
          faultStartTime = millis(); 
          if (isRhythmBroken) faultReason = "IMBALANCE";
          else if (currentTemp > tempThreshold) faultReason = "OVERHEAT!";
          else faultReason = "TAMPERED!";
      }
    }

    // ---------------------------------------------------------
    // PHASE 3: BLUETOOTH PUSH NOTIFICATIONS
    // ---------------------------------------------------------
    if (isFaultActive && !lastFaultState) {
      // The exact moment a fault triggers, blast a warning to the phone!
      SerialBT.println("\n==============================");
      SerialBT.println(" 🚨 CRITICAL MACHINE FAULT 🚨");
      SerialBT.println("==============================");
      SerialBT.println("CAUSE: " + faultReason);
      SerialBT.print("Deviation: "); SerialBT.println(rhythmDeviation);
      SerialBT.print("Temp: "); SerialBT.print(currentTemp); SerialBT.println(" C");
      SerialBT.print("Light: "); SerialBT.println(currentLight);
      SerialBT.println("Status: LOCKOUT INITIATED (10s)");
      SerialBT.println("==============================\n");
    } 
    else if (!isFaultActive && lastFaultState) {
      // The exact moment the tech resets the machine, send an all-clear!
      SerialBT.println("\n✅ SYSTEM RESET SUCCESSFUL");
      SerialBT.println("Machine returned to HEALTHY state.\n");
    }
    
    // Update the tracker memory for the next loop
    lastFaultState = isFaultActive;

    // 4. Hardware Actuation
    if (isFaultActive && !isMuted) digitalWrite(BUZZER_PIN, HIGH);
    else digitalWrite(BUZZER_PIN, LOW);

    // 5. OLED Dashboard Updates
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F(" AuraNode Edge-AI"));
    display.drawLine(0, 10, 128, 10, WHITE); 
    
    display.setCursor(0, 15);
    display.print(F("Dev: ")); display.print(rhythmDeviation, 1);
    display.setCursor(0, 25);
    display.print(F("Tmp: ")); display.print(currentTemp, 1);
    display.setCursor(70, 25);
    display.print(F("L: ")); display.print(currentLight);

    if (isFaultActive) {
      display.fillRect(0, 40, 128, 24, WHITE);
      display.setTextColor(BLACK, WHITE); 
      display.setTextSize(2);
      
      if (isMuted) {
        display.setCursor(15, 45);
        display.println(F(" ACK'D ")); 
      } else {
        if (duration < 10000) {
            display.setCursor(10, 45);
            display.print(F("LOCK:"));
            display.print((10000 - duration) / 1000);
            display.print(F("s"));
        } else {
            display.setCursor(10, 45);
            display.println(faultReason);  
        }
      }
      display.setTextColor(WHITE, BLACK); 
    } else {
      display.setTextSize(2);
      display.setCursor(25, 45);
      display.println(F("HEALTHY"));
    }
    display.display();
  }
}
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// ================= SERVO MOTORS =================
// Entry and exit gate control motors
Servo entryServo;
Servo exitServo;

#define ENTRY_SERVO 15
#define EXIT_SERVO 14

// ================= ULTRASONIC SENSORS =================
// Used to detect vehicle presence at entry and exit gates
#define TRIG_ENTRY 32
#define ECHO_ENTRY 33

#define TRIG_EXIT 25
#define ECHO_EXIT 26

// ================= OLED DISPLAY =================
// Displays system status, slot info, and billing details
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= RFID MODULE =================
// Used for identifying vehicles/users via RFID cards
#define SS_PIN 13
#define RST_PIN 27
MFRC522 rfid(SS_PIN, RST_PIN);

// ================= PARKING SLOT LED INDICATORS =================
// Each slot has Green (free) and Red (occupied) LED
#define S1_G 2
#define S1_R 12

#define S2_G 4
#define S2_R 5

#define S3_G 16
#define S3_R 17

// Stores entry time for billing calculation
unsigned long entryTime[3] = {0, 0, 0};

// Billing rate per minute
float ratePerMinute = 1.0;

// ================= ALLOWED RFID CARDS =================
// Each UID corresponds to a specific parking slot
String allowedUIDs[3] = {
  "01020304",
  "11223344",
  "55667788"
};

// Slot status: false = free, true = occupied
bool slotOccupied[3] = {false, false, false};

// ================= OLED STATUS DISPLAY =================
// Shows current status of all parking slots
void showStatus() {

  int occupied = 0;

  // Count occupied slots
  for (int i = 0; i < 3; i++) {
    if (slotOccupied[i]) occupied++;
  }

  int freeSlots = 3 - occupied;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println("SMART PARKING");

  // Show each slot status
  display.setCursor(0, 12);
  display.print("S1:");
  display.println(slotOccupied[0] ? " OCCUPIED" : " FREE");

  display.setCursor(0, 24);
  display.print("S2:");
  display.println(slotOccupied[1] ? " OCCUPIED" : " FREE");

  display.setCursor(0, 36);
  display.print("S3:");
  display.println(slotOccupied[2] ? " OCCUPIED" : " FREE");

  // Show total free slots or FULL status
  display.setCursor(0, 52);

  if (freeSlots == 0)
    display.println("PARKING FULL");
  else {
    display.print("FREE:");
    display.println(freeSlots);
  }

  display.display();
}

// ================= CHECK IF PARKING IS FULL =================
bool parkingFull() {
  for (int i = 0; i < 3; i++) {
    if (!slotOccupied[i]) return false;
  }
  return true;
}

// ================= ULTRASONIC DISTANCE FUNCTION =================
// Measures distance using HC-SR04 sensor
long getDistance(int trig, int echo) {

  digitalWrite(trig, LOW);
  delayMicroseconds(2);

  digitalWrite(trig, HIGH);
  delayMicroseconds(10);

  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH, 30000);

  if (duration == 0) return 999; // No object detected

  return duration * 0.034 / 2; // Convert to cm
}

// ================= OLED MESSAGE DISPLAY =================
// Shows temporary messages (alerts, instructions)
void showMessage(String line1, String line2 = "") {

  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 10);
  display.println(line1);

  display.setCursor(0, 30);
  display.println(line2);

  display.display();
}

// ================= LED STATUS UPDATE =================
// Green = free slot, Red = occupied slot
void updateLEDs() {

  digitalWrite(S1_G, !slotOccupied[0]);
  digitalWrite(S1_R, slotOccupied[0]);

  digitalWrite(S2_G, !slotOccupied[1]);
  digitalWrite(S2_R, slotOccupied[1]);

  digitalWrite(S3_G, !slotOccupied[2]);
  digitalWrite(S3_R, slotOccupied[2]);
}

// ================= GET RFID UID =================
// Reads and converts RFID card UID to string
String getUID() {

  String uid = "";

  for (byte i = 0; i < rfid.uid.size; i++) {

    if (rfid.uid.uidByte[i] < 0x10)
      uid += "0";

    uid += String(rfid.uid.uidByte[i], HEX);
  }

  uid.toUpperCase();
  return uid;
}

// ================= FIND SLOT FOR UID =================
// Maps RFID card to a specific parking slot
int getUIDIndex(String uid) {

  for (int i = 0; i < 3; i++) {
    if (uid == allowedUIDs[i])
      return i;
  }

  return -1; // Invalid card
}

// ================= SETUP FUNCTION =================
void setup() {

  Serial.begin(115200);

  // Servo initialization (gate control)
  entryServo.attach(ENTRY_SERVO);
  exitServo.attach(EXIT_SERVO);

  entryServo.write(0);
  exitServo.write(0);

  // Ultrasonic sensor pins
  pinMode(TRIG_ENTRY, OUTPUT);
  pinMode(ECHO_ENTRY, INPUT);

  pinMode(TRIG_EXIT, OUTPUT);
  pinMode(ECHO_EXIT, INPUT);

  // OLED initialization
  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // RFID initialization
  SPI.begin(18, 19, 23);
  rfid.PCD_Init();

  // LED pins setup
  pinMode(S1_G, OUTPUT);
  pinMode(S1_R, OUTPUT);

  pinMode(S2_G, OUTPUT);
  pinMode(S2_R, OUTPUT);

  pinMode(S3_G, OUTPUT);
  pinMode(S3_R, OUTPUT);

  updateLEDs();
  showStatus();

  Serial.println("System Ready");
}

// ================= MAIN LOOP =================
void loop() {

  // Continuously read distance at entry and exit gates
  long entryDist = getDistance(TRIG_ENTRY, ECHO_ENTRY);
  long exitDist  = getDistance(TRIG_EXIT, ECHO_EXIT);

  // ================= ENTRY GATE LOGIC =================
  if (entryDist < 10) {

    showMessage("ENTRY GATE", "Scan Card");

    // Check RFID card presence
    if (rfid.PICC_IsNewCardPresent() &&
        rfid.PICC_ReadCardSerial()) {

      String uid = getUID();
      int slot = getUIDIndex(uid);

      // Invalid card check
      if (slot == -1) {
        showMessage("INVALID CARD");
        delay(2000);
        showStatus();
        rfid.PICC_HaltA();
        return;
      }

      // Already parked check
      if (slotOccupied[slot]) {
        showMessage("ALREADY PARKED");
        delay(2000);
        showStatus();
        rfid.PICC_HaltA();
        return;
      }

      // Parking full check
      if (parkingFull()) {
        showMessage("PARKING FULL");
        delay(5000);
        showStatus();
        rfid.PICC_HaltA();
        return;
      }

      // Mark slot as occupied
      slotOccupied[slot] = true;
      entryTime[slot] = millis();

      updateLEDs();
      showStatus();

      delay(4000);

      // Open entry gate
      entryServo.write(90);

      // Display entry success
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("ENTRY SUCCESS");

      display.setCursor(0, 15);
      display.print("UID:");
      display.println(uid);

      display.setCursor(0, 35);
      display.print("GO TO SLOT ");
      display.println(slot + 1);

      display.display();

      delay(4000);
      entryServo.write(0);

      showStatus();
      rfid.PICC_HaltA();
    }
  }

  // ================= EXIT GATE LOGIC =================
  if (exitDist < 10) {

    showMessage("EXIT GATE", "Scan Card");

    if (rfid.PICC_IsNewCardPresent() &&
        rfid.PICC_ReadCardSerial()) {

      String uid = getUID();
      int slot = getUIDIndex(uid);

      // Invalid card
      if (slot == -1) {
        showMessage("INVALID CARD");
        delay(2000);
        showStatus();
        rfid.PICC_HaltA();
        return;
      }

      // Not parked check
      if (!slotOccupied[slot]) {
        showMessage("NOT PARKED");
        delay(2000);
        showStatus();
        rfid.PICC_HaltA();
        return;
      }

      // Calculate parking time
      unsigned long parkedTime = millis() - entryTime[slot];
      float minutes = parkedTime / 60000.0;

      // Calculate bill
      float bill = minutes * ratePerMinute;

      // Free slot
      slotOccupied[slot] = false;
      entryTime[slot] = 0;

      updateLEDs();

      // Open exit gate
      exitServo.write(90);

      // Display billing info
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("EXIT SUCCESS");

      display.setCursor(0, 15);
      display.print("UID:");
      display.println(uid);

      display.setCursor(0, 30);
      display.print("TIME:");
      display.print(minutes, 1);
      display.println(" min");

      display.setCursor(0, 50);
      display.print("BILL: ");
      display.println(bill, 2);

      display.display();

      delay(5000);
      exitServo.write(0);

      showStatus();
      rfid.PICC_HaltA();
    }
  }
}
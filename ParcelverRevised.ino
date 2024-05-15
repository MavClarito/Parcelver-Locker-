#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

int red = 4;
int green = 5;
const int buttonPin = A0;

Servo S1, S2, S3;
bool servo1Unlocked = false;
bool servo2Unlocked = false;
bool servo3Unlocked = false;


const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = { 13, 12, 11, 10 };
byte colPins[COLS] = { 9, 8, 7, 6 };
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2);

SoftwareSerial sim800l(3, 2);
int otp;
String otpstring = "";

// Define passcodes for each servo (owner-specific)
String servo1Passcode = "AAAA";
String servo2Passcode = "BBBB";
String servo3Passcode = "CCCC";

void setup() {
  S1.attach(A1);
  S2.attach(A2);
  S3.attach(A3);

  pinMode(buttonPin, INPUT); // Set button pin as input
  sim800l.begin(4800);
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  Serial.print("Welcome to Parcelver \n");
  sim800l.println("AT");
  updateSerial();
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  delay(500);
  sim800l.println("AT+CSQ");
  updateSerial();
  delay(1000);

  S1.write(0); // Lock S1
  S2.write(0); // Lock S2
  S3.write(0); // Lock S3
}

void updateSerial() {
  delay(500);
  while (Serial.available()) {
    sim800l.write(Serial.read());
  }
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
}

void loop() {
  lcd.setCursor(0, 0);
  lcd.print("  Please wait");
  lcd.setCursor(0, 1);
  lcd.print("for the OTP...");

  // Check if the button is pressed
  if (digitalRead(buttonPin) == HIGH) {
    delay(50); 
    if (digitalRead(buttonPin) == HIGH) { // Check again to confirm button press

      // Generate a new OTP
      otp = random(1000, 9999);
      otpstring = String(otp);
      Serial.println(otpstring);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  OTP sent to ");
      lcd.setCursor(0, 1);
      lcd.print("  your Mobile ");
      Serial.print("OTP is ");
      delay(100);
      Serial.println(otpstring);
      delay(100);
      SendSMS();
      delay(1000); // Wait a moment to ensure SMS is sent
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter OTP: ");
      getotp();
    }
  }
}

void getotp() {
  String enteredOTP = "";
  int enteredLength = 0;
  while (enteredLength < 4) {
    char customKey = customKeypad.getKey();
    if (customKey) {
      lcd.setCursor(enteredLength, 1);
      enteredOTP += customKey;
      lcd.print(customKey);
      enteredLength++;
    }
  }
  Serial.print("Entered OTP is: ");
  Serial.println(enteredOTP);

  // Initial Lock State
  S1.write(0); // Lock S1
  S2.write(0); // Lock S2
  S3.write(0); // Lock S3

  if (otpstring == enteredOTP) {
    lcd.clear();
    lcd.print("Access Granted");
    lcd.setCursor(0, 1);
    lcd.print("Door Opening");
    unlockNextServo();
  } else if (checkOwnerPasscode(enteredOTP)) {
    // Owner-specific passcode entered
    lcd.clear();
    lcd.print("Owner Access");
    lcd.setCursor(0, 1);
    lcd.print("Granted");
    delay(2000); 
  } else {
    // Incorrect OTP or passcode entered, return all servos to locked position
    S1.write(0); // Lock S1
    S2.write(0); // Lock S2
    S3.write(0); // Lock S3

    delay(300);
    lcd.clear();
    lcd.print("Access Failed");
    lcd.setCursor(0, 1);
    lcd.print("Please try Again!");

    digitalWrite(red, LOW);
    delay(3000);
    digitalWrite(red, HIGH);
  }
}

void unlockNextServo() {
  if (!servo1Unlocked) {
    unlockServo(S1, servo1Unlocked, "Servo 1");
    servo1Unlocked = true;
  }
  else if (!servo2Unlocked) {
    unlockServo(S2, servo2Unlocked, "Servo 2");
    servo2Unlocked = true;
  }
  else if (!servo3Unlocked) {
    unlockServo(S3, servo3Unlocked, "Servo 3");
    servo3Unlocked = true;
  }
  else {
    lcd.clear();
    lcd.print("  All drawers");
    lcd.setCursor(0, 1);
    lcd.print("  occupied");
    digitalWrite(red, HIGH);
    delay(3000);
  }
}

void unlockServo(Servo& servo, bool& servoUnlocked, String servoName) {
  lcd.clear();
  lcd.print("  Unlocking ");
  lcd.setCursor(0, 1);
  lcd.print(servoName);
  servo.write(90);
  digitalWrite(green, HIGH);
  for (int i = 9; i >= 0; i--) {
    lcd.setCursor(12, 1);
    lcd.print(i);
    delay(1000);
  }
  servo.write(0);
  digitalWrite(green, LOW);
  digitalWrite(red, HIGH);
  delay(1000);
  digitalWrite(red, LOW);
}

bool checkOwnerPasscode(String enteredPasscode) {
  if (enteredPasscode == servo1Passcode) {
    unlockServo(S1, servo1Unlocked, "Servo 1");
    digitalWrite(red, LOW);
    delay(500);
    digitalWrite(red, HIGH);
    delay(1000);
    digitalWrite(red, LOW);
    servo1Unlocked = true;
    return true;
  } else if (enteredPasscode == servo2Passcode) {
    unlockServo(S2, servo2Unlocked, "Servo 2");
    digitalWrite(red, LOW);
    delay(500);
    digitalWrite(red, HIGH);
    delay(1000);
    digitalWrite(red, LOW);
    servo2Unlocked = true;
    return true;
  } else if (enteredPasscode == servo3Passcode) {
    unlockServo(S3, servo3Unlocked, "Servo 3");
    servo3Unlocked = true;
    digitalWrite(red, LOW);
    delay(500);
    digitalWrite(red, HIGH);
    delay(1000);
    digitalWrite(red, LOW);
    return true;
  }
  return false;
}

void SendSMS() {
  Serial.println("Sending SMS...");
  sim800l.print("AT+CMGF=1\r");
  delay(100);
  sim800l.print("AT+CSMP=17,167,0,0\r");
  delay(500);
  sim800l.print("AT+CMGS=\"+639292696071\"\r");
  delay(500);
  String message = "The OTP of Parcelver is " + otpstring + ". Just type this into the keypad and a drawer will unlock. Thank you so much for your service! >w< ";
  sim800l.print(message);
  delay(500);
  sim800l.print((char)26);
  delay(500);
  sim800l.println();
  Serial.println("Text Sent.");
  delay(500);
}

void makeCall(String phoneNumber) {
  sim800l.println("ATD" + phoneNumber + ";");
  delay(1000);
  Serial.println("Calling " + phoneNumber);
}

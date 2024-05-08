#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

int red = 4;
int green = 5;

#define relay 2

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
int irsensor = A0;
int otp;
String otpstring = "";
int i = 0;

void setup() {
  S1.attach(A1);
  S2.attach(A2);
  S3.attach(A3);

  pinMode(irsensor, INPUT_PULLUP);
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
  lcd.print(" for the OTP...");

  if (digitalRead(irsensor) == LOW){
    while (digitalRead(irsensor) == LOW){
    }
    // Generate a new OTP
    otp = random(1000, 9999);
    otpstring = String(otp);
    Serial.println(otpstring);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  OTP Sent to ");
    lcd.setCursor(0, 1);
    lcd.print("  Your Mobile ");
    Serial.print("OTP is ");
    delay(100);
    Serial.println(otpstring);
    delay(100);
    SendSMS();
    //makeCall("+639292696071");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter OTP: ");
    getotp();
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
  S1.write(90); // Lock S1
  S2.write(90); // Lock S2
  S3.write(90); // Lock S3

  if (otpstring == enteredOTP) {
    lcd.clear();
    lcd.print("Access Granted");
    lcd.setCursor(0, 1);
    lcd.print("Door Opening");

    int randomServo = random(0, 4); 

    if (randomServo == 1 && !servo1Unlocked) {
      lcd.clear();
      lcd.print("  Unlocking ");
      lcd.setCursor(0, 1);
      lcd.print("  Servo 1");
      S1.write(180); 
      servo1Unlocked = true; 
      digitalWrite(green, HIGH); 
      for (int i = 9; i >= 0; i--) { 
        lcd.setCursor(12, 1);
        lcd.print(i); 
        delay(1000); 
      }
      S1.write(90); 
      digitalWrite(green, LOW);
      digitalWrite(red, HIGH);
    } 
    
    else if (randomServo == 2 && !servo2Unlocked) {
      lcd.clear();
      lcd.print("  Unlocking ");
      lcd.setCursor(0, 1);
      lcd.print("  Servo 2");
      S2.write(0); 
      servo2Unlocked = true; 
      digitalWrite(green, HIGH); 
      for (int i = 9; i >= 0; i--) { 
        lcd.setCursor(12, 1);
        lcd.print(i); 
        delay(1000); 
      }
      S2.write(90); 
      digitalWrite(green, LOW); 
      digitalWrite(red, HIGH);
    } 
    
    else if (randomServo == 3 && !servo3Unlocked) {
      lcd.clear();
      lcd.print("  Unlocking ");
      lcd.setCursor(0, 1);
      lcd.print("  Servo 3");
      S3.write(0); 
      servo3Unlocked = true; 
      digitalWrite(green, HIGH);
      for (int i = 9; i >= 0; i--) { 
        lcd.setCursor(12, 1);
        lcd.print(i); 
        delay(1000); 
      }
      S3.write(90); 
      digitalWrite(green, LOW);
      digitalWrite(red, HIGH);
    }

    else {
      lcd.clear();
      lcd.print("  Drawer is");
      lcd.setCursor(0, 1);
      lcd.print("  occupied");
      digitalWrite(red, HIGH);
      delay(3000);
    }
  } 
  
  else {
    // Incorrect OTP entered, return all servos to locked position
    S1.write(90); // Lock S1
    S2.write(90); // Lock S2
    S3.write(90); // Lock S3

    delay(300);
    lcd.clear();
    lcd.print("Access Failed");
    lcd.setCursor(0, 1);
    lcd.print("Try Again !!!");

    digitalWrite(red, LOW); // Turn on red LED
    delay(3000);
    digitalWrite(red, HIGH); // Turn off red LED
  }
}

void SendSMS() {
  Serial.println("Sending SMS...");
  sim800l.print("AT+CMGF=1\r");
  delay(100);
  sim800l.print("AT+CSMP=17,167,0,0\r");
  delay(500);
  sim800l.print("AT+CMGS=\"+639292696071\"\r");
  delay(500);
  String message = "Your OTP is " + otpstring + " Just Type OTP And Unlock The Door";
  sim800l.print(message);
  delay(500);
  sim800l.print((char)26);
  delay(500);
  sim800l.println();
  Serial.println("Text Sent.");
  delay(500);
}

void makeCall(String phoneNumber) {
  sim800l.println("ATD" + phoneNumber + ";");  // Command to dial a number
  delay(1000);
  Serial.println("Calling " + phoneNumber);
}


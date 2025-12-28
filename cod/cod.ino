#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>


LiquidCrystal_I2C lcd(0x27, 16, 2);


#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);


Servo servo;
const int servoPin = A1;


const int lockPin = A2;


String pin_corect = "1234";
String pin_introdus = "";


byte uid_corect[] = {0xA6, 0x07, 0xA7, 0x00};


const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, A0};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

/* FUNCTII */

void deschideSeif() {
  lcd.clear();
  lcd.print("Acces permis");

  digitalWrite(lockPin, HIGH); // activeaza incuietoarea
  servo.write(90);             // deschide usa

  delay(10000);                // 10 secunde deschis

  servo.write(0);              // inchide usa
  digitalWrite(lockPin, LOW);  // blocheaza

  lcd.clear();
}

bool verificaCardUID() {
  if (!mfrc522.PICC_IsNewCardPresent()) return false;
  if (!mfrc522.PICC_ReadCardSerial()) return false;

  if (mfrc522.uid.size != sizeof(uid_corect)) return false;

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] != uid_corect[i]) {
      return false;
    }
  }

  mfrc522.PICC_HaltA();
  return true;
}

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.print("Introdu PIN-ul");

  SPI.begin();
  mfrc522.PCD_Init();

  servo.attach(servoPin);
  servo.write(0);

  pinMode(lockPin, OUTPUT);
  digitalWrite(lockPin, LOW);
}

void loop() {

  /*KEYPAD*/
  char tasta = keypad.getKey();

  if (tasta) {
    if (tasta == '#') {
      if (pin_introdus == pin_corect) {
        deschideSeif();
      } else {
        lcd.clear();
        lcd.print("PIN gresit!");
        delay(2000);
        lcd.clear();
        lcd.print("Introdu PIN-ul");
      }
      pin_introdus = "";
    }
    else if (tasta == '*') {
      pin_introdus = "";
      lcd.clear();
      lcd.print("Reset PIN");
      delay(1000);
      lcd.clear();
      lcd.print("Introdu PIN-ul");
    }
    else {
      pin_introdus += tasta;
      lcd.setCursor(0, 1);
      lcd.print(pin_introdus);
    }
  }

  /*RFID*/
  if (verificaCardUID()) {
    lcd.clear();
    lcd.print("Card valid");
    delay(1000);
    deschideSeif();
    lcd.print("Introdu PIN-ul");
  }
}

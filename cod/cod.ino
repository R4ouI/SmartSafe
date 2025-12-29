#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <SPI.h>
#include <MFRC522.h>


/*  LCD  */
LiquidCrystal_I2C lcd(0x27, 16, 2);

/*  RFID  */
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);



bool seifDeschis = false;

/*  INCUIETOARE  */
const int lockPin = A2;

/*  PIN  */
String pin_corect = "1234";
String pin_introdus = "";

/* UID CORECT */
byte uid_corect[] = {0xA6, 0x07, 0xA7, 0x00};

/* ================= KEYPAD ================= */
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

/*  FUNCTII  */

void afisareInitiala() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Introdu PIN-ul");
  lcd.setCursor(0,1);
  lcd.print("sau card");
}
void resetLCD() {
  lcd.clear();
  delay(50);
  lcd.init();
  lcd.backlight();
}

void resetRFID() {
  mfrc522.PCD_StopCrypto1();
  mfrc522.PICC_HaltA();
  delay(50);
  mfrc522.PCD_Init();   
}

void deschideSeif() {
  seifDeschis = true;

  lcd.clear();
  lcd.print("Acces permis");

  digitalWrite(lockPin, HIGH);


  delay(5000);


  digitalWrite(lockPin, LOW);

  lcd.clear();
  lcd.print("Scoate cardul");

  // Asteapta sa fie scos cardul
  while (mfrc522.PICC_IsNewCardPresent()) {
    delay(100);
  }

  resetRFID();
  delay(500);

  seifDeschis = false;
  afisareInitiala();
}


bool verificaCardUID() {
  if (!mfrc522.PICC_IsNewCardPresent()) return false;
  if (!mfrc522.PICC_ReadCardSerial()) return false;

  if (mfrc522.uid.size != sizeof(uid_corect)) return false;

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] != uid_corect[i]) {
      resetRFID();
      return false;
    }
  }

  resetRFID();
  return true;
}

/*  SETUP  */
void setup() {
  Serial.begin(9600);

  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH);

  lcd.init();
  lcd.backlight();
  afisareInitiala();

  SPI.begin();
  mfrc522.PCD_Init();


  pinMode(lockPin, OUTPUT);
  digitalWrite(lockPin, LOW);
}

void loop() {

  /*  KEYPAD  */
  char tasta = keypad.getKey();

  if (tasta) {
    if (tasta == '#') {
      if (pin_introdus == pin_corect) {
        pin_introdus = "";
        deschideSeif();
      } else {
        lcd.clear();
        lcd.print("PIN gresit!");
        delay(1500);
        pin_introdus = "";
        afisareInitiala();
      }
    }
    else if (tasta == '*') {
      pin_introdus = "";
      afisareInitiala();
    }
    else {
      pin_introdus += tasta;
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      for (byte i = 0; i < pin_introdus.length(); i++) {
        lcd.print('*');
      }
    }
  }

  if (!seifDeschis && verificaCardUID()) {
  deschideSeif();
}

}

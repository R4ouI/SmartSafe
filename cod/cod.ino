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

/*  INCUIETOARE  */
const int lockPin = A2;

/*  PIN  */
char pin_corect[5] = "1234";
char pin_introdus[5];
byte pinIndex = 0;

/*  UID CARD  */
byte uid_corect[] = {0xA6, 0x07, 0xA7, 0x00};

/*  KEYPAD  */
const byte ROWS = 4, COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {2,3,4,5};
byte colPins[COLS] = {6,7,8,A0};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

/*  STARE  */
bool seifDeschis = false;
bool modSetarePIN = false;
unsigned long timpDeschidere = 0;

/*  FUNCTII  */
void afisareInitiala() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Introdu PIN-ul");
  lcd.setCursor(0,1);
  lcd.print("sau card");
}

void afisareSetarePIN() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Seteaza PIN");
  lcd.setCursor(0,1);
}

void resetRFID() {
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  delay(50);
  mfrc522.PCD_Init();
}

bool verificaCard() {
  if (!mfrc522.PICC_IsNewCardPresent()) return false;
  if (!mfrc522.PICC_ReadCardSerial()) return false;

  if (mfrc522.uid.size != sizeof(uid_corect)) {
    resetRFID();
    return false;
  }

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] != uid_corect[i]) {
      resetRFID();
      return false;
    }
  }

  resetRFID();
  return true;
}

void deschideSeif() {
  seifDeschis = true;
  timpDeschidere = millis();

  lcd.clear();
  lcd.print("Acces permis");
  digitalWrite(lockPin, HIGH);
}

void inchideSeif() {
  digitalWrite(lockPin, LOW);

  seifDeschis = false;
  pinIndex = 0;
  pin_introdus[0] = '\0';

  afisareInitiala();
}

/*  SETUP  */
void setup() {
  pinMode(SS_PIN, OUTPUT);          // CRITIC pentru SPI
  digitalWrite(SS_PIN, HIGH);

  pinMode(lockPin, OUTPUT);
  digitalWrite(lockPin, LOW);

  lcd.init();
  lcd.backlight();
  afisareInitiala();

  SPI.begin();
  mfrc522.PCD_Init();
}

void loop() {

  /* AUTO INCHIDERE DUPA 5s */
  if (seifDeschis && millis() - timpDeschidere >= 5000) {
    inchideSeif();
  }

  /*  RFID (mereu activ)  */
  if (!seifDeschis && !modSetarePIN) {
    if (verificaCard()) {
      deschideSeif();
      return;
    }
  }

  char tasta = keypad.getKey();
  if (!tasta || seifDeschis) return;

  /*  MOD SETARE PIN*/
  if (modSetarePIN) {
    if (tasta >= '0' && tasta <= '9' && pinIndex < 4) {
      pin_introdus[pinIndex++] = tasta;
      lcd.print('*');
    }
    else if (tasta == 'B' && pinIndex == 4) {
      pin_introdus[4] = '\0';
      strcpy(pin_corect, pin_introdus);

      lcd.clear();
      lcd.print("PIN salvat!");
      delay(1500);

      pinIndex = 0;
      modSetarePIN = false;
      afisareInitiala();
    }
    return;
  }


  if (tasta == 'A') {
    modSetarePIN = true;
    pinIndex = 0;
    afisareSetarePIN();
    return;
  }

  if (tasta == '#') {
    pin_introdus[pinIndex] = '\0';
    if (strcmp(pin_introdus, pin_corect) == 0) {
      deschideSeif();
    } else {
      lcd.clear();
      lcd.print("PIN gresit");
      delay(1500);
      afisareInitiala();
    }
    pinIndex = 0;
  }
  else if (tasta == '*') {
    pinIndex = 0;
    afisareInitiala();
  }
  else if (tasta >= '0' && tasta <= '9' && pinIndex < 4) {
    pin_introdus[pinIndex++] = tasta;
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,1);
    for (byte i = 0; i < pinIndex; i++) lcd.print('*');
  }
}

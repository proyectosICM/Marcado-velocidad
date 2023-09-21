#include <SoftwareSerial.h>
#include "src/NOKIA5110_TEXT/NOKIA5110_TEXT.h"
#include "src/FreqMeasure/FreqMeasure.h"
#include <EEPROM.h>

#define RST 2  // Reset pin
#define CE 3   // Chip enable
#define DC 4   // data or command
#define DIN 5  // Serial Data input
#define CLK 6  // Serial clock

// Create an LCD object
NOKIA5110_TEXT mylcd(RST, CE, DC, DIN, CLK);

#define inverse false
#define contrast 0xBF  // default is 0xBF set in LCDinit, Try 0xB1 - 0xBF if your display is too dark/dim
#define bias 0x12      // LCD bias mode 1:48: Try 0x12, 0x13 or 0x14

SoftwareSerial MP3(10, 11);  // RX, TX

#define PROG A1
#define UP A0
#define DOWN A2

#define OUT 9

float frequency;
float tmp;
uint32_t updateTime;
int32_t lastPlay = -10000;
uint32_t debounce;

uint8_t limit = 30;
uint8_t speed = 0;

uint8_t ppr = 4;
uint8_t diameter = 100;
uint8_t digits[4];
uint16_t btnCounter;

uint32_t zeroLimit = 0;

uint32_t outTime = 0;



void decompose(uint8_t number) {
  uint8_t j;
  for (j = 0; j < 4; j++) {
    digits[j] = ' ';
  }
  for (j = 0; j < 4; j++) {
    digits[j] = (number % 10) + '0';
    if (number / 10 == 0)
      return;
    else
      number /= 10;
  }
}

void programLimit() {

  mylcd.LCDClear();  // clear whole screen

  mylcd.LCDFont(LCDFont_Default);  // Font 1 default
  mylcd.LCDgotoXY(0, 1);
  mylcd.LCDString("   LIMITE   ");
  mylcd.LCDgotoXY(0, 4);
  mylcd.LCDString("      KM/H  ");


  mylcd.LCDgotoXY(14, 4);
  decompose(limit);
  mylcd.LCDCharacter(digits[2]);
  mylcd.LCDCharacter(digits[1]);
  mylcd.LCDCharacter(digits[0]);

  while (digitalRead(PROG) == LOW) {
    btnCounter++;
    delay(10);
    if (btnCounter >= 300) {
      btnCounter = 0;
      return;
    }
  }
  btnCounter = 0;


  while (1) {


    mylcd.LCDgotoXY(14, 4);
    decompose(limit);
    mylcd.LCDCharacter(digits[2]);
    mylcd.LCDCharacter(digits[1]);
    mylcd.LCDCharacter(digits[0]);

    if (digitalRead(PROG) == LOW) {
      while (digitalRead(PROG) == LOW) {
        btnCounter++;
        delay(10);
        if (btnCounter >= 300) {

          btnCounter = 0;
          return;
        }
      }
    }

    if (millis() > (debounce + 500)) {
      if (btnCounter > 0) {
        debounce = millis();
        if (limit < 35) //Vel max
          limit++;
        else
          limit = 5; //Min
      }
    }


    btnCounter = 0;
  }
}

void programPPR() {

  mylcd.LCDClear();  // clear whole screen

  mylcd.LCDFont(LCDFont_Default);  // Font 1 default
  mylcd.LCDgotoXY(0, 0);
  mylcd.LCDString(" PULSOS POR ");
  mylcd.LCDgotoXY(0, 1);
  mylcd.LCDString("   VUELTA   ");
  mylcd.LCDgotoXY(0, 4);
  mylcd.LCDString("       ppr  ");

  mylcd.LCDgotoXY(14, 4);
  decompose(ppr);
  mylcd.LCDCharacter(digits[2]);
  mylcd.LCDCharacter(digits[1]);
  mylcd.LCDCharacter(digits[0]);


  while (digitalRead(PROG) == LOW) {
    btnCounter++;
    delay(10);
    if (btnCounter >= 300) {
      btnCounter = 0;
      return;
    }
  }
  btnCounter = 0;

  while (1) {


    mylcd.LCDgotoXY(14, 4);
    decompose(ppr);
    mylcd.LCDCharacter(digits[2]);
    mylcd.LCDCharacter(digits[1]);
    mylcd.LCDCharacter(digits[0]);


    if (digitalRead(PROG) == LOW) {
      while (digitalRead(PROG) == LOW) {
        btnCounter++;
        delay(10);
        if (btnCounter >= 300) {

          btnCounter = 0;
          return;
        }
      }
    }

    if (millis() > (debounce + 500)) {
      if (btnCounter > 0) {
        debounce = millis();
        if (ppr < 60)
          ppr++;
        else
          ppr = 1;
      }
    }
    btnCounter = 0;
  }
}

void programDiameter() {

  mylcd.LCDClear();  // clear whole screen

  mylcd.LCDFont(LCDFont_Default);  // Font 1 default
  mylcd.LCDgotoXY(0, 0);
  mylcd.LCDString("  DIAMETRO  ");
  mylcd.LCDgotoXY(0, 4);
  mylcd.LCDString("        cm  ");

  mylcd.LCDgotoXY(14, 4);
  decompose(diameter);
  mylcd.LCDCharacter(digits[2]);
  mylcd.LCDCharacter(digits[1]);
  mylcd.LCDCharacter(digits[0]);

  while (digitalRead(PROG) == LOW) {
    btnCounter++;
    delay(10);
    if (btnCounter >= 300) {
      btnCounter = 0;
      return;
    }
  }
  btnCounter = 0;

  while (1) {


    mylcd.LCDgotoXY(14, 4);
    decompose(diameter);
    mylcd.LCDCharacter(digits[2]);
    mylcd.LCDCharacter(digits[1]);
    mylcd.LCDCharacter(digits[0]);

    if (digitalRead(PROG) == LOW) {
      while (digitalRead(PROG) == LOW) {
        btnCounter++;
        delay(10);
        if (btnCounter >= 300) {
          btnCounter = 0;
          return;
        }
      }
    }

    if (millis() > (debounce + 500)) {
      if (btnCounter > 0) {
        debounce = millis();
        if (diameter < 70)
          diameter++;
        else
          diameter = 15;
      }
    }
    btnCounter = 0;
  }
}

void setup() {

  pinMode(OUT, OUTPUT);
  pinMode(PROG, INPUT_PULLUP);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);

  digitalWrite(OUT, LOW);

  Serial.begin(115200);
  MP3.begin(9600);

  mylcd.LCDInit(inverse, contrast, bias);  // init the lCD
  mylcd.LCDClear();                        // clear whole screen
  mylcd.LCDFont(LCDFont_Default);          // Font 1 default
  mylcd.LCDgotoXY(0, 2);
  mylcd.LCDString("VELOCIMETRO");
  mylcd.LCDgotoXY(0, 3);
  mylcd.LCDString(" INICIANDO ");


  delay(500);
  MP3.print("AF:28");
  delay(500);

  mylcd.LCDClear();                // clear whole screen
  mylcd.LCDFont(LCDFont_Default);  // Font 1 default
  mylcd.LCDgotoXY(0, 5);
  mylcd.LCDString("    KM/H");

  EEPROM.get(0, limit);
  EEPROM.get(1, ppr);
  EEPROM.get(2, diameter);

  if ((limit < 5) || (limit > 35) || (ppr == 0) || (ppr > 60) || (diameter < 15) || (diameter > 70)) {
    limit = 5;
    ppr = 1;
    diameter = 40;
/*
    EEPROM.put(0, limit);
    EEPROM.put(1, ppr);
    EEPROM.put(2, diameter);*/
  }

  FreqMeasure.begin();
  btnCounter = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  if (FreqMeasure.available()) {
    frequency = FreqMeasure.countToFrequency(FreqMeasure.read());
    tmp = frequency / ppr;
    tmp *= (float)diameter / 100 * PI * 0.2777;
    speed = tmp;
    zeroLimit = millis();
  }

  if (millis() > (zeroLimit + 1000)) {
    speed = 0;
  }

  if (millis() > (updateTime + 500)) {
    updateTime = millis();
    decompose(speed);
    mylcd.LCDFont(LCDFont_Mega);  // font 9, this font takes 4 blocks

    // mylcd.LCDClear();  // clear whole screen
    mylcd.LCDgotoXY(4, 0);
    if (digits[2] != ' ')
      mylcd.LCDCharacter(digits[2]);
    else
      mylcd.LCDCharacter(';');

    mylcd.LCDgotoXY(24, 0);
    if (digits[1] != ' ')
      mylcd.LCDCharacter(digits[1]);
    else
      mylcd.LCDCharacter(';');

    mylcd.LCDgotoXY(44, 0);
    mylcd.LCDCharacter(digits[0]);
    mylcd.LCDFont(LCDFont_Default);  // Font 1 default
    mylcd.LCDgotoXY(0, 5);
    mylcd.LCDString("    KM/H");
  }

  if (speed > limit) {
      outTime = millis() + 3000;
    if (millis() > (lastPlay + 30000)) {
      lastPlay = millis();
      // enviamos advertencia
      digitalWrite(OUT, HIGH);
      Serial.println("ENCENDIDO");
      MP3.print("A7:00001");
    }
  }

  if (millis() > outTime)
    digitalWrite(OUT, LOW);
  else
    digitalWrite(OUT, HIGH);

  if (digitalRead(PROG) == LOW) {

    while (digitalRead(PROG) == LOW) {
      btnCounter++;
      if (FreqMeasure.available())
        FreqMeasure.read();
      delay(10);
      if (btnCounter > 300)
        break;
    }

    if (btnCounter > 300) {
      btnCounter = 0;
      debounce = millis();
      FreqMeasure.end();
      programLimit();
      programPPR();
      programDiameter();
      EEPROM.put(0, limit);
      EEPROM.put(1, ppr);
      EEPROM.put(2, diameter);
      debounce = millis();
      mylcd.LCDClear();

      FreqMeasure.begin();
    }
    btnCounter = 0;
  }
}

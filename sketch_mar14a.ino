#include <TM1637Display.h>
#include <EEPROM.h>

const int CLK = D6; //Set the CLK pin connection to the display
const int DIO = D5; //Set the DIO pin connection to the display
const int SWITCH = D1;
const int USER_BUTTON = 16;

// Zapni nbebo vypni tecku
#define DOT_OFF 0b00000000
#define DOT_ON  0b01000000

#define SECONDS_ADDRESS  0
#define MINUTES_ADDRESS  1

TM1637Display display(CLK, DIO);

// Aktualni mninuty a sekundy
byte mm = 0;
byte ss = 0;

unsigned long nextSecondChange = 0; // Cas pristi zmeny sekundy
unsigned long nextDotChange = 0; // Cas pristiho zapnutí dvojtecky

int switchValue = 2;
int userButtonValue = 2;

void displayTime(int dot) {
  int displayValue = mm * 100 + ss;
  display.showNumberDecEx(displayValue, dot, true);
}

void saveToMemory(byte minutes, byte seconds) {
  // write to EEPROM.
  EEPROM.write(SECONDS_ADDRESS, seconds);
  EEPROM.write(MINUTES_ADDRESS, minutes);

  EEPROM.commit();    //Store data to EEPROM
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(USER_BUTTON, INPUT);
  pinMode(SWITCH, INPUT);
  // Nastaveni jasu: 0 (min) - 7 (max)
  display.setBrightness(4);

  mm = EEPROM.read(MINUTES_ADDRESS);
  ss = EEPROM.read(SECONDS_ADDRESS);

  displayTime(DOT_ON);
  nextDotChange = millis() + 500;
  nextSecondChange = nextDotChange + 500;

  delay(1000);
}

void loop() {
  userButtonValue = digitalRead(USER_BUTTON);
  if (userButtonValue == 0 && (ss != 0 || mm != 0)) {
      mm = 0;
      ss = 0;
      saveToMemory(mm, ss);
  }
  // Zobraz cas, pokud uplynula sekunda
  if (millis() > nextSecondChange) {
    nextSecondChange += 1000;

    switchValue = digitalRead(SWITCH);
    if (switchValue == 0) {
      // Inkrementuj cas
      ss++;
      if (ss >= 60) {
        ss = 0;
        mm++;
      }
      if (mm >= 100) mm = 0;
    }
    displayTime(DOT_OFF);
    if (switchValue == 0) {
      saveToMemory(mm, ss);
    }
  }

  // Zapni dvojtecku v pulce sekundy (zmena sekundy ji vypne)
  if (millis() > nextDotChange) {
    nextDotChange += 1000;
    displayTime(DOT_ON);
  }
}

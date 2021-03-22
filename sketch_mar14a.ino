#include <TM1637Display.h>
#include <EEPROM.h>

const int CLK = D6; //Set the CLK pin connection to the display
const int DIO = D5; //Set the DIO pin connection to the display
const int SWITCH = D1;
const int USER_BUTTON = 16;

#define COLON_OFF 0b00000000
#define COLON_ON  0b01000000

#define SECONDS_ADDRESS  0
#define MINUTES_ADDRESS  1

TM1637Display display(CLK, DIO);

byte elapsedMinutes = 0;
byte elapsedSeconds = 0;

unsigned long nextSecondChange = 0; // Time of next second change
unsigned long nextDotChange = 0; // Time of next displaying colon

int switchValue = 2;
int userButtonValue = 2;

void displayTime(int colon) {
  int displayValue = elapsedMinutes * 100 + elapsedSeconds;
  display.showNumberDecEx(displayValue, colon, true);
}

void saveToMemory(byte minutes, byte seconds) {
  // write to EEPROM.
  EEPROM.write(SECONDS_ADDRESS, seconds);
  EEPROM.write(MINUTES_ADDRESS, minutes);
  // Store data to EEPROM
  EEPROM.commit();    
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(USER_BUTTON, INPUT);
  pinMode(SWITCH, INPUT);
  display.setBrightness(4);  //  0 (min) - 7 (max)

  elapsedMinutes = EEPROM.read(MINUTES_ADDRESS);
  elapsedSeconds = EEPROM.read(SECONDS_ADDRESS);

  displayTime(COLON_ON);
  nextDotChange = millis() + 500;
  nextSecondChange = nextDotChange + 500;

  delay(1000);
}

void resetTime() {
  elapsedMinutes = 0;
  elapsedSeconds = 0;
  saveToMemory(elapsedMinutes, elapsedSeconds);
}

void loop() {
  userButtonValue = digitalRead(USER_BUTTON);
  bool isUserButtonPresssed = userButtonValue == 0;

  if (isUserButtonPresssed && (elapsedSeconds != 0 || elapsedMinutes != 0)) {
    resetTime();
  }
  
  switchValue = digitalRead(SWITCH);
  bool isSwitchOn = switchValue == 0;

  // If one second passed
  if (millis() > nextSecondChange) {
    nextSecondChange += 1000;

    if (isSwitchOn) {
      // Inkrementuj cas
      elapsedSeconds++;
      if (elapsedSeconds >= 60) {
        elapsedSeconds = 0;
        elapsedMinutes++;
      }
      if (elapsedMinutes >= 100) elapsedMinutes = 0;
    }
    displayTime(COLON_OFF);
    if (isSwitchOn) {
      saveToMemory(elapsedMinutes, elapsedSeconds);
    }
  }

  // Zapni dvojtecku v pulce sekundy (zmena sekundy ji vypne)
  if (millis() > nextDotChange) {
    nextDotChange += 1000;
    displayTime(COLON_ON);
  }
}

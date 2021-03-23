#include <TM1637Display.h>
#include <EEPROM.h>

const int CLK = D6; // Set the CLK pin connection to the display
const int DIO = D5; // Set the DIO pin connection to the display
const int SWITCH = D1;
const int RESET_BUTTON = D3;

#define COLON_OFF 0b00000000
#define COLON_ON  0b01000000

#define MINUTES_ADDRESS  1
#define HOURS_ADDRESS  2

TM1637Display display(CLK, DIO);

byte elapsedHours = 0;
byte elapsedMinutes = 0;
byte elapsedSeconds = 0;

// This number will overflow (go back to zero), after approximately 50 days, but also the returned value from millis() function
unsigned long nextSecondChange = 0; // Time of next second change
unsigned long nextDotChange = 0; // Time of next displaying colon

int switchValue = 2;
int resetButtonValue = 2;

void displayTime(int colon) {
  int displayValue = elapsedHours * 100 + elapsedMinutes;
  display.showNumberDecEx(displayValue, colon, true);
}

void saveToMemory(byte minutes, byte hours) {
  EEPROM.write(MINUTES_ADDRESS, minutes);
  EEPROM.write(HOURS_ADDRESS, hours);
  EEPROM.commit();
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(RESET_BUTTON, INPUT);
  pinMode(SWITCH, INPUT);
  display.setBrightness(4);  //  0 (min) - 7 (max)

  elapsedMinutes = EEPROM.read(MINUTES_ADDRESS);
  elapsedHours = EEPROM.read(HOURS_ADDRESS);

  displayTime(COLON_ON);
  nextDotChange = millis() + 500;
  nextSecondChange = nextDotChange + 500;

  delay(1000);
}

void resetTime() {
  elapsedMinutes = 0;
  elapsedSeconds = 0;
  elapsedHours = 0;
  saveToMemory(elapsedMinutes, elapsedHours);
}

void addSecond(bool isSwitchOn) {
  nextSecondChange += 1000;
  bool hasMinutesChanged = false;

  if (isSwitchOn) {
    elapsedSeconds++;
    if (elapsedSeconds >= 60) {
      elapsedSeconds = 0;
      elapsedMinutes++;
      hasMinutesChanged = true;
    }
    if (elapsedMinutes >= 60) {
      elapsedMinutes = 0;
      elapsedHours++;
    }
  }

  displayTime(COLON_OFF);

  if (hasMinutesChanged) {
    saveToMemory(elapsedMinutes, elapsedHours);
  }
}

void turnColonOn(bool isSwitchOn) {
  nextDotChange += 1000;
  if (isSwitchOn) {
    displayTime(COLON_ON);
  }
}

void loop() {
  resetButtonValue = digitalRead(RESET_BUTTON);
  bool isResetButtonPresssed = resetButtonValue == 0;

  if (isResetButtonPresssed && (elapsedSeconds != 0 || elapsedMinutes != 0)) {
    resetTime();
  }

  switchValue = digitalRead(SWITCH);
  bool isSwitchOn = switchValue == 0;

  // If one second passed
  if (millis() > nextSecondChange) {
    addSecond(isSwitchOn);
  }

  // Turn the colon on in half of second
  if (millis() > nextDotChange) {
    turnColonOn(isSwitchOn);
  }
}

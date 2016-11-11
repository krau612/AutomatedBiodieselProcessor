#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal.h>

/*
 This sketch incorperates a constant temperature reading 
 by a onewire Temperature probe during the "normal" running state
 and then goes into manual override mode when the override button
 is pressed. The override button is *. Manual override lets the 
 user press a button on the keypad to toggle the corresponding
 relay.
 
 
 Important notes
 -The code never stops looping even the arduino is waiting for a key
 to be pressed. In order to achieve this, use while loops that only end
 when the desired action in complete. Try to avoid delay functions, and 
 yes I know there are still some being used
 */

const int ONE_WIRE_BUS = 53;  // Data wire is plugged into 53 on the Arduino
const int BACKLIGHT_PIN = 13; // Backlight plugged into 13 on the Arduino


// Assign the relay pins
const int RELAY_1 = 31;
const int RELAY_2 = 33;
const int RELAY_3 = 35;
const int RELAY_4 = 37;
const int RELAY_5 = 39;
const int RELAY_6 = 41;
const int RELAY_7 = 43;
const int RELAY_8 = 45;

// Pin 13 for test led
// blinks each cycle of the loop
const int TEST_LED = 13;

// Keypad constants
const byte ROWS = 4;
const byte COLS = 4;
byte ROW_PINS[ROWS] = { 
  9, 8, 7, 6 };
byte COL_PINS[COLS] = { 
  5, 4, 3, 2 };
const char keymap[ROWS][COLS] = {
  { 
    '1', '2', '3', 'A'}
  ,
  { 
    '4', '5', '6', 'B'}
  ,
  { 
    '7', '8', '9', 'C'}
  ,
  { 
    '*', '0', '#', 'D'}
};


int relayNum;  // Currently selected relay number
int serialNum; // Most recently pressed key on keypad
char serialOC; // State of the selected relay (o for open, c for closed)
bool ovMode;   // State of override mode (false for normal mode, true for override mode)
float temp;    // Recorded temperature


// Setup a oneWire instance to communicate with any OneWire device
// not just Maxim/Dallas temperature ICs
OneWire oneWire(ONE_WIRE_BUS);

// Pass oneWire reference to Dallas Temperature
DallasTemperature sensors(&oneWire);

// Initalize keypad
Keypad keypad(makeKeymap(keymap), ROW_PINS, COL_PINS, ROWS, COLS);

// Initialize LCD and set the LCD I2C Address
LiquidCrystal_I2C lcd(0x3f, 2, 1, 0, 4, 5, 6, 7, 8, POSITIVE);



void setup () {
  // Initialize serial connection
  Serial.begin(9600);
  sensors.begin();

  // Set all relay pins to high because high = open
  for (int i = 31; i <= 45; i += 2) {
    pinMode(i, OUTPUT);
    digitalWrite(i, HIGH);
  }

  // Turn on LCD
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH);
  lcd.begin(16, 2);
  keypad.addEventListener(keypadEvent);
  Serial.println("Standby");
  lcd.home();
  lcd.print("Initialized");
  delay(1000);
  lcd.clear();
}

void loop () {
  // Set cursor to top left
  lcd.setCursor(0, 0);
  lcd.print("Normal Mode");

  // Scans for pressed key
  char key = keypad.getKey();

  // Update temperatures and display them
  sensors.requestTemperatures();
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  temp = sensors.getTempCByIndex(0);
  lcd.print(temp, 2);

  // Blink the test led
  digitalWrite(TEST_LED, HIGH);
  delay(50);
  digitalWrite(TEST_LED, LOW);
}

/*
 This function is called whenever a key is pressed.
 Note: this function will stop the main loop code
 until it is complete.
 */
void keypadEvent (KeypadEvent key) {
  if (keypad.getState() == PRESSED) {
    switch (key) {
    case '*':
      lcd.clear();
      ovMode = true;
      Serial.print(ovMode);
      lcd.print("Override Mode");
      delay(1000);

      while (ovMode) {
        relayfunc();
      }

      break;
    case '#':
      lcd.clear();
      ovMode = false;
      lcd.print("Normal Mode");
      delay(1000);
      break;
    default:
      break;
    }
  } 
  else {
  }
}

void relayfunc () {
  lcd.clear();
  lcd.print("Relay Num?: ");
  boolean noKey = true;

  // Loop until relay number is chosen
  while (noKey && ovMode) {
    int key = keypad.getKey();
    if ((key > 0) && (key < 19)) {
      serialNum = key;
      lcd.print(serialNum);
      lcd.setCursor(0, 1);
      noKey = false;
    }
  }


  lcd.print("Open or Close?: ");
  noKey = true;

  // Loop until choice is made to open/close
  while (noKey && ovMode) {
    char key = keypad.getKey();
    if ((key == 'A') || (key == 'B')) {
      serialOC = key;
      lcd.print(serialOC);
      noKey = false;
    }
  }


  // Match relay number to pin on arduino
  switch (serialNum) {
  case 1:
    relayNum = RELAY_1;
    break;
  case 2:
    relayNum = RELAY_2;
    break;
  case 3:
    relayNum = RELAY_3;
    break;
  case 4:
    relayNum = RELAY_4;
    break;
  case 5:
    relayNum = RELAY_5;
    break;
  case 6:
    relayNum = RELAY_6;
    break;
  case 7:
    relayNum = RELAY_7;
    break;
  case 8:
    relayNum = RELAY_8;
    break;
  }

  lcd.clear();

  // Open/Close the selected valve
  if (serialOC == 'A' && ovMode) {
    digitalWrite(relayNum, LOW);

    lcd.print("Valve ");
    lcd.setCursor(6, 0);
    lcd.print(serialNum);
    lcd.setCursor(8, 0);
    lcd.print(" Opened");
  } 
  else if (serialOC == 'B' && ovMode) {
    digitalWrite(relayNum, HIGH);

    lcd.print("Valve ");
    lcd.setCursor(6, 0);
    lcd.print(serialNum);
    lcd.setCursor(8, 0);
    lcd.print(" Closed");
  }

  // Give time for valve to open/close
  delay(1000);
}

// Grouping
// Stage #
// Description
// I/O
// End condition

// WVO loading group
void step1 () {
  // load VWO through filter
  // Everything turned off
  // when loading is finished
}
// end WVO loading group

// Prewash group
void step2 () {
  // heat oil
  // turn on bubblers
  // mist water into washer/drier
}

void step3 () {
  // Keep bubblers running
  // p3 on
  // time
}

void step4 () {
  // wait for wash water to settle
  // everything turned off
  // time
}

void step5 () {
  // drain wash water
  // v5 open
  // until light sensor registers oil instead of water
}

void step6 () {
  // dry VWO
  // p2 on
  // time
}
// end prewash group

// Reaction group
void step7 () {
  // transfer VWO into reactor, attach carboy w/Methoxide, ball valve must be closed
  // v1 open
  // v3 close
  // p1 on
  // when liquid sensor detects no more oil in washer dryer
}

void step8 () {
  // heat oil/methoxide
  // reactor heat on
  // until oil is about 50C
}

void step9 () {
  // mix/reaction
  // reactor heat on
  // p1 on
  // time
}

void step10 () {
  // transfer BD/glycerol to washer dryer
  // v2 close
  // v4 open
  // p1 on
  // when liquid level sensor detects oil has return to washer dryer
}
// end reaction group

// start BD Separation/water wash group
void step11 () {
  // drain glycerol
  // v5 open
  // until light sensor registers BD instead of oil
}

void step12 () {
  step2();
}

void step13 () {
  step3();
}

void step14 () {
  step4();
}

void step15 () {
  step5();
}

void step16 () {
  step6();
}

void step17 () {
  // pump out BD, turn washer dryer ball valve
  // p2 on
}

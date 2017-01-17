#include <Keypad.h>
#include <Wire.h>
#include <NewPing.h>
#include "Adafruit_TCS34725.h"
/* This sketch incorperates a constant temperature reading 
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
#include <OneWire.h>
#include <DallasTemperature.h>
 
// Data wire is plugged into pin 53 on the Arduino
#define ONE_WIRE_BUS 53
#define ONE_WIRE_BUS2 52

// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
OneWire oneWire2(ONE_WIRE_BUS2);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
DallasTemperature sensors2(&oneWire2);


//initialize the keypad
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
  {1,2,3,'A'},
  {4,5,6,'B'},
  {7,8,9,'C'},
  {'*',0,'#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 4, 3, 2}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


//Initialize the LCD display
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#define BACKLIGHT_PIN 13
LiquidCrystal_I2C lcd(0x3f, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); // Set the LCD I2C address


//assign the relay pins
const int relay1  = 31;
const int relay2  = 33;
const int relay3  = 35;
const int relay4  = 37;
const int relay5  = 39;
const int relay6  = 41;
const int relay7  = 43;
const int relay8  = 45;

const int MAX_DIST = 100; // 100cm. 55gal drum is 85.1cm
const int TRIGGER_PIN = 10;
const int ECHO_PIN = 11; //pwm
const int TRIGGER_PIN2 = 12;
const int ECHO_PIN2 = 13; //pwm

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DIST);
NewPing sonar2(TRIGGER_PIN2, ECHO_PIN2, MAX_DIST);

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);
boolean tcsFound = false;


int relaynum; //the currently selected relay number
int serialnum; //stores the number from the keypad that was most recently pressed
char serialoc; //stores the current state of the selected relay (o for open, c for closed)
bool ovmode = false; //stores the boolean state of override mode (false for "normal" mode, true for override mode)
const int testled = 13; //pin 13 for the test led (blinks each cycle of the loop)
float temp; //value of the recorded temperature


void setup() {
  //Initialize serial connection
  Serial.begin(9600);
  sensors.begin();
  
  //set all relay pins to high becasuse the relays are open when the pins are set to high
  for (int i=31; i <= 45;){
    pinMode(i, OUTPUT);
    digitalWrite(i,HIGH);
    i = i + 2;
   } 


   //Turn on LCD
   pinMode ( BACKLIGHT_PIN, OUTPUT );
   digitalWrite ( BACKLIGHT_PIN, HIGH );
   lcd.begin(16,2); // initialize the lcd
   keypad.addEventListener(keypadEvent);
   Serial.println ("Standby");
   lcd.home (); // go home
   lcd.print("Initialized");
   delay ( 1000 );
   lcd.clear();
   
   
   if (tcs.begin()) {
    Serial.println("Found sensor");
    tcsFound = true;
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    tcsFound = false;
  }
  
}


void loop() {
  lcd.setCursor(0, 0); //set curser to top left
  lcd.print ("Normal mode");
  char key = keypad.getKey(); //Scans for pressed key
  
  sensors.requestTemperatures(); // Send the command to get temperatures
  sensors2.requestTemperatures();
  lcd.setCursor(0, 1);
  lcd.write("Temp:");
  temp = sensors.getTempCByIndex(0); //store temperature reading
  lcd.print(temp,2);
  temp = sensors2.getTempCByIndex(0); //store temperature reading
  lcd.write("_");
  lcd.print(temp,2);
  
  //blink the LED
  digitalWrite(testled, HIGH);   
  delay(50);              
  digitalWrite(testled, LOW);   
  
  getLiquidLevel();
  Serial.print(" ----- ");
  getLiquidColor();
  Serial.print("\n");
}


//End of loop

void getLiquidLevel () {
  unsigned int uS = sonar.ping();
  Serial.print("Ping: ");
  Serial.print(uS / US_ROUNDTRIP_CM);
  Serial.print("cm ------ ");
  
  uS = sonar2.ping();
  Serial.print("Ping: ");
  Serial.print(uS / US_ROUNDTRIP_CM);
  Serial.print("cm ------ ");
}

void getLiquidColor () {
  if (tcsFound) {
    uint16_t r, g, b, c, colorTemp, lux;
    
    tcs.getRawData(&r, &g, &b, &c);
    colorTemp = tcs.calculateColorTemperature(r, g, b);
    lux = tcs.calculateLux(r, g, b);
    
    Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
    Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
    Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
    Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
    Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
    Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
    Serial.print(" ");
  }
}
  


/*This function is called whenever a key is pressed.
 * Note: this function will stop the main loop code
 * until it is complete.
 * 
 */
void keypadEvent(KeypadEvent key){
    switch (keypad.getState()){
    case PRESSED:
    if (key == '*') {
      lcd.clear();
      ovmode = true;
      Serial.print(ovmode);
      lcd.print ("Override mode");
      delay(1000);
      while (ovmode) {  
        relayfunc();  //Runs this function until the # key is pressed
      }
    }
    if (key == '#') {
      lcd.clear();
      ovmode = false;
      lcd.print ("Normal mode");
      delay(1000);  
      //After this line is complete the arduino wil continue where it left off in the loop
    }
        break;
    }
}


int relayfunc() {
  lcd.clear();
  lcd.write("Relay Num?:");
  boolean NO_KEY = true;
  //Select the relay number
  while (NO_KEY && ovmode){ //waits until key is pressed
    int key = keypad.getKey();
    if ((key > 0) && (key < 9)) {
      serialnum = key;
      lcd.print (serialnum);
      lcd.setCursor(0, 1);
      NO_KEY = false;
      }
  }
  lcd.print("Open or Close?:");
    NO_KEY = true;
    //Chose to open or close the selected relay
  while (NO_KEY && ovmode){ //waits until key is pressed
    char key = keypad.getKey();
    if ((key == 'A') || (key == 'B')) {
      serialoc = key;
      lcd.print (serialoc);
      NO_KEY = false;
      }
  }


  //This is bad coding on my part but I use this switch function to match the relay number to the right pin on the arduino
  switch (serialnum) {
    case 1:
    relaynum = relay1;
    break;
    case 2:
    relaynum = relay2;
    break;
    case 3:
    relaynum = relay3;
    break;
    case 4:
    relaynum = relay4;
    break;
    case 5:
    relaynum = relay5;
    break;
    case 6:
    relaynum = relay6;
    break;
    case 7:
    relaynum = relay7;
    break;
    case 8:
    relaynum = relay8;
    break;
  }
  lcd.clear();


  //Executes the opening/closing of the selected valve and prints it on the lcd
if (serialoc == 'B' && ovmode){
  digitalWrite(relaynum,HIGH);
  lcd.print ("Valve ");
  lcd.setCursor(6, 0);
  lcd.print (serialnum);
  lcd.setCursor(8, 0);
  lcd.print (" Closed");
}
if (serialoc == 'A' && ovmode){
  digitalWrite(relaynum,LOW);
  lcd.print ("Valve ");
  lcd.setCursor(6, 0);
  lcd.print (serialnum);
  lcd.setCursor(8, 0);
  lcd.print (" Opened");
}
delay(1000); //This gives time for the valve to open/close
}








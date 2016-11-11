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

// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);


//initialize the keypad
#include <Keypad.h>
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
  
}


void loop() {
  lcd.setCursor(0, 0); //set curser to top left
  lcd.print ("Normal mode");
  char key = keypad.getKey(); //Scans for pressed key
  sensors.requestTemperatures(); // Send the command to get temperatures
  lcd.setCursor(0, 1);
  lcd.write("Temp:");
  temp = sensors.getTempCByIndex(0); //store temperature reading
  lcd.print(temp,2);
  //blink the LED
  digitalWrite(testled, HIGH);   
  delay(50);              
  digitalWrite(testled, LOW);   
  
}


//End of loop


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








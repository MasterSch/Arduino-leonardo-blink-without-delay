#include <Arduino.h>
// RX Led ist bei Pin 7, TX Led ist bei Pin 14 !!!!
// bei micro LEDs invertiert


//  On and Off Times (as int, max=32secs)  // micro
/*
const unsigned int onTime1 = 750;
const unsigned int offTime1 = 10;
const unsigned int onTime2 = 10;
const unsigned int offTime2 = 700;
*/


// On and Off Times (as int, max=32secs)  // pro micro und Leonardo
const unsigned int onTime1 = 50;
const unsigned int offTime1 = 750;
const unsigned int onTime2 = 700;
const unsigned int offTime2 = 50;


// Tracks the last time event fired
unsigned long previousMillis1=0;
unsigned long previousMillis2=0;
 
// Interval is how long we wait
int interval1 = onTime1;
int interval2 = onTime2;
 
// Used to track if LED should be on or off
boolean RXLed = true;
boolean TXLed = true;
 
int RXLED = 17;

// Usual Setup Stuff
void setup() {

 Serial.begin(9600); //This pipes to the serial monitor
 Serial1.begin(9600); //This is the UART, pipes to sensors attached to board

pinMode(RXLED, OUTPUT);  // Set RX LED as an output
 // TX LED is set as an output behind the scenes

digitalWrite(RXLED, HIGH);   // set the LED on
TXLED0; //TX LED is not tied to a normally controlled pin, 0 = an!!  


}
 
void loop() {

  // Grab snapshot of current time, this keeps all timing
  // consistent, regardless of how much code is inside the next if-statement
  unsigned long currentMillis = millis();

  // Compare to previous capture to see if enough time has passed
  if ((unsigned long)(currentMillis - previousMillis1) >= interval1) {
    // Change wait interval, based on current LED state
    if (RXLed) {
      // LED is currently on, set time to stay off
      interval1 = offTime1;
    } else {
      // LED is currently off, set time to stay on
      interval1 = onTime1;
    }
    // Toggle the LED's state, Fancy, eh!?
    RXLed = !(RXLed);
    digitalWrite(RXLED, !RXLed); 
    // Save the current time to compare "later"
    previousMillis1 = currentMillis;
  }
   if ((unsigned long)(currentMillis - previousMillis2) >= interval2) {
    // Change wait interval, based on current LED state
    if (TXLed) {
      // LED is currently on, set time to stay off
      interval2 = offTime2;
    } else {
      // LED is currently off, set time to stay on
      interval2 = onTime2;
    }
    // Toggle the LED's state, Fancy, eh!?
    TXLed = !(TXLed);
      if (TXLed) TXLED0;
      else TXLED1;//TX LED is not tied to a normally controlled pin
    // Save the current time to compare "later"
    previousMillis2 = currentMillis;
  } 

// hier Code der immer ausgeführt wird!!!!
  
  
}  // ende von loop


 


/*
und hier das blink mit der onboard LED

#include <Arduino.h>
// These variables store the flash pattern
// and the current state of the LED

int ledPin =  13;                 // the number of the LED pin
int ledState = LOW;               // ledState used to set the LED
unsigned long previousMillis = 0; // will store last time LED was updated
long OnTime = 50;                // milliseconds of on-time
long OffTime = 450;               // milliseconds of off-time



void setup() 
{
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, 0);
  Serial.begin(115200);
  delay(3000);   
}

void loop()
{
  // check to see if it's time to change the state of the LED
  unsigned long currentMillis = millis();
 
  if((ledState == HIGH) && (currentMillis - previousMillis >= OnTime))
  {
    previousMillis += OnTime;          // on time is over, set new start
    ledState = LOW;                    // Turn it off
    digitalWrite(ledPin, ledState);    // Update the actual LED
    Serial.println(previousMillis);
  }
  else if ((ledState == LOW) && (currentMillis - previousMillis >= OffTime))
  {
    previousMillis += OffTime;           // off time is over, set new start
    ledState = HIGH;                     // turn it on
    digitalWrite(ledPin, ledState);	     // Update the actual LED
    Serial.println(previousMillis);
  }
}

*/

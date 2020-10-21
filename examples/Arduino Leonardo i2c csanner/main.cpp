/**
 * I2CScanner.ino -- I2C bus scanner for Arduino
 *
 * 2009,2014, Tod E. Kurt, http://todbot.com/blog/
 *
 */
#include <Arduino.h>
#include <Wire.h>
//extern "C" { 
//#include "utility/twi.h"  // from Wire library, so we can do bus scanning
//}

// Scan the I2C bus between addresses from_addr and to_addr.
// On each address, call the callback function with the address and result.
// If result==0, address was found, otherwise, address wasn't found
// (can use result to potentially get other status on the I2C bus, see twi.c)
// Assumes Wire.begin() has already been called



// standard Arduino setup()

void setup()
{
    Wire.begin();
    Serial.begin(115200);      // Changed from 19200 to 9600 which seems to be default for Arduino serial monitor
    delay(5000);
    Serial.println("\nI2CScanner ready!");
    // Set pin mode so the loop code works ( Not required for the functionality)
    pinMode(13, OUTPUT);

}


// standard Arduino loop()
void loop() 
{
   byte start_address = 8;       // lower addresses are reserved to prevent conflicts with other protocols
   byte end_address = 119;       // higher addresses unlock other modes, like 10-bit addressing
    
    Serial.println("\nI2CScanner ready!");

    Serial.print("starting scanning of I2C bus from ");
    Serial.print(start_address,DEC);
    Serial.print(" to ");
    Serial.print(end_address,DEC);
    Serial.println("...");

  byte error = 0;
  for( byte addr = start_address; addr <= end_address; addr++ ) 
  {
    Wire.beginTransmission(addr);
    error = Wire.endTransmission();
  Serial.print("addr: ");
  Serial.print(addr,DEC);
  Serial.print( (error==0) ? " found":"      ");
  Serial.print( (addr%4) ? "\t":"\n");
  }

    Serial.println("\ndone");
    


    // Nothing to do here, so we'll just blink the built-in LED
    int x = 0;
    do {
    digitalWrite(13, HIGH);
    delay(300);
    digitalWrite(13, LOW);
    delay(300);
    x++;
    } while (x < 30);
}
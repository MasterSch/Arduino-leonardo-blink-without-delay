#include <Arduino.h>

// Arduino Leonardo mit 2 LED Modulen an mcp23017
// A0 A1 A2 auf 0   oder offen lassen...
// Adresse ist 0x20

/*
 Example 41.1 - Microchip MCP23017 with Arduino
 http://tronixstuff.com/tutorials > chapter 41
 John Boxall | CC by-sa-nc
*/

// pins 15~17 to GND, I2C bus address is 0x20

#include "Wire.h"
void setup()
{
 // Serial.begin(115200);

 Wire.begin(); // wake up I2C bus
// set I/O pins to outputs
 
 Wire.beginTransmission(0x20);
 Wire.write(0x00); // IODIRA register
 Wire.write(0x00); // set all of port A to outputs
 Wire.endTransmission();
 Wire.beginTransmission(0x20);
 Wire.write(0x01); // IODIRB register
 Wire.write(0x00); // set all of port B to outputs
 Wire.endTransmission();
}
void output(int a)
{
 Wire.beginTransmission(0x20);
 Wire.write(0x12); // GPIOA
 //Wire.write(~lowByte(a)); // port A
 Wire.write(lowByte(a)); // port A
 Wire.endTransmission();
 Wire.beginTransmission(0x20);
 Wire.write(0x13); // GPIOB
 //Wire.write(~highByte(a)); // port B
 Wire.write(highByte(a)); // port B
 Wire.endTransmission();
}

void loop()
{

int count = 1;    
int dir = 1;
while(1)
 {

if (dir == 1)
   {
if (count == 512) 
{
    count = count >> 1;
    dir = 0;
}
    else count = count << 1;
   }   else  
   {
if (count == 1) 
{
    count = count << 1;
    dir = 1;
}
    else count = count >> 1;
   }

 output(count);
//Serial.println(count);
 delay (150);
 }
}
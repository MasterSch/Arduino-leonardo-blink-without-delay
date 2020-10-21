#include <Arduino.h>

// Arduino Leonardo mit 2 LED Modulen an PCF8574
// LED an bei output Low
// I2C bus address is 0x20 und 0x21

#include "Wire.h"
void setup()
{
 // Serial.begin(115200);

 Wire.begin(); // wake up I2C bus
// set I/O pins to outputs
}

void output(int a)
{
 Wire.beginTransmission(0x20);
 Wire.write(~lowByte(a)); // port A
 Wire.endTransmission();
 Wire.beginTransmission(0x21);
 Wire.write(~highByte(a)); // port B
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
 delay (50);
 }
}



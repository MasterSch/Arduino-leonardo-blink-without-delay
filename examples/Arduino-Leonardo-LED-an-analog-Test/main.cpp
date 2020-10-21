#include <Arduino.h>


void setup() {
  // put your setup code here, to run once:

pinMode(18, OUTPUT);
pinMode(19, OUTPUT);
pinMode(20, OUTPUT);
pinMode(21, OUTPUT);
pinMode(22, OUTPUT);
pinMode(23, OUTPUT);

digitalWrite(18,0);
digitalWrite(19,0);
digitalWrite(20,0);
digitalWrite(21,0);
digitalWrite(22,0);
digitalWrite(23,0);

}

void loop() {
  // put your main code here, to run repeatedly:

int dir = 1, i = 18;
do {
digitalWrite(i,1);
delay(100);
digitalWrite(i,0);
if (dir == 1) 
{
if ( i == 23) dir = 0;
else i++;
} else {
if ( i == 18) dir = 1;
else i--;
   }

 } while(1);

}
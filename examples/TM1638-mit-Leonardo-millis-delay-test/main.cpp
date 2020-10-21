#include <Arduino.h>
#include <tm1638.h>    //  von https://github.com/int2str/TM1638


//  STB      _BV(PB4)     Leonardo PIN 8
//  CLK      _BV(PB5)     Leonardo PIN 9
//  DAT      _BV(PB6)     Leonardo PIN 10
//  in der tm1638.h datei angepasst
//  display.setLEDs(rot,grün)

TM1638 display;

char buffer[20] = "Start...";
const char* pos;
unsigned long currTime, prevTime = 0, interval = 50;

int waitTime = 0, Demo = 1, x = 0, y = 1;
int status = 0, ende = 0;

byte buttonOld = 0, buttonNew = 0, redstatus=0, greenstatus=0;

void DoIt();


void setup() {
    // put your setup code here, to run once:

     display.setChars(buffer, 0);

     delay(3000);

     display.clear();

}

void loop() {
    // put your main code here, to run repeatedly:



currTime = millis();
if (currTime - prevTime >= interval) {    // 50ms abgelaufen, tu jetzt was...
if (status > 0)   DoIt();           // eine Aufgabe ist zu erledigen


prevTime = currTime;  // starte Timer neu

                       // hier der normale Code

}
if (status == 0)  {

ende = sprintf(buffer,"Demo   %d",y++);
status = 1;

}

}


void DoIt() {

switch (status)  {

case 1:     // init
waitTime = 20;     // 20 mal 50ms = 1 Sek.
x = 0;
ende -= 7;
display.setChars(buffer, 0);
status++;
break;

case 2:     // wait
if (waitTime > 0) waitTime--;
else status++; 
break;


case 3:     //  shift out
if (waitTime == 0)  {
waitTime = 3;
    if (x < ende)  {
    pos = buffer + x;
    display.setChars(pos, 0);
    x++;
    } else {
        waitTime = 20;     // 20 mal 50ms = 1 Sek.
        status++; }
} else waitTime--;
break;

case 4:     // wait 
if (waitTime > 0) waitTime--;
else status++; 
break;

case 5:     //  finished
      display.clear();
      status = 0;
break;


}
}

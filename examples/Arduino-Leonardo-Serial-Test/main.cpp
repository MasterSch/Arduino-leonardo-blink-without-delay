#include <Arduino.h>
/*
Serial Test

works with Serial and Serial1 on Arduino micro, pro micro, Leonardo, Mega 2560

 modified 27 Dec 2017
 by Günter Schmitt
 based on Mikal Hart's example

 This example code is in the public domain.

 */

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Serial1.begin(115200);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }



  Serial.println("...start....enter   end for exit.....");
  Serial1.println("ATE0");   // Echo off
}

void loop() { // run over and over
byte a = ('\n');
int b = 3;
do {
   if (Serial1.available())  {
     a = Serial1.read();
  // Serial.print(" ");  
   if (a == 13) Serial.print("\\r");
   if (a == 10) Serial.println("\\n");
   if (a > 13)  Serial.write(a);
  // Serial.println(" ");
  }
  if (Serial.available()) {
    a = (Serial.read());
  
  switch(b) {

   case(3):
    if (a == 101) 
    {
      b--;
      break;
    }

    case(2):
     if (a == 110)
     {
       b--; 
       break;
     }
       else 
     {
       b = 3;
       break;
     }

    case(1):
     if (a == 100)
     {
       b--; 
       break;
     }
       else 
     {
       b = 3;
       break;
     }

    case(0):
     if (a == 13)
     {
        Serial.println(" ");
        Serial.println(" Serial wird beendet..... ");
        Serial.println(" ");
        Serial1.end();
        Serial.end();
        delay(3000);
        while(1);
       }
     }
     Serial.print(" send  ");
     Serial.print(a);     // wert dezimal
     Serial.print("   ");
    if (a == 13) Serial.print("\\r");
    if (a == 10) Serial.print("\\n");
    if (a >13) Serial.write(a);     // wert als zeichen
     Serial.println("  ");
     Serial1.write(a);
 //  }
  }
} while (1);
}


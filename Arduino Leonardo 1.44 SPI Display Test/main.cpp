
#include "Arduino.h"

#include <TM1638GS.h>




// I/O pins on the Arduino connected to strobe, clock, data
// (power should go to 3.3v and GND)
//TM1638GS LedAndKey(14,12,13);   // D5 D6 D7     WeMos D1R2
TM1638GS LedAndKey(5,6,7);    // D5 D6 D7     Leonardo

char string[] = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtuUVvWwXxYyZz";
int end = sizeof(string) - 8;
char text[9] =  "        ";



void setup() {

Serial.begin(115200);
delay(3000);
Serial.println("");
Serial.println("___________start_____________");
Serial.println("");

LedAndKey.reset();

}



void loop() {

  int x = 0;
  
  LedAndKey.set_Chars(0,"briGHt  ");

  do {
  LedAndKey.setColorLED(x, 1);
  LedAndKey.set_Brightness(x);
  LedAndKey.set_Char(7, 48+x);
  delay(600);
  x++;
  } while(x<8); 

  LedAndKey.reset();    
  LedAndKey.set_Brightness(2);
  delay(2000);

  LedAndKey.set_Chars(0,"12345678");
  LedAndKey.setColorLED(0, 1);
  LedAndKey.setColorLED(1, 2);
  LedAndKey.setColorLED(6, 2);
  LedAndKey.setColorLED(7, 1);
  delay(2000);
  LedAndKey.set_Chars(0,"--------");
  delay(2000);
  
  LedAndKey.reset();    
  delay(2000);
  
  //const char* pos = string;
  //Chars(pos, 0);
  

     x = 0;
     do {
     LedAndKey.set_Hex(x, x+8);
     delay(300);
     LedAndKey.set_Char(x,' ');
     x++;
     } while(x<8); 
     delay(2000);

     LedAndKey.clear_digits();
     delay(2000);

    strcpy(string, "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz");
    int y = 0;
    while (string[y]) { 
      y++; 
    };
    if (y < 8) y = 8;
    x = 0;
    do {
    char* position = string + x;
    LedAndKey.set_Chars(0, position);
    if (x == 0) delay(2000);
      else  delay(200);
      x++;
    } while (x+7 < y);
      delay(3000);
      LedAndKey.clear_digits();
      delay(2000);


      LedAndKey.setColorLEDs(0xF0, 0x0F);   //<-- left byte green, right byte red
      delay(2000);
      LedAndKey.setColorLEDs(0x55, 0x00);   //<-- left byte green, right byte red
      delay(2000);
      LedAndKey.setColorLEDs(0x00, 0xAA);   //<-- left byte green, right byte red
      delay(2000);
      LedAndKey.setColorLEDs(0x55, 0xAA);   //<-- left byte green, right byte red
      delay(2000);
      LedAndKey.clear_LEDs();
      x = 0;
      do {
      LedAndKey.setColorLED(x, 1);
      delay(150);
      LedAndKey.setColorLED(x, 0);
      x++;
      } while(x<8); 
      
      do {
      x--;  
      LedAndKey.setColorLED(x, 2);
      delay(150);
      LedAndKey.setColorLED(x, 0);
      } while(x != 0); 
  
      x = 0;
      do {
      LedAndKey.setColorLED(x, 3);
      delay(150);
      LedAndKey.setColorLED(x, 0);
      x++;
      } while(x<8); 
       delay(1000);

      //LedAndKey.set_Chars(0,"button  ");
      x = 0;
      while(x < 4000) {    
      uint8_t value = LedAndKey.get_buttons();
      if (value > 128) break;
      if (value == 0) {
        LedAndKey.set_Chars(0,"button  ");
        LedAndKey.set_Char(7, ' ');  
      } 
      for (uint8_t position = 0; position < 8; position++) {
       LedAndKey.setColorLED(position, value & 1);
       if ((value & 1) == 1) 
              LedAndKey.set_Hex(7, position+1);
       value = value >> 1;
       x++; 
         } 
       delay(50);
       x++;
      }
      LedAndKey.clear_LEDs();
      LedAndKey.set_Chars(0,"restart ");
      delay(2000);
      LedAndKey.clear_digits();    
      delay(1000);

}





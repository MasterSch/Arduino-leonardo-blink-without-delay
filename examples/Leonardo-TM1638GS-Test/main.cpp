#include <Arduino.h>

#include <TM1638GS.h>




// I/O pins on the Arduino connected to strobe, clock, data
// (power should go to 3.3v and GND)
//TM1638GS tm(0,2,14);   // D3 D4 D5     WeMos D1R23
TM1638GS LedAndKey(5,6,7);    // D5 D6 D7     Leonardo

void doLEDs(uint8_t value);

char string[] = "IP = 192.168.2.71";
int end = sizeof(string) - 8;
char text[9] =  "        ";


void setup() {
LedAndKey.reset();

}

void loop() {

  int x = 0;
  
  LedAndKey.displayText("AbCdEF");
  LedAndKey.setColorLED(0, 1);
  LedAndKey.setColorLED(1, 2);
  LedAndKey.setColorLED(6, 2);
  LedAndKey.setColorLED(7, 1);
  delay(2000);
  LedAndKey.clearLEDs();
  delay(1000);
  
  strcpy(string, "briGht");
  const char* pos = string;
  LedAndKey.displayText(pos);
  
  x = 0;
  do {
  LedAndKey.setColorLED(x, 1);
  LedAndKey.setBrightness(x);
  LedAndKey.displayASCII(7, x+48);
  delay(500);
  x++;
  } while(x<8); 

  LedAndKey.setBrightness(2);
  LedAndKey.clearLEDs();
  LedAndKey.clear();     

  delay(1000);

  LedAndKey.displayHex(0, 8);
  LedAndKey.displayHex(1, 9);
  LedAndKey.displayHex(2, 10);
  LedAndKey.displayHex(3, 11);
  LedAndKey.displayHex(4, 12);
  LedAndKey.displayHex(5, 13);
  LedAndKey.displayHex(6, 14);
  LedAndKey.displayHex(7, 15);

  delay(2000);
  LedAndKey.clear();     
  delay(2000);

    x = 0;
    strcpy(string, "IP = 192.168.2.71");
    while (x < end)  {

    const char* pos = string + x;

    //display.setChars(pos, 0);
    LedAndKey.displayText(pos);
    if (x == 0) delay(1000);
      else  delay(300);
      x++;
    } 

      delay(3000);
      LedAndKey.clear();
      delay(2000);
      LedAndKey.setColorLEDs(0xF0, 0x0F);   //<-- Right half green, left half red
      delay(2000);
      LedAndKey.setColorLEDs(0x55, 0x00);   //<-- Right half green, left half red
      delay(2000);
      LedAndKey.setColorLEDs(0x00, 0xAA);   //<-- Right half green, left half red
      delay(2000);
      LedAndKey.clearLEDs();
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
 
      strcpy(string, "buttons");
      pos = string;
      LedAndKey.displayText(pos);
      //LedAndKey.displayText("buttons");

      x = 0;
      do {    
      uint8_t buttons = LedAndKey.readButtons();
      doLEDs(buttons);
      x++;
      } while(x < 2000);
      

//      delay(2000);
      LedAndKey.clear();     
      delay(2000);



}

// scans the individual bits of value
void doLEDs(uint8_t value) {
  for (uint8_t position = 0; position < 8; position++) {
    LedAndKey.setColorLED(position, value & 1);
    value = value >> 1;
  }
}


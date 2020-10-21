

/*
  12.11.2017
  Arduino Leonardo mit LCDKeypad shield und DS3231 und grosse und kleine Anzeigeund stellen


  http://mikeyancey.com/hamcalc/lcd_characters.php
  LiquidCrystal_i2C.h ist aus NewLiquidCrystal Library
  https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home
  http://www.nikolaus-lueneburg.de/2016/02/new-liquidcrystal-library/
  https://github.com/marcmerlin/NewLiquidCrystal
  
  https://github.com/dzindra/LCDKeypad
  Tasten 0=keine, 1=rechts,2=hoch, 3=runter, 4=links, 5=select, 255=blocked
  KEYPAD_NONE, KEYPAD_UP, KEYPAD_DOWN, KEYPAD_LEFT, KEYPAD_RIGHT, KEYPAD_SELECT, KEYPAD_BLOCKED

   Schaltjahr
   Die Regel lautet: Alles, was durch 4 teilbar ist, ist ein Schaltjahr.
   Es sei denn, das Jahr ist durch 100 teilbar, dann ist es keins.
   Aber wenn es durch 400 teilbar ist, ist es doch wieder eins.

  if ((uJahr % 400) == 0) return true;
  else if ((uJahr % 100) == 0) return false;
  else if ((uJahr % 4) == 0) return true;
  return false;





 */


// include the library
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include "LCDKeypad.h"

void rtcReadTime();
void rtcReadSec();
byte decToBcd(byte val);
byte bcdToDec(byte val);
void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year);
byte calcDayOfWeek(int jahr, byte monat, byte tag);
int Schaltjahr(int year);
void updateTimegross();
void updateTimeklein();
void UhrstellenAnzeigen();
void Blinken();
void Wertsichern();
void Uhrstellen();
void printNumber(int value, int col);




// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
LCDKeypad keys;



#define RTC_I2C_ADDRESS 0x68 // I2C Adresse des RTC  DS3231

//   Display_kleingross  1=grosse Zahlen  0=kleine Anzeige
int jahr, monat, tag, stunde, minute, sekunde, wochentag, oldseconds;
int Min = 0, Max = 0, Row = 0, Col = 0, Temp = 0;
int Display_kleingross = 1, blinkOn = 0, timeLong = 0, stellen = 0, button = 0, blink = 0;
int daysInMonth[13]={0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
String daysInWeek[8] = {"Son", "Mon", "Die", "Mit", "Don", "Fre", "Sam","Son",};
String monthInYear[13] = {"xxx","Jan","Feb", "Mrz", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"};
unsigned long Now = millis(), Last = Now, Intervall = 200;

int Down = 1, backlightPin = 10;    // backlight connected to digital pin 10

//Ließt den aktuellen Zeitstempel aus dem RTC Modul.

void rtcReadTime(){

  Wire.beginTransmission(RTC_I2C_ADDRESS); //Aufbau der Verbindung zur Adresse 0x68
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(RTC_I2C_ADDRESS, 7);
  sekunde    = bcdToDec(Wire.read() & 0x7f);
  minute     = bcdToDec(Wire.read()); 
  stunde     = bcdToDec(Wire.read() & 0x3f); 

  wochentag  = bcdToDec(Wire.read());
  tag        = bcdToDec(Wire.read());
  monat      = bcdToDec(Wire.read());
  jahr       = bcdToDec(Wire.read())+2000;    
}


void rtcReadSec(){

  Wire.beginTransmission(RTC_I2C_ADDRESS); //Aufbau der Verbindung zur Adresse 0x68
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(RTC_I2C_ADDRESS, 1);
  sekunde    = bcdToDec(Wire.read() & 0x7f);
}


//Convertiert Dezimalzeichen in binäre Zeichen.

byte decToBcd(byte val){
  return ( (val/10*16) + (val%10) );
}


//Convertiert binäre Zeichen in Dezimal Zeichen.

byte bcdToDec(byte val){
  return ( (val/16*10) + (val%16) );
}

void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(RTC_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (0=Sunday, 6=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}


//Berechnet den Tag der Woche aus dem übergebenen Datumswerten.

  byte calcDayOfWeek(int jahr, byte monat, byte tag) {
  static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  jahr -= monat < 3;
  return ((jahr + jahr/4 - jahr/100 + jahr/400 + t[monat-1] + tag) % 7); 
}


int Schaltjahr(int year)   //   true = schaltjahr, false = keins
{
return ((year % 4) == 0) && ((year % 100) != 0 || (year % 400) == 0);
}


void updateTimegross() {

  rtcReadTime();
int stdE, stdZ, minZ, minE;
if (stunde < 10) {
   stdE = stunde;
   lcd.setCursor(0, 0);
   lcd.print("   ");
   lcd.setCursor(0, 1);
   lcd.print("   ");
     }  else  {
    stdE = stunde % 10; 
    stdZ = (stunde - stdE) / 10;
    printNumber( stdZ, 0);
   } 
    printNumber( stdE,3);
if (minute < 10) {
   minZ = 0;
   minE = minute;
  }  else  {
    minE = minute % 10; 
    minZ = (minute - minE) / 10;
   } 
    printNumber( minZ, 7);
    printNumber( minE, 10);
}


void updateTimeklein() {

  rtcReadTime();
  
  lcd.setCursor(0, 0);
  
  lcd.print(daysInWeek[wochentag]);
  lcd.print(" ");
  if (tag < 10) lcd.print(" ");
  lcd.print(tag);
  lcd.print(". ");
  lcd.print(monthInYear[monat]);
  lcd.print(" ");
  lcd.print (jahr);
  lcd.setCursor(0,1);
  lcd.print("  Zeit: "); 
  if (stunde<10) lcd.print(" "); 
  lcd.print(stunde);
  lcd.print(":");
  if(minute<10) lcd.print("0");
  lcd.print(minute);
  lcd.print(":");
  if(sekunde<10) lcd.print("0");
  lcd.print(sekunde);
}

void UhrstellenAnzeigen() {
   lcd.clear();
   lcd.setCursor(2,0);
   lcd.print (jahr);
   lcd.print ("-");
   lcd.print(monthInYear[monat]);
   lcd.print ("-");
    if (tag < 10) lcd.print("0");
   lcd.print(tag);
   lcd.setCursor(2,1);
   lcd.print(daysInWeek[wochentag]); 
   lcd.setCursor(7,1);
    if (stunde<10) lcd.print(" "); 
   lcd.print(stunde);
   lcd.print(":");
    if(minute<10) lcd.print("0");
   lcd.print(minute);  
  }

void Blinken() {
 lcd.setCursor(Col,Row);
if (blink == 6) blink = 0;
if (blink < 3) {
  if (stellen == 21) lcd.print(monthInYear[Temp]);
  else {
  if (stellen == 41 && Temp < 10) lcd.print(" ");
  if (stellen == 51 && Temp < 10) lcd.print("0"); 
  if (stellen == 31 && Temp < 10) lcd.print("0");
    lcd.print(Temp);
  }

}  else {
  if (stellen  == 11) lcd.print("  ");
  if (stellen  == 21) lcd.print(" ");
  lcd.print("  ");  
   }
}



void Wertsichern() {
if (stellen == 11)  jahr = Temp;
if (stellen == 21)  monat = Temp;
if (stellen == 31)  tag = Temp;
if (stellen == 41)  stunde = Temp;
if (stellen == 51)  minute = Temp;
}


void Uhrstellen() {

   if (stellen <10) {
  UhrstellenAnzeigen();
  if (button == KEYPAD_NONE) stellen = 10;
   }  else {
  if ((stellen & 1) == 0) {  
  switch (stellen) {  
case 10: 
  Min = 2000;
  Max = 2099;
  Col = 2;
  Row = 0;
  Temp = jahr;
  break;

case 20: 
  Min = 1;
  Max = 12;
  Col = 7;
  Row = 0;
  Temp = monat;
  break;

case 30: 
  Min = 1;
if (monat == 2) {
  if (Schaltjahr(jahr)) Max = 29;
  else Max = 28; 
  } else Max = daysInMonth[monat];  
  Col = 11;
  Row = 0;
  Temp = tag;
  break;

case 40: 
  Min = 0;
  Max = 23;
  Col = 7;
  Row = 1;
  Temp = stunde;
  break;

case 50: 
  Min = 0;
  Max = 59;
  Col = 10;
  Row = 1;
  Temp = minute;
  break;
 }
stellen++;
blink = 0;
} else {  // wird weiter ausgeführt bei stellen 11,21,31,41,51

blink++;
Blinken();

if (button == KEYPAD_UP) {
  if (button != Down) {
    if (Temp < Max) Temp++;
    else Temp = Min;
  if (stellen  < 40) {
    blink = calcDayOfWeek(jahr, monat, Temp);
    lcd.setCursor(2,1);
  lcd.print(daysInWeek[blink]); 
  }
  blink = 0;
  Down = KEYPAD_UP;
} else {
if (blink > 2) {
    if (Temp < Max) Temp++;
    else Temp = Min;
  blink = 0;
   }
  } 
}
   
if (button == KEYPAD_DOWN) {
  if (button != Down) {
    if (Temp > Min) Temp--;
    else Temp = Max;
  if (stellen < 40 ) {
    blink = calcDayOfWeek(jahr, monat, Temp);
   lcd.setCursor(2,1);
   lcd.print(daysInWeek[blink]);    
  }
  blink = 0;
  Down = KEYPAD_DOWN;
} else {
if (blink > 2) {
    if (Temp > Min) Temp--;
    else Temp = Max;
  blink = 0;
   }
  } 
}

if (button == KEYPAD_RIGHT) {
  if (button != Down) {
  blink = 0;
  Blinken();
  Wertsichern();
  if (stellen < 50) stellen += 9;
  else stellen = 10;
  Down = KEYPAD_RIGHT;
}  
}
if (button == KEYPAD_LEFT) {
  if (button != Down) {
  blink = 0;
  Blinken();
  Wertsichern();
  if (stellen > 15) stellen -= 11;
  else stellen = 50;
  Down = KEYPAD_LEFT;
}  
}

if (button == KEYPAD_SELECT) {
  if (button != Down) {
  Wertsichern();
  Down = KEYPAD_SELECT;
  byte dayOfWeek = calcDayOfWeek(jahr, monat, tag);  
  setDS3231time(0, minute, stunde, dayOfWeek, tag, monat, jahr - 2000);
  stellen = 0;
  blink = 0;
   lcd.clear();
     if (Display_kleingross == 0)  updateTimeklein();    
     else  updateTimegross();
 }
}

if (button == KEYPAD_NONE) Down = KEYPAD_NONE;
     } 
   }  
}

  



// the 8 arrays that form each segment of the custom numbers

byte bar1[8] = 
{
        B11000,
        B11100,
        B11100,
        B11100,
        B11100,
        B11100,
        B11100,
        B11000
};

byte bar2[8] =
{
        B00011,
        B00111,
        B00111,
        B00111,
        B00111,
        B00111,
        B00111,
        B00011
};

byte bar3[8] =
{
        B11111,
        B11111,
        B00000,
        B00000,
        B00000,
        B00000,
        B11111,
        B11111
};

byte bar4[8] =
{
        B11000,
        B11100,
        B00000,
        B00000,
        B00000,
        B00000,
        B11000,
        B11100
};

byte bar5[8] =
{
        B00011,
        B00111,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000
};

byte bar6[8] =
{
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B11111,
        B11111
};

byte bar7[8] =
{
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00111,
        B00011
};

byte bar8[8] =
{
        B11111,
        B11111,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000
};


void setup() {

  Serial.begin(115200);

  Wire.begin(); //Kommunikation über die Wire.h bibliothek beginnen  
  lcd.begin(16,2);            // sets the LCD's rows and colums:
  lcd.clear();

    // assignes each segment a write number

   lcd.createChar(1,bar1);
   lcd.createChar(2,bar2);
   lcd.createChar(3,bar3);
   lcd.createChar(4,bar4);
   lcd.createChar(5,bar5);
   lcd.createChar(6,bar6);
   lcd.createChar(7,bar7);
   lcd.createChar(8,bar8);

   analogWrite(backlightPin, 80);

   if (Display_kleingross == 1) updateTimegross();
   else updateTimeklein();     
     



}


void printNumber(int value, int col) {

  switch(value) {  

  case 0:
    {
  lcd.setCursor(col, 0); 
  lcd.write(2);  
  lcd.write(8); 
  lcd.write(1);
  lcd.setCursor(col, 1); 
  lcd.write(2);  
  lcd.write(6);  
  lcd.write(1);      
    }
    break;

  case 1:
{
  lcd.setCursor(col,0);
  lcd.write(32);
  lcd.write(1);
  lcd.write(32);
  lcd.setCursor(col,1);
  lcd.write(32);
  lcd.write(1);
  lcd.write(32);
}
    break;

  case 2:
{
  lcd.setCursor(col,0);
  lcd.write(5);
  lcd.write(3);
  lcd.write(1);
  lcd.setCursor(col, 1);
  lcd.write(2);
  lcd.write(6);
  lcd.write(6);
}
    break;

  case 3:
{
  lcd.setCursor(col,0);
  lcd.write(5);
  lcd.write(3);
  lcd.write(1);
  lcd.setCursor(col, 1);
  lcd.write(7);
  lcd.write(6);
  lcd.write(1); 
}
    break;

  case 4:
{
  lcd.setCursor(col,0);
  lcd.write(2);
  lcd.write(6);
  lcd.write(1);
  lcd.setCursor(col, 1);
  lcd.write(32);
  lcd.write(32);
  lcd.write(1);
}
    break;

  case 5:
{
  lcd.setCursor(col,0);
  lcd.write(2);
  lcd.write(3);
  lcd.write(4);
  lcd.setCursor(col, 1);
  lcd.write(7);
  lcd.write(6);
  lcd.write(1);
}
    break;

  case 6:
{
  lcd.setCursor(col,0);
  lcd.write(2);
  lcd.write(3);
  lcd.write(4);
  lcd.setCursor(col, 1);
  lcd.write(2);
  lcd.write(6);
  lcd.write(1);
}
    break;

  case 7:
{
  lcd.setCursor(col,0);
  lcd.write(2);
  lcd.write(8);
  lcd.write(1);
  lcd.setCursor(col, 1);
  lcd.write(32);
  lcd.write(32);
  lcd.write(1);
}
    break;

  case 8:
{
  lcd.setCursor(col, 0); 
  lcd.write(2);  
  lcd.write(3); 
  lcd.write(1);
  lcd.setCursor(col, 1); 
  lcd.write(2);  
  lcd.write(6);  
  lcd.write(1);
}
    break;
    
  case 9:
{
  lcd.setCursor(col, 0); 
  lcd.write(2);  
  lcd.write(3); 
  lcd.write(1);
  lcd.setCursor(col, 1); 
  lcd.write(7);  
  lcd.write(6);  
  lcd.write(1);
      }
    break;        
  }      
}  




void loop()  {
    
    Now = millis();
    if (Now - Last >= Intervall) {

    button = keys.button(); 
     if (stellen > 6) Uhrstellen();
     else {   
     if (button == KEYPAD_SELECT) {
     if (stellen < 7) stellen++;
      } // else stellen = 0;
     rtcReadSec();
    if (sekunde != oldseconds)
        {   
     if (sekunde == 0) {
      if (Display_kleingross == 1) updateTimegross();
      else updateTimeklein();     
     }
    blinkOn = 0;
    lcd.setCursor(14,1);
     if (sekunde < 10) lcd.print("0");
      lcd.print(sekunde);  
     oldseconds = sekunde;      
       }
 if (Display_kleingross == 1)  {     // wird ausgeführt wenn millis abgelaufen aber keine volle sekunde
     if (blinkOn == 4) {
     lcd.setCursor(6,0);
     lcd.print((char)165 );
     lcd.setCursor(6,1);
     lcd.print((char)165);
     }
     if (blinkOn == 2) {     
     lcd.setCursor(6,0);
     lcd.print(" ");
     lcd.setCursor(6,1);
     lcd.print(" "); 
      }  
     blinkOn++;
        } 
            
//  Serial.print("Button  :");
//  Serial.println(button);        

    if (button != Down) {
     if ((button > 0)  &&  (button < 5)) {     // Taste neu gedrückt Anzeige umschalten
        lcd.clear();
      if (Display_kleingross == 1)  {
      Display_kleingross = 0;
      updateTimeklein();    
      }  else {
      Display_kleingross = 1;
      updateTimegross();  
          }     
        }
    Down = button;    
       }
     }
     Last = Now;  // für millis Schleife
    }

//                                                 hier der normale code 

}
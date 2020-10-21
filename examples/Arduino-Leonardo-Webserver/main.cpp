// Arduino Leonardo ESP8266 STA Webserver


//    Arduino_Leonardo_Webserver_Jul2019_new V50
//    with Ringbuffer from WifiESP library
//    but now RingBufferGS
//    tm1638 library 
//    but now TM1638GS
//    relays are active low
//    with refresh

//    Arduino Leonardo mit LED Module an PCF8574
//    LED an bei output Low
//    I2C bus address is 0x20
//    an SDA und SCL

//    TM1638 LED&KEY pins
//    STB         Leonardo PIN 5
//    CLK         Leonardo PIN 6
//    DAT         Leonardo PIN 7

//  "<a href=\"?rel1+\"><button class=\"an1\">Relay 1 ON </button></a><a href=\"?rel2-\"><button class=\"an0\">Relay 1 OFF</button></a><br><br>";
//   is the complete string for one Line with 2 buttons....

#include <Arduino.h>
#include <TM1638GS.h>
#include <RingBufferGS.h>
#include "Wire.h"

#define debug true
Serial_ & Monitor = Serial;
HardwareSerial & esp8266 = Serial1;  // für Leonardo, micro, pro micro



const unsigned int MAX_INPUT = 60;
static char ipadress[16] = " "; 
int error, input_pos = 0, ip = -1, temp = 0, connID = -1, index = 0x40;
byte relay = 0, onoff = 0, favicon = 0, relayState = 0x00, keyOld = 0, keyNew = 0, keyPressTime = 0;
unsigned long starttime, prevTime;
int state = 0, waitTime = 0, x = 0, end = 0;
byte buttonOld = 0, buttonNew = 0, redstate=0, greenstate=0;
char displayBuffer[20] = "";
const char* pos;
int DisplayOn = 1, brightness = 2, temp1 = 0;


void initESP8266();
int sendData(const char command[20], const char answer[10], unsigned long duration);
int sendData1(int lenght);
int getResponse(const char answer[], unsigned long duration);
void clearSerialBuffer(void);
int processIncomingByte(void);
void closeConnection(int connID); 
void setRelay();
void sendHomepage(void);
void DoIt(void);
void processKey(void);

RingBufferGS buffer(10);
TM1638GS display(5,6,7);    // D5 D6 D7     Leonardo


void setup() {

Wire.begin();               // wake up I2C bus
                            // set I/O pins to outputs

  display.reset();
  display.set_Chars(0, "Start...");
  display.setColorLEDs(0x01, 0x80);
  
  Monitor.begin(115200);  // serial monitor
  esp8266.begin(115200); // ESPSerial ESP8266

relayState = 0;
setRelay();
  
  delay(3000);


  display.clear_digits();
  display.setColorLED(0, 0);

  clearSerialBuffer();
  initESP8266();
  starttime = millis();
}


void loop() {

  // put your main code here, to run repeatedly:
      if (esp8266.available () > 0)   { 
    connID = processIncomingByte();
  if (connID >= 0) {
  if (connID < 3)  {    // connected ID 0 - 2
    Monitor.print("\r\n   ConnID  = "); 
    Monitor.println(connID);    
  int temp =  processIncomingByte();   // get command
     Monitor.println(" read until \\r\\n\\r\\n = empty line ");
     int temp1 = getResponse("\r\n\r\n",100);
     if (temp1 != 0) {
        Monitor.println("no empty line found....");
        temp = 0;
            }
     if (temp > 9) {
      sprintf(displayBuffer, "Relay %d ", temp / 10);
      temp -= 10;   //  10 - 80  --> 0 - 70
      Monitor.print("before relayState  = ");
      Monitor.print(relayState, HEX);        
     if ((temp & 1 ) == 1) {
      bitSet(relayState, temp / 10);
      strcat(displayBuffer, "on");
      }  else   {
      bitClear(relayState, temp / 10);
      strcat(displayBuffer, "off");
      }
     setRelay();    // relay set/clear
     state = 1;
    temp = 6;
  }
     if (temp > 5) sendHomepage();        // temp = 5 /favicon
     closeConnection(connID);
     Monitor.println("connection closed!");
     Monitor.println("................................waiting for next connection"); 
   
    }    
  }  // else  Monitor.println("timeout, no client!!!  ");
}


 if (millis() - starttime > 300000)   // check WIFI connection every 300 Sec.
     {
      starttime = millis();
      Monitor.println("                                                      checking connection every 5 minutes..."); 
      int error = sendData("AT+CIPSTATUS","STATUS:",1000); 
      // int error = sendData("AT+CIPSTATUS","OK\r\n",1000); 
      char inByte = '0'; 
      unsigned long start;
      start = millis();
      while (millis() - start <= 50)   // 50ms
      {
      if (esp8266.available () > 0)   {    
      inByte = (esp8266.read());
      break;
        }
      }
      Monitor.print(inByte);
      error = getResponse("OK\r\n",1000);
      if (error == 0) {
      if ((inByte == '2') || (inByte == '4'))  {
       Monitor.println("connection  OK");
       strcpy(displayBuffer, "still connected");
       state = 1; 
      } else error++;
      }
      if (error != 0) {
      Monitor.print(inByte);
      Monitor.print("connection to AP lost....");
      Monitor.print("retry to connect....");
      initESP8266();
       }
      }


   if (millis() - prevTime > 50)  {   // each 50ms tm1638 check
    if (state > 0)   DoIt();
    processKey();
    prevTime = millis();    // timer restart
   }
      //   put more activity here...

}     //  loop end-----------------------------------------------------



void DoIt() {

switch (state)  {

case 1:     // init
x = 0;
end = 0;
while (displayBuffer[x] != '\0')
x++;
if (x > 7) {
  end = x - 7;
  x = 0;
}
  else end = x;
display.set_Chars(0, displayBuffer);
waitTime = 20;     // 20 * 50ms = 1 sec.
state++;
break;

case 2:     // wait
if (waitTime > 0) waitTime--;
else state++; 
break;

case 3:     //  shift left
if (waitTime > 0) waitTime--; 
else {
waitTime = 5;
 
    if (x == end) {
    waitTime = 40;     // 40 * 50ms = 1 Sec.
    state++; 
    } else {
    pos = displayBuffer + x;
    display.set_Chars(0, pos);
    x++;
    }
}
break;

case 4:     // wait 
if (waitTime > 0) waitTime--;
else state++; 
break;

case 5:     //  finished
      display.clear_digits();
      state = 0;
break;

   }
}



void processKey() {
keyNew = display.readButtons();
if (keyNew == 0) {
   if (keyPressTime > 0 && keyPressTime < 7) {  // short keypress
  int temp = keyOld, temp1 = 1;
  do {
    temp = temp >> 1;
    if (temp == 0) break;
    temp1++;
     }  while (temp1 < 8);
sprintf(displayBuffer, "Relay %d ", temp1);
if (bitRead(relayState, temp1-1) == 1)
{  strcat(displayBuffer, "off");
   bitClear(relayState, temp1-1);
}  else  { 
strcat(displayBuffer, "on");
   bitSet(relayState, temp1-1);
}
state = 1;
//   relayState ^= keyOld;    // XOR toggle Bit
   setRelay();
     }
  keyOld = 0;       
  keyPressTime = 0;
} else {          // key s(till) pressed
if (keyPressTime == 6) {
strcpy(displayBuffer, "IP=");
strcat(displayBuffer, ipadress);
state = 1;
}
keyPressTime++;
keyOld = keyNew;
  }   
}



void initESP8266() {
error = sendData("AT","OK\r\n",5000);
if (error == 0) {
display.set_Chars(0, "ESP-8266");  
Monitor.println("ESP-8266 found...");
if (error == 0) error = sendData("AT+RST","ready\r\n",1000);
if (error == 0) error = sendData("ATE0","OK\r\n",1000);
if (error == 0) error = sendData("AT+CWAUTOCONN=?","OK\r\n",1000);
error = sendData("AT+CWMODE_CUR=1","OK\r\n",1000);
if (error == 0) error = sendData("AT+CWQAP","OK\r\n",1000);
if (error == 0) display.set_Chars(0, "connect1");  
if (error == 0) error = sendData("AT+CWJAP_CUR=\"zuhause\",\"schmitt.obw@web.de\"","OK\r\n",10000);
if (error != 0) { 
error = sendData("AT+RST","ready\r\n",1000);
if (error == 0) error = sendData("ATE0","OK\r\n",1000);
if (error == 0) error = sendData("AT+CWAUTOCONN=?","OK\r\n",1000);
error = sendData("AT+CWMODE_CUR=1","OK\r\n",1000);
if (error == 0) error = sendData("AT+CWQAP","OK\r\n",1000);
if (error == 0) display.set_Chars(0, "connect2");  
error = sendData("AT+CWJAP_CUR=\"Obernburg1\",\"Sabine.Obernburg\"","OK\r\n",10000);
}
if (error == 0 ) {
ip = 0;
error = sendData("AT+CIFSR","STAIP,\"",1000);
if (error == 0) {
     do {
     if (esp8266.available () > 0)   {    
      ipadress[ip] = (esp8266.read());
         ip++; 
         } 
       }  while (ipadress[ip-1] != 34);   // read until "
     ipadress[ip-1] = '\0';              
Monitor.print("ip adress: ");
Monitor.println(ipadress); 
strcpy(displayBuffer, "IP=");
strcat(displayBuffer, ipadress);
state = 1;
error = getResponse("OK\r\n",1000);
if (error == 0) error = sendData("AT+CIPMUX=1","OK\r\n",1000);
if (error == 0) error = sendData("AT+CIPSERVER=1,80","OK\r\n",1000);
if (error == 0) { 
  Monitor.println("--------------------   server started!  --------------------");
  Monitor.println("");
  }
}
if (error) {
Monitor.println("");
Monitor.println("--------------------   Server not started!!!!!!  error!!!!  --------------------");
Monitor.println("");
}
} 
else {
  Monitor.println("no connection to router!");
  display.set_Chars(0, "Error...");  
}
} else {
  display.set_Chars(0, "-No ESP-");
  Monitor.println("no ESP-8266 found!");
  }
}
  

int sendData1(int lenght)
   {
Monitor.print("lenght = ");
Monitor.println(lenght);
        char command[20] = "AT+CIPSEND=";
        char buffer[8];
        sprintf(buffer, "%d,%d", connID, lenght);
        strcat(command, buffer);      
int error = sendData(command, ">", 1000);
     return error;
      }
    

int sendData( const char command[30], const char answer[20], unsigned long duration)  {

      Monitor.print("Send to 8266: ");
      Monitor.println(command); 
      Monitor.print("expected  answer: ");
      int l = 0;
      while (answer[l] != 0) { 
      if (answer[l] == '\r') Monitor.print("\\r");
      if (answer[l] == '\n') Monitor.print("\\n");
      if (answer[l] > 13) Monitor.print(answer[l]);  // 13 = 0x0d = \n
      l++;
      }
      Monitor.println("");
      esp8266.println(command); // send command to the ESP8266
   
   return(getResponse(answer, duration));
}


int getResponse(const char answer[], unsigned long duration)   {

     unsigned long start;
     buffer.init();      
     Monitor.println("waiting for responce "); 
     start = millis();
  while (millis() - start <= duration) 
     {
      if (esp8266.available () > 0)   {    
      const byte inByte = (esp8266.read());
      if (inByte == '\r') Monitor.print("\\r");
      if (inByte == '\n') Monitor.print("\\n");
      if (inByte > 13) Monitor.write(inByte);  
      buffer.push(inByte);               // push into ring buffer
      if (buffer.endsWith(answer)) 
        {
         Monitor.println(" <-- correct responce found!!"); 
         return(0);
      }   
   }   //   no Serial data

 }   // ends with timeout, go on with while....
     Monitor.println(" correct responce not found!!"); 
     return(1);     
}    // finished response ESP8266


void clearSerialBuffer(void) {
       delay(5);
       while (esp8266.available() > 0 ) {
       char c = esp8266.read();
       Monitor.write(c); 
       }
}



int processIncomingByte() {
     int state = 0, temp = 0;
     unsigned long start;
     buffer.init();      
     start = millis();
  while (millis() - start <= 50)   // 50ms
     {
      if (esp8266.available () > 0)   {    
  const byte inByte = (esp8266.read());

   if (inByte == '\r') Monitor.print("\\r");
   if (inByte == '\n') Monitor.print("\\n");
   if (inByte > 15) Monitor.write(inByte);
   
   buffer.push(inByte);               // push it to the ring buffer
 
      if (state == 1) return(inByte - 48);   // connID 0-2
      if (buffer.endsWith("+IPD,")) state = 1; 
      
      if (buffer.endsWith("/favicon")) return(5);

      if (buffer.endsWith("GET / "))  {
        temp1 = 0;
        return(6);
      }

      if (state == 3) {
        if (temp1 == temp) return(6);   // refresh
        else {
        temp1 = temp;
        temp = (temp & 0x0f) * 10;  
        index ^= 0x20;    // XOR toggle Bit
        if (inByte == '-') return(temp);
        if (inByte == '+') return(++temp);
        //return(6);
        }
      }
      
       if (state == 2) {
        temp = (inByte);
        state++;
      }
       if (buffer.endsWith("/?rel")) state = 2;
      }                  // end serial avail
   }                     // end time while
   return(-1);           // timeout
}
   

void closeConnection(int connID)
  {
   char command[16];
   sprintf(command,"AT+CIPCLOSE=%d",connID);
   error = sendData(command,"OK\r\n",1000); 
   if (error == 0) starttime = millis();  // reset counter for state check every 5 minutes
    else {
    Monitor.println("");
    Monitor.print("connection close error");
       }  
  }

void setRelay()    // relayState
  {
    byte temp = ~relayState;     // my relays are 0 active  
    Wire.beginTransmission(0x20);
    Wire.write(temp); // port A
    Wire.endTransmission();
    Monitor.print("     new relayState = ");
    Monitor.println(relayState, HEX);
    display.setColorLEDs(0, relayState);      
    }


void sendHomepage()
   {
clearSerialBuffer();
Monitor.println("...............sending homepage..........");
            
   //error = sendData1(1099); 
   error = sendData1(1139);
   if (error == 0) {
    Monitor.println("sending Header.....");    
    esp8266.print(F("HTTP/1.1 200 OK\r\n Server: Arduino\r\n Content-Type:text/html\r\n Connection:close\r\n\r\n"));
    esp8266.print(F("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"));
    esp8266.print(F("<meta http-equiv=\"refresh\" content=\"10\">"));  // geö
    esp8266.print(F("<style>* {color:#ff0} h1 {background:#000} button{width:130px;height:40px; font-size:16px; border:none;"));
    esp8266.print(F("cursor:pointer;border-radius:5px; margin-right: 15px;} .an1{background:#4CAF50; color:white;}"));
    esp8266.print(F(".an0{background:silver;color:black;} #google{color:blue;font-size:26px;}</style></head>"));
    esp8266.print(F("<body><center><br><br><h1>ESP8266 WiFi Web Server V50 </h1><br><br>"));
  Monitor.println("sending Buttons....");
int i = 1; 
do {            
esp8266.print(F("<a href=\"?rel"));
//esp8266.print(i);
esp8266.write(i + index);
if (bitRead(relayState, i - 1) == 1) esp8266.print('-');
     else esp8266.print('+');
esp8266.print(F("\"><button class=\"an"));
if (bitRead(relayState, i - 1) == 1) esp8266.print('1');
     else esp8266.print('0');
esp8266.print(F("\">Relay "));
esp8266.print(i);
if (bitRead(relayState, i - 1) == 1) esp8266.print(" ON ");
     else esp8266.print(" OFF");
esp8266.print(F("</button></a><a href=\"?rel"));
 i++;
// esp8266.print(i);
esp8266.write(i + index);
if (bitRead(relayState, i - 1) == 1) esp8266.print('-');
     else esp8266.print('+');
esp8266.print(F("\"><button class=\"an"));
if (bitRead(relayState, i - 1) == 1) esp8266.print('1');
     else esp8266.print('0');
esp8266.print(F("\">Relay "));
esp8266.print(i);
if (bitRead(relayState, i - 1) == 1) esp8266.print(" ON ");
     else esp8266.print(" OFF");
esp8266.print(F("</button></a><br><br>")); 
  i++; 
} while (i < 8);
    Monitor.println("sending bottom.....");
    esp8266.print(F("<br><br><hr><a id=\"google\" href=\"http://google.de\">Google.de</a>\r\n\r\n"));
    error = getResponse("SEND OK\r\n",5000);
if (error == 0) Monitor.println("homepage sent");     
         
 }  
}
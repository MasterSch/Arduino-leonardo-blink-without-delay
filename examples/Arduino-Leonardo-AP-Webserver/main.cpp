// Arduino Leonardo ESP8266 AP Webserver

#include <Arduino.h>

//    Arduino_Leonardo_Webserver_Okt2018_new V20
//    with Ringbuffer from WifiESP library
//    https://github.com/bportaluri/WiFiEsp
//    in folder /src/utility
//    tm1638 library
//    https://github.com/int2str/TM1638 aber geändert
//    relays are active low

//    TM1638 LED&KEY pins
//  STB         Leonardo PIN 8
//  CLK         Leonardo PIN 9
//  DAT         Leonardo PIN 10



#include <tm1638.h>
#include <RingBuffer.h>
RingBuffer buffer(16);

#define debug true

Serial_ & Monitor = Serial;
HardwareSerial & esp8266 = Serial1;  // für Leonardo, micro, pro micro


static char Line[135] = "<a href=\"?rel1+\"><button class=\"on1\">Relay 1 ON </button></a><a href=\"?rel2-\"><button class=\"on0\">Relay 1 OFF</button></a><br><br>";
//   is the complete string for one Line with 2 buttons....
const unsigned int MAX_INPUT = 60;
static char ipadress[16] = " "; 
int error, input_pos = 0, ip = -1, temp = 0, connID = -1;
byte relay = 0, onoff = 0, favicon = 0, relayState = 0x00, keyOld = 0, keyNew = 0, keyPressTime = 0;
unsigned long starttime, prevTime;
int state = 0, waitTime = 0, x = 0, end = 0;
byte buttonOld = 0, buttonNew = 0, redstate=0, greenstate=0;
char displayBuffer[20] = "";
const char* pos;
int DisplayOn = 1, brightness = 2;


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

TM1638 display;

void setup() {
  // put your setup code here, to run once:
//  display.setupDisplay(DisplayOn, brightness);
  display.setChars("Start...");
  display.setLEDs(0x01, 0x80);
  
  Monitor.begin(115200);  // serial monitor
  esp8266.begin(115200); // ESPSerial ESP8266
pinMode(A0, OUTPUT);
pinMode(A1, OUTPUT);
pinMode(A2, OUTPUT);
pinMode(A3, OUTPUT);
pinMode(A4, OUTPUT);
pinMode(A5, OUTPUT);
pinMode(2, OUTPUT);
pinMode(3, OUTPUT);

relayState = 0;
setRelay();
  
  delay(3000);


  display.clear();
  display.clearLEDs();

  clearSerialBuffer();
  initESP8266();
  starttime = millis();
}


void loop() {

  // put your main code here, to run repeatedly:
      if (esp8266.available () > 0)   { 
          Monitor.println(" Daten erkannt..................");    
    connID = processIncomingByte();
  if ((connID >= 0) && (connID < 3))
        {     // connected ID 0 - 2
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
      sprintf(displayBuffer, "Relay %d ", temp/10);
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
     Monitor.println("waiting for next connection..."); 
   
       
  }  // else  Monitor.println("timeout, no client!!!  ");
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
display.setChars(displayBuffer, 0);
waitTime = 10;     // 20 * 50ms = 1 sec.
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
    display.setChars(pos, 0);
    x++;
    }
}
break;

case 4:     // wait 
if (waitTime > 0) waitTime--;
else state++; 
break;

case 5:     //  finished
      display.clear();
      state = 0;
break;

   }
}

void processKey() {
keyNew = display.getButtons();
if (keyNew == 0) {
   if ((keyPressTime > 0) && (keyPressTime < 7)) {  // short keypress
   relayState ^= keyOld;    // XOR toggle Bit
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
display.setChars("ESP-8266");  
Monitor.println("ESP-8266 found...");
}
if (error == 0) error = sendData("AT+RST","ready\r\n",1000);   // reset
if (error == 0) error = sendData("ATE0","OK\r\n",1000);        // echo off
if (error == 0) error = sendData("AT+CWAUTOCONN=?","OK\r\n",1000);   // autoconnection off
error = sendData("AT+CWMODE_CUR=2","OK\r\n",1000);       //   soft AP mode on
if (error == 0) error = sendData("AT+CWSAP_CUR=\"ESP8266AP\",\"abcd1234\",6,3","OK\r\n",1000);
if (error == 0) error = sendData("AT+CIPMUX=1","OK\r\n",1000);
if (error == 0) error = sendData("AT+CIPSERVER=1,80","OK\r\n",1000);
if (error == 0) error = sendData("AT+CIPAP_CUR?","OK\r\n",1000);

ip = 0;
error = sendData("AT+CIPAP_CUR?","ip:\"",1000);
if (error == 0) {
     do {
     if (Serial1.available () > 0)   {    
      ipadress[ip] = (Serial1.read());
         ip++; 
         } 
       }  while (ipadress[ip-1] != 34);   // bis zum "
     ipadress[ip-1] = '\0';              
strcpy(displayBuffer, "IP=");
strcat(displayBuffer, ipadress);
state = 1;
Monitor.println(displayBuffer);
clearSerialBuffer();
  }
if (error == 0) {

Monitor.println("--------------------   server started!  --------------------");
Monitor.println("");
  }
if (error) {
display.setChars("ESP-Err");
Monitor.println("ESP-8266 Error");
Monitor.println("");
Monitor.println("--------------------   Server not started!!!!!!  error!!!!  --------------------");
Monitor.println(""); 
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
int error = sendData(command, "OK\r\n>", 1000);
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
     Monitor.print("waiting for responce "); 
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
     clearSerialBuffer();
     return(1);     
}    // finished response ESP8266



void clearSerialBuffer(void) {
       Monitor.println("clear serial buffer....");
       delay(10);
       while (esp8266.available() > 0 ) {
       char c = esp8266.read();
       Monitor.write(c);
       delay(1); 
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
       if (buffer.endsWith("GET / "))  return(6);
       if (state == 3) {
        if (inByte == '-') return(temp);
        if (inByte == '+') return(++temp);
      }
      
      if (state == 2) {
        temp = (inByte - 48) * 10;    //   Relay 10 -80
        state++;
      }
      
      if (buffer.endsWith("GET /?rel")) state = 2;
      }                  // end serial avail
   }                     // end time while
   return(-1);           // timeout
}
   


void closeConnection(int connID)
  {
   char command[16];
   sprintf(command,"AT+CIPCLOSE=%d",connID);
   error = sendData(command,"CLOSED\r\n\r\nOK\r\n",1000); 
   if (error !=0) {
    Monitor.println("");
    Monitor.print("connection close error");
    clearSerialBuffer();   
       }  
  }

void setRelay()    // relayState
  {
   byte temp = ~relayState;     // my relays are 0 active  
digitalWrite(A0, bitRead(temp,0));
digitalWrite(A1, bitRead(temp,1));
digitalWrite(A2, bitRead(temp,2));
digitalWrite(A3, bitRead(temp,3));
digitalWrite(A4, bitRead(temp,4));
digitalWrite(A5, bitRead(temp,5));
digitalWrite(2, bitRead(temp,6));
digitalWrite(3, bitRead(temp,7));
    Monitor.print("     new relayState = ");
    Monitor.println(relayState, HEX);
    display.setLEDs(0, relayState);      
    }


void sendHomepage()
   {
clearSerialBuffer();
Monitor.println("...............sending homepage..........");
            
error = sendData1(471); 
   if (error == 0) {
    Monitor.println("sending Header.....");    

esp8266.print(F("HTTP/1.1 200 OK\r\n Server: Arduino\r\n Content-Type:text/html\r\n Connection:close\r\n\r\n"));    
esp8266.print(F("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"));
esp8266.print(F("<style>* {color:#000} h1 {background:#cccccc} "));
esp8266.print(F("button{width:130px;height:40px; font-size:16px; border:none;cursor:pointer;border-radius:5px; margin-right: 15px;}"));
esp8266.print(F(".on1{background:#4CAF50; color:white;}.on0{background:silver;color:black;}"));
esp8266.print(F("</style></head><body><center><br><h1>ESP8266 WiFi Web Server AP</h1><br><br>"));

    error = getResponse("bytes\r\n",5000);
    if (error == 0) error = getResponse("SEND OK\r\n",5000);
    if (error == 0) Monitor.println("Header sent");     

error = sendData1(520);
   if (error == 0) {
  Monitor.println("sending Buttons....");

int i = 0; 
do {
Line[13] = i+49;
Line[43] = i+49;
if (bitRead(relayState, i) == 1) 
{
Line[14] = '-';
Line[34] = '1';
Line[46] = 'N';
Line[47] = ' ';
}
else
{
Line[14] = '+';
Line[34] = '0';
Line[46] = 'F';
Line[47] = 'F';
}
i++;
 Line[74] = i+49;
Line[104] = i+49;
if (bitRead(relayState, i) == 1) 
{
Line[75] = '-';
Line[95] = '1';
Line[107] = 'N';
Line[108] = ' ';
}
else
{
Line[75] = '+';
Line[95] = '0';
Line[107] = 'F';
Line[108] = 'F';
}            
    Monitor.println(Line); 
    esp8266.print(Line);

i++;
  }  while (i<7);

    error = getResponse("bytes\r\n",20000);
    if (error == 0) error = getResponse("SEND OK\r\n",500);
    if (error == 0) Monitor.println("Buttons sent");     

   if (error == 0) {
    error = sendData1(45); 
    Monitor.println("sending bottom.....");
    esp8266.print(F("<br><br><h1>Guenter Schmitt V3.0</h1><br><br>"));

    error = getResponse("bytes\r\n",5000);
    if (error == 0) error = getResponse("SEND OK\r\n",5000);
    if (error == 0) Monitor.println("homepage sent");     
    }
   }
 }
    if (error != 0) clearSerialBuffer();    
  
}
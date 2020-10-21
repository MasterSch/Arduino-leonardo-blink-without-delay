#include <Arduino.h>

//    Arduino_Leonardo_Webserver_Feb2018_new V12
//    with Ringbuffer from WifiESP library
//    https://github.com/bportaluri/WiFiEsp
//    in folder /src/utility

#include <RingBuffer.h>
RingBuffer buffer(10);

#define debug true

const unsigned int MAX_INPUT = 60;
static char ipadress[16] = " "; 
static char Line[135] = "<a href=\"?rel1+\"><button class=\"an1\">Relay 1 ON </button></a><a href=\"?rel2-\"><button class=\"an0\">Relay 1 OFF</button></a><br><br>";
int error, input_pos = 0, ip = -1, temp = 0, connID = -1;
byte relay = 0, onoff = 0, favicon = 0, relayState = 0x00;
unsigned long starttime;

void initESP8266();
int sendData(const char command[20], const char answer[10], unsigned long duration);
int sendData1(int lenght);
int getResponse(const char answer[], unsigned long duration);
void clearSerialBuffer(void);
int processIncomingByte(void);
void closeConnection(int connID); 
void processRelay(int);
void sendHomepage(void);


void setup() {
  // put your setup code here, to run once:

  
  Serial.begin(115200);  // serial monitor
  Serial1.begin(115200); // serial1 ESP8266
  delay(3000);
  clearSerialBuffer();
  initESP8266();
  starttime = millis();
}


void loop() {
  // put your main code here, to run repeatedly:

    connID = processIncomingByte();
  if (connID >= 0) {
  if (connID < 3)  {    // connected ID 0 - 2
    Serial.print("\r\n   ConnID  = "); 
    Serial.println(connID);    
  int temp =  processIncomingByte();   // get command
     Serial.println("read until empty line ");
     int temp1 = getResponse("\r\n\r\n",100);
     if (temp1 == 1) {
     Serial.println("");
     Serial.println("empty line found...."); 
     }  else      Serial.print("no empty line found....");
  if (temp > 9) processRelay(temp);    // relay set/clear
  if (temp > 5) sendHomepage();        // temp = 5 /favicon
  closeConnection(connID);
     Serial.println("Homepage sent, connection closed!");
     Serial.println("waiting...."); 
   
    }    
  }  // else  Serial.println("timeout, no client!!!  ");



 if (millis() - starttime > 60000)   // check WIFI connection every 60 Sec.
     {
      Serial.println(" check connection..."); 
    int error = sendData("AT+CIPSTATUS","STATUS:",500); 
  if (error != 1) {
    Serial.println("");
   Serial.println("no answer from ESP8266");
   //        init ESP8266
  }
     else Serial.println("  OK...");

   byte inByte = 0;
   do {
      if (Serial1.available () > 0)   {    
      inByte = (Serial1.read());
      }
    }  while (inByte == 0);   
    error = getResponse("OK\r\n",100);
    if (inByte == 53)
    {
      Serial.println("");
      Serial.print("connection to AP lost....");
      Serial.print("retry to connect....");
      initESP8266();
    }    
   starttime = millis();
   }    
       //   put more activity here...
     
}


void initESP8266() {
error = sendData("AT","OK\r\n",5000);
if (error == 0) {
Serial.println("ESP-8266 found...");
if (error == 0) error = sendData("AT+RST","ready\r\n",1000);
if (error == 0) error = sendData("ATE0","OK\r\n",1000);
if (error == 0) error = sendData("AT+CWAUTOCONN=0","OK\r\n",1000);
if (error == 0) error = sendData("AT+CWMODE_CUR=1","OK\r\n",1000);
if (error == 0) error = sendData("AT+CWQAP","OK\r\n",1000);
if (error == 0) error = sendData("AT+CWJAP_CUR=\"Obernburg1\",\"Sabine.Obernburg\"","OK\r\n",10000);
//if (error == 0) error = (sendData("AT+CWJAP_CUR=\"zuhause\",\"schmitt.obw@web.de\"","OK\r\n",10000);
if (error == 0 ) {
ip = 0;
error = sendData("AT+CIFSR","STAIP,\"",1000);
if (error == 0) {
     do {
     if (Serial1.available () > 0)   {    
      ipadress[ip] = (Serial1.read());
         ip++; 
         } 
       }  while (ipadress[ip-1] != 34);   // bis zum "
     ipadress[ip-1] = '\0';              

Serial.print("ip adress: ");
Serial.println(ipadress); 
error = getResponse("OK\r\n",1000);
if (error == 0) error = sendData("AT+CIPMUX=1","OK\r\n",1000);
if (error == 0) error = sendData("AT+CIPSERVER=1,80","OK\r\n",1000);
if (error == 0) Serial.println("--------------------   server started!  --------------------");
if (error == 0) Serial.println(""); 
}
if (error) {
Serial.println("");
Serial.println("--------------------   Server nicht gestartet!!!!!!  Fehler!!!!  --------------------");
Serial.println("");
}
} 
else Serial.println("no connection to router!");
} else 
Serial.println("no ESP-8266 found!");
}
  

int sendData1(int lenght)
   {
Serial.print("lenght = ");
Serial.println(lenght);
        char command[20] = "AT+CIPSEND=";
        char buffer[8];
        sprintf(buffer, "%d,%d", connID, lenght);
        strcat(command, buffer);      
int error = sendData(command, ">", 500);
     return error;
      }
    

int sendData( const char command[20], const char answer[10], unsigned long duration)  {

      Serial.print("Send to 8266: ");
      Serial.print(command); 
      Serial.print("\\r\\n      expected answer: ");
      int l = 0;
      while (answer[l] != 0) { 
      if (answer[l] == '\r') Serial.print("\\r");
      if (answer[l] == '\n') Serial.println("\\n");
      Serial.print(answer[l]);  
      l++;
      }
      Serial1.print(command); // send command to the Serial1
      Serial1.print("\r\n");
      delay(1);
      // sent.......
   
   return(getResponse(answer, duration));
}


int getResponse(const char answer[], unsigned long duration)   {

     unsigned long start;
     buffer.init();      
     Serial.println(" responce start !!"); 
     start = millis();
  while (millis() - start <= duration) 
     {
      if (Serial1.available () > 0)   {    
      const byte inByte = (Serial1.read());
      if (inByte == '\r') Serial.print("\\r");
      if (inByte == '\n') Serial.println("\\n");
      if (inByte > 15) Serial.write(inByte);  
      buffer.push(inByte);               // push it to the ring buffer
      if (buffer.endsWith(answer)) 
        {
         Serial.println(" responce found!!"); 
         return(0);
      }   
   }   //   no Serial data

 }   // ends with timeout, go on with while....
     Serial.println(" responce timeout!!"); 
     return(1);     
}    // finished response ESP8266



void clearSerialBuffer(void) {
       delay(5);
       while (Serial1.available() > 0 ) {
       char c = Serial1.read();
       Serial.write(c); 
       }
}





int processIncomingByte() {
     int state = 0, temp = 0;
     unsigned long start;
     buffer.init();      
     start = millis();
  while (millis() - start <= 50)   // 50ms
     {
      if (Serial1.available () > 0)   {    
  const byte inByte = (Serial1.read());

   if (inByte == '\r') Serial.print("\\r");
   if (inByte == '\n') Serial.print("\\n");
   if (inByte > 15) Serial.write(inByte);
   
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
   error = sendData(command,"OK\r\n",1000); 
   if (error != 0) {
    Serial.println("");
   Serial.print("connection close error = ");
   Serial.println(error);
   }
  }

void processRelay(int temp)
  {
    temp -= 10;
    Serial.print("before relayState  = ");
    Serial.print(relayState, HEX);        
    if ((temp & 1 ) == 1) bitSet(relayState, temp / 10);
        else            bitClear(relayState, temp / 10);
    Serial.print("     new relayState = ");
    Serial.println(relayState, HEX);
    }


void sendHomepage()
   {

clearSerialBuffer();

Serial.println("               send    homepage..........");
            
error = sendData1(81); 
   if (error != 0) {
Serial.print("errorcode = ");
Serial.println(error);
   }
    Serial.println(" send header.....");    
    Serial1.print(F("HTTP/1.1 200 OK\r\n Server: Arduino\r\n Content-Type:text/html\r\n Connection:close\r\n\r\n"));
error = getResponse("SEND OK\r\n",100);
Serial.print("errorcode = ");
Serial.println(error);
if (error == 1) Serial.println("header sent");

error = sendData1(169); 
   if (error != 1){
Serial.print("errorcode = ");
Serial.println(error);
   }
    Serial.println("send title1.....");   
    Serial1.print(F("<html><head><style>* {color:#ff0} h1 {background:#000} button{width:130px;height:40px; font-size:16px; border:none;cursor:pointer;border-radius:5px; margin-right: 15px;}"));
error = getResponse("SEND OK\r\n",100);

if (error == 1) Serial.println("title1   sent");

error = sendData1(192); 
   if (error != 1){
Serial.print("errorcode = ");
Serial.println(error);
   }
    Serial.println("send title2.....");
    delay(10);
    Serial1.print(F(".an1{background:#4CAF50; color:white;}.an0{background:silver;color:black;} #google{color:blue;font-size:26px;}</style></head><body><center><br><br><h1>ESP8266 WiFi Web Server V1.0</h1><br><br>"));
error = getResponse("SEND OK\r\n",100);
Serial.print("errorcode = ");
Serial.println(error);
if (error == 1) Serial.println("title2   sent");

int i = 0; 
do {

error = sendData1(130);
   if (error != 1){
Serial.print("errorcode = ");
Serial.println(error);
   }
    Serial.print("     send Line.....relayState = ");
    Serial.println(relayState);
  
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
i++;

    Serial1.print(Line);
    Serial.print(Line);
error = getResponse("SEND OK\r\n",500);
Serial.print("errorcode = ");
Serial.println(error);
if (error == 1) Serial.println("Line sent");

} while (i < 7);

error = sendData1(68); 
   if (error != 1){
Serial.print("errorcode = ");
Serial.println(error);
   }
    Serial.println("send bottom.....");
    Serial1.print(F("<br><br><hr><a id=\"google\" href=\"http://google.de\">Google.de</a>\r\n\r\n"));
error = getResponse("SEND OK\r\n",100);
Serial.print("errorcode = ");
Serial.println(error);
if (error == 1) Serial.println("bottom   sent");


    }    


/*
 WiFiEsp example: WebServerLed
 
 A simple web server that lets you turn on and of an LED via a web page.
 This sketch will print the IP address of your ESP8266 module (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to turn on and off the LED on pin 13.

 For more details see: http://yaab-arduino.blogspot.com/p/wifiesp.html
*/

#include "WiFiEsp.h"
#include <TM1638GS.h>

#define debug true    // true means debug messages on serial monitor

//Serial_ & Monitor = Serial;
//HardwareSerial & esp8266 = Serial1;  // für Leonardo, micro, pro micro
#define Monitor Serial
#define esp8266 Serial1

// I/O pins on the Arduino connected to strobe, clock, data
// (power should go to 3.3v and GND)
//TM1638GS display(14,12,13);   // D5 D6 D7     WeMos D1R23
TM1638GS display(5,6,7);        // D5 D6 D7     Leonardo


void printWifiStatus(void);
void checkServer(void);
void processKey(void);
void processDisplay(void);
void setRelay();


char ssid[] = "zuhause";            // your network SSID (name)
char pass[] = "schmitt.obw@web.de";        // your network password
int status = WL_IDLE_STATUS;
int keyOld = 0, keyNew = 0, keyPressTime = 0;
int waitTime = 0, state = 0, x = 0, end = 0;
unsigned long prevTime = millis();

static char Line[135] = "<a href=\"?rel1+\"><button class=\"on1\">Relay 1 ON </button></a><a href=\"?rel2-\"><button class=\"on0\">Relay 1 OFF</button></a><br><br>";
char displayBuffer[20] = " StArt  ";
const char* pos = displayBuffer;
int relay[] = {18, 19, 20, 21, 22, 23, 2, 3};
int relayState = 0;
WiFiEspServer server(80);

// use a ring buffer to increase speed and reduce memory allocation
RingBuffer buf(12);

void setup()
{
  display.reset();
  display.setBrightness(2);
  display.displayText(pos);

  Monitor.begin(115200);   // initialize serial for debugging
  esp8266.begin(115200);    // initialize serial for ESP module
  WiFi.init(&esp8266);    // initialize ESP module

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Monitor.println("WiFi shield not present");
    display.displayText("No ESP..");
    // don't continue
    while (true);
  }
    display.displayText(pos);

  // attempt to connect to WiFi network
  while (status != WL_CONNECTED) {
    Monitor.print("Attempting to connect to WPA SSID: ");
    Monitor.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Monitor.println("You're connected to the network");
  printWifiStatus();
  Monitor.println("Start server.");
  strcat(displayBuffer, "connect.");
  display.displayText(pos);

  // start the web server on port 80
  server.begin();
}


void loop()
{

checkServer();

  if (millis() - prevTime > 50)  {   // each 50ms tm1638 check
    if (state > 0)   processDisplay();
       processKey();
       prevTime = millis();           // timer restart
   }
      //   put more activity here...

}     //  loop end-----------------------------------------------------


void checkServer()
{
  WiFiEspClient client = server.available();  // listen for incoming clients
  if (client) {                               // if you get a client,
    Monitor.println("New client");             // print a message out the serial port
     int error = 1;
     int relay = 10;
    buf.init();                               // initialize the circular buffer
    while (client.connected()) {              // loop while the client's connected
      if (client.available()) {               // if there's bytes to read from the client,
      char c = client.read();               // read a byte, then
      if ((relay > 47) && (relay < 58)) {
      relay = relay - 48;
      if (c == '+') bitSet(relayState, relay -1); 
        else bitClear(relayState, relay -1);
      Monitor.print("relay = ");
      Monitor.print(relay);
      Monitor.print("  c = ");
      Monitor.print(c);
      Monitor.print("  Bit = ");
      Monitor.println(relay - 1);
      error = 0;
      }
      if (relay == 0) relay = c;
     
      if (c == '\r') Monitor.print("\\r");
      if (c == '\n') Monitor.print("\\n");
      if (c > 13) Monitor.write(c);  
               
        buf.push(c);                          // push it to the ring buffer

        // printing the stream to the serial monitor will slow down
        // the receiving of data from the ESP filling the serial buffer
        //Serial.write(c);
        
        // you got two newline characters in a row
        // that's the end of the HTTP request, so send a response
        if (buf.endsWith("\n")) {
          client.flush();
          if (error == 0)  // /GET found
            {
               Monitor.println(" ");
               Monitor.println("...............sending homepage..........");            

               client.print(F("HTTP/1.1 200 OK\r\n Content-Type:text/html\r\n Connection:close\r\n\r\n"));    
               client.print(F("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"));
               client.print(F("<style>* {color:#000} h1 {background:#cccccc} "));
               client.print(F("button{width:130px;height:40px; font-size:16px; border:none;cursor:pointer;border-radius:5px; margin-right: 15px;}"));
               client.print(F(".on1{background:#4CAF50; color:white;}.on0{background:silver;color:black;}"));
               client.print(F("</style></head><body><center><br><h1>ESP8266 WiFi Web Server ST</h1><br><br>"));

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
            client.print(Line);
            i++;
                  }  while (i<7);

                client.print(F("<br><br><h1>Guenter Schmitt V5.0</h1><br><br>"));

                Monitor.println("................homepage sent.................");     

                  }
         else {    // also printed with /favicon
          Monitor.println("");
          Monitor.println("Sende 404 not found Fehler....");
          client.print(F("HTTP/1.1 404 Not found\r\n Content-Type:text/html\r\n Connection:close\r\n\r\n"));    
          client.print(F("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"));
          client.print(F("<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>"));
         }
          break;
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (buf.endsWith("GET / ")) {
          Monitor.println("  -> GET / found");
          error = 0;
        }  

        if (buf.endsWith("?rel")) {
          Monitor.println("  -> relay found");
          relay = 0;
        }
      }
    }
    
    // close the connection
    client.stop();
    Monitor.println("Client disconnected");
  }
}

void processDisplay() {

switch (state)  {
case 0:   // nothing to do....
  break;

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
display.displayText(pos);
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
    if (x < end) {
    pos = displayBuffer + x;
    display.displayText(pos);
    x++;
    waitTime = 5;
    } else {
    waitTime = 40;     // 40 * 50ms = 1 Sec.
    state++; 
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
//strcpy(displayBuffer, "IP=");
//strcat(displayBuffer, ip);
state = 1;
}
keyPressTime++;
keyOld = keyNew;
}   

}


void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Monitor.print("SSID: ");
  Monitor.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Monitor.print("IP Address: ");
  Monitor.println(ip);
//  strcpy(displayBuffer, "IP=");
//  int temp = 0;
//  do {
//      displayBuffer[temp+3] = ip[temp];
//      temp++; 
//      }  while (ip[temp-1] != '\0');              
//  display.clear();
//  display.displayText(displayBuffer);

//  state = 0; 

  // print where to go in the browser
  Monitor.println();
  Monitor.print("To see this page in action, open a browser to http://");
  Monitor.println(ip);
  Monitor.println();
}

void setRelay()    // relayState
  {
   byte temp = ~relayState;   //  relays are 0 active  
//   byte temp = relayState;   //   relays are 1 active  
int temp1 = 0;
do {
digitalWrite(relay[temp1], bitRead(temp,temp1));
temp1++;
   } while (temp1 < 8);   
     display.setColorLEDs(0, relayState);      
}

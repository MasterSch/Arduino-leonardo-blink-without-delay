//    Arduino Leonardo Webserver with WiFiEsp library


//    Arduino Leonardo Webserver_Jul2019_new V50
//    tm1638 library 
//    but now TM1638GS
//    relays are active low
//    with refresh

//    Arduino Leonardo mit LED Modul an PCF8574 Port A
//    LED an bei output Low
//    I2C bus address is 0x20
//    an SDA und SCL

//    TM1638 LED&KEY pins
//    STB      Arduino Leonardo  PIN D5
//    CLK      Arduino Leonardo  PIN D6
//    DAT      Arduino Leonardo  PIN D7



#include <Arduino.h>
#include "WiFiEsp.h"
#include <TM1638GS.h>
#include "Wire.h"

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
int keyOld = 0, keyNew = 0, keyPressTime = 0, repeat = 0;
int waitTime = 0, state = 0, x = 0, end = 0, base = 0x40, relay1 = 0;
unsigned long prevTime = millis();

static char Line[135] = "<a href=\"?rel1+\"><button class=\"on1\">Relay 1 ON </button></a><a href=\"?rel2-\"><button class=\"on0\">Relay 1 OFF</button></a><br><br>";
char displayBuffer[20] = " StArt  ";
const char* pos = displayBuffer;
int relayState = 0;
WiFiEspServer server(80);

// use a ring buffer to increase speed and reduce memory allocation
RingBuffer buf(20);

void setup()
{
  display.reset();
  display.set_Brightness(2);
  display.set_Chars(0,pos);

  Monitor.begin(115200);   // initialize serial for debugging
  esp8266.begin(115200);   // initialize serial for ESP module

  Wire.begin(); // wake up I2C bus
                // set I/O pins to outputs
  setRelay();

  WiFi.init(&esp8266);    // initialize ESP module

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Monitor.println("WiFi shield not present");
    display.set_Chars(0,"No ESP..");
    // don't continue
    while (true);
  }
    display.set_Chars(0,pos);

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
  IPAddress ip = WiFi.localIP();
  sprintf(displayBuffer, "IP = %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  state = 1;
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
     int relay = -1;
    buf.init();                               // initialize the circular buffer
    while (client.connected()) {              // loop while the client's connected
      if (client.available()) {               // if there's bytes to read from the client,
      char c = client.read();                 // read a byte, then
      if (relay > 10) {                       // only when not refreh
      relay1 = relay;
      base ^= 0x20;    // XOR toggle Bit
      relay &= 0x0f;
      sprintf(displayBuffer, "Relay %d", relay);   
      if (c == '+') {
        bitSet(relayState, relay -1);
        strcat(displayBuffer, " On");
      }
      if (c == '-') {
        bitClear(relayState, relay -1);
        strcat(displayBuffer, " Off");
      }
      setRelay();
      state = 1;      
      error = 0;
      }


      if (relay == 0) {
        relay = c;
        if (relay == relay1) {
          relay = -1;  // is refresh
          error = 0;   // show homepage
        }   
      }
               
        buf.push(c);                          // push it to the ring buffer

        if (buf.endsWith("\n")) {
          client.flush();
          if (error == 0)  // /GET found
            {
               Monitor.println(" ");
               Monitor.println("...............sending homepage..........");            

               client.print(F("HTTP/1.1 200 OK\r\n Content-Type:text/html\r\n Connection:close\r\n\r\n"));    
               client.print(F("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"));
               client.print(F("<meta http-equiv=\"refresh\" content=\"10\">"));  // geö
               client.print(F("<style>* {color:#000} h1 {background:#cccccc} "));
               client.print(F("button{width:130px;height:40px; font-size:16px; border:none;cursor:pointer;border-radius:5px; margin-right: 15px;}"));
               client.print(F(".on1{background:#4CAF50; color:white;}.on0{background:silver;color:black;}"));
               client.print(F(" #google{color:blue;font-size:26px;}</style></head>"));
               client.print(F("<body><center><br><br><h1>ESP8266 WiFi Web Server V50 </h1><br><br>"));

              int i = 0; 
              do {
                Line[13] = i + base + 1;
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
                Line[74] = i + base + 1;
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
            // Monitor.println(Line); 
            client.print(Line);
            i++;
                  }  while (i<7);

                client.print(F("<br><br><hr><a id=\"google\" href=\"http://google.de\">Google.de</a>\r\n\r\n"));

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
          Monitor.println("  -> ?rel found");
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

case 1:     // init and display message
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
waitTime = 10;     // 20 * 50ms = 1 sec.
state++;
break;

case 2:     // wait
if (waitTime > 0) waitTime--;
else state++; 
break;

case 3:     //  shift left if needed
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

case 4:     // wait again
 if (waitTime > 0) waitTime--;
 else state++; 
break;

case 5:     //  finished
 display.clear_digits();
 if (repeat > 0) {
   repeat--;
   waitTime = 40;     // 40 * 50ms = 1 Sec. 
   state++;
 } 
 else state = 0;
break;

case 6:     // wait again
 if (waitTime > 0) waitTime--;
 else state=1; 
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
displayBuffer[0] = '\0';
sprintf(displayBuffer, "Relay %d ", temp1);
if (bitRead(relayState, temp1-1) == 1)
{  strcat(displayBuffer, "off");
   bitClear(relayState, temp1-1);
}  else  { 
strcat(displayBuffer, "on");
   bitSet(relayState, temp1-1);
}
state = 1;
   setRelay();
     }
  keyOld = 0;       
  keyPressTime = 0;
} else {          // key s(till) pressed
if (keyPressTime == 6) {
  IPAddress ip = WiFi.localIP();
  sprintf(displayBuffer, "IP = %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
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

  // print where to go in the browser
  Monitor.println();
  Monitor.print("To see this page in action, open a browser to http://");
  Monitor.println(ip);
  Monitor.println();
}

  
 void setRelay() {
 Wire.beginTransmission(0x20);
 Wire.write(~relayState); // port data
 Wire.endTransmission();
 display.setColorLEDs(0x00, relayState);  // <--  green
}
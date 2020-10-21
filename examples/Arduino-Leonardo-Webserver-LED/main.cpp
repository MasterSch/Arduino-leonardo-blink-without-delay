/*
 WiFiEsp example: WebServerLed
 
 A simple web server that lets you turn on and of an LED via a web page.
 This sketch will print the IP address of your ESP8266 module (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to turn on and off the LED on pin 13.

 For more details see: http://yaab-arduino.blogspot.com/p/wifiesp.html
*/
#include <Arduino.h>
#include "WiFiEsp.h"

#define debug true    // true means debug messages on serial monitor

char ssid[] = "Obernburg1";            // your network SSID (name)
char pass[] = "Sabine.Obernburg";        // your network password

static char Line[135] = "<a href=\"?rel1+\"><button class=\"on1\">Relay 1 ON </button></a><a href=\"?rel2-\"><button class=\"on0\">Relay 1 OFF</button></a><br><br>";

int status = WL_IDLE_STATUS;
int ledStatus = LOW;
int relayState = 0;

void sendHttpResponse(WiFiEspClient client);
void printWifiStatus(void);

WiFiEspServer server(80);



// use a ring buffer to increase speed and reduce memory allocation
RingBuffer buf(8);

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);	// initialize digital pin LED_BUILTIN as an output.
  Serial.begin(115200);   // initialize serial for debugging
  Serial1.begin(115200);    // initialize serial for ESP module
  WiFi.init(&Serial1);    // initialize ESP module

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
  printWifiStatus();
  
  // start the web server on port 80
  server.begin();
}


void loop() {
  // Check if a client has connected
  WiFiEspClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while (!client.available()) {
    delay(1);
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Match the request
 if (request.indexOf("/favicon") == -1) {
 int index = request.indexOf("/?rel");
 if (index != -1) {
   Serial.print("index = ");
   Serial.println(index);
 int Relay = request[index + 5] - 48;  
 char onoff = request[index + 6];  
   Serial.print("Relay = ");
   Serial.println(Relay);
      Serial.print("on off = ");
   Serial.println(onoff);
 if (onoff == '+') bitSet(relayState, Relay -1);
 if (onoff == '-') bitClear(relayState, Relay -1);
 }
  // Prepare the response
if (debug) Serial.println("...............sending homepage..........");
if (debug) Serial.println("sending Header.....");    

client.print(F("HTTP/1.1 200 OK\r\n Server: Arduino\r\n Content-Type:text/html\r\n Connection:close\r\n\r\n"));    
client.print(F("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"));
client.print(F("<style>* {color:#000} h1 {background:#cccccc} "));
client.print(F("button{width:130px;height:40px; font-size:16px; border:none;cursor:pointer;border-radius:5px; margin-right: 15px;}"));
client.print(F(".on1{background:#4CAF50; color:white;}.on0{background:silver;color:black;}"));
client.print(F("</style></head><body><center><br><h1>client WiFi Web Server AP</h1><br><br>"));

if (debug) Serial.println("Header sent");     
if (debug)  Serial.println("sending Buttons....");

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
if (debug) Serial.println(Line); 
client.print(Line);

i++;
  }  while (i<7);

if (debug) Serial.println("Buttons sent");     

if (debug) Serial.println("sending bottom.....");

client.print(F("<br><br><h1>Guenter Schmitt V4.0</h1><br><br>"));

if (debug) Serial.println("homepage sent");     

 }

  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed

  client.flush();
  client.stop();
}







void sendHttpResponse(WiFiEspClient client)
{
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  
  // the content of the HTTP response follows the header:
  client.print("The LED is ");
  client.print(ledStatus);
  client.println("<br>");
  client.println("<br>");
  
  client.println("Click <a href=\"/H\">here</a> turn the LED on<br>");
  client.println("Click <a href=\"/L\">here</a> turn the LED off<br>");
  
  // The HTTP response ends with another blank line:
  client.println();
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in the browser
  Serial.println();
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
  Serial.println();
}
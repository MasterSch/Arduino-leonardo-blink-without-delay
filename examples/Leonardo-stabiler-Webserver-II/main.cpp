#include <Arduino.h>

/*
 WiFiEsp example: WebServerLed
 
 A simple web server that lets you turn on and of an LED via a web page.
 This sketch will print the IP address of your ESP8266 module (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to turn on and off the LED on pin 13.

 For more details see: http://yaab-arduino.blogspot.com/p/wifiesp.html
*/

#include <WiFiEsp.h>


void printWifiStatus(void);
void sendHttpResponse(WiFiEspClient client);


char ssid[] = "zuhause";            // your network SSID (name)
char pass[] = "schmitt.obw@web.de";        // your network password
int status = WL_IDLE_STATUS;

static char Line[135] = "<a href=\"?rel1+\"><button class=\"on1\">Relay 1 ON </button></a><a href=\"?rel2-\"><button class=\"on0\">Relay 1 OFF</button></a><br><br>";

int ledStatus = LOW;
int relayState = 0;
WiFiEspServer server(80);


// use a ring buffer to increase speed and reduce memory allocation
RingBuffer buf(12);

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
  while(!client.available()){
    delay(1);
  }
 
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.print("request: ");
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
sendHttpResponse(client);
client.stop();
}  else  {
          Serial.println("");
          Serial.println("Sende 404 not found Fehler....");
          client.print(F("HTTP/1.1 404 Not found\r\n Content-Type:text/html\r\n Connection:close\r\n\r\n"));    
          client.print(F("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"));
          client.print(F("<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>"));
         }

}



void sendHttpResponse(WiFiEspClient client)   {

Serial.println("...............sending homepage..........");            
Serial.println("sending Header.....");    

client.print(F("HTTP/1.1 200 OK\r\n Content-Type:text/html\r\n Connection:close\r\n\r\n"));    
client.print(F("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"));
client.print(F("<style>* {color:#000} h1 {background:#cccccc} "));
client.print(F("button{width:130px;height:40px; font-size:16px; border:none;cursor:pointer;border-radius:5px; margin-right: 15px;}"));
client.print(F(".on1{background:#4CAF50; color:white;}.on0{background:silver;color:black;}"));
client.print(F("</style></head><body><center><br><h1>ESP8266 WiFi Web Server ST</h1><br><br>"));

Serial.println("Header sent.........");     

Serial.println("sending Buttons.....");

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
Serial.println(Line); 
client.print(Line);

i++;
  }  while (i<7);

Serial.println("Buttons sent");     

Serial.println("sending bottom.....");

client.print(F("<br><br><h1>Guenter Schmitt V4.0</h1><br><br>"));

Serial.println("homepage sent");     

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








/*
void loop()
{
  WiFiEspClient client = server.available();  // listen for incoming clients
  int favicon = 0;
  int relay = 10;
  if (client) {                               // if you get a client,
    Serial.println("New client");             // print a message out the serial port
    buf.init();                               // initialize the circular buffer
    while (client.connected()) {              // loop while the client's connected
      if (client.available()) {               // if there's bytes to read from the client,
      char c = client.read();               // read a byte, then
      if (relay > 10) {
      relay = relay - 48;
      if (c == '+') bitSet(relayState, relay -1); 
        else bitClear(relayState, relay -1);
      Serial.print("relay = ");
      Serial.print(relay);
      Serial.print("  c = ");
      Serial.print(c);
      Serial.print("  Bit = ");
      Serial.println(relay - 1);
      }
      if (relay == 0) relay = c;
     
      if (c == '\r') Serial.print("\\r");
      if (c == '\n') Serial.print("\\n");
      if (c > 13) Serial.write(c);  
               
        buf.push(c);                          // push it to the ring buffer

        // printing the stream to the serial monitor will slow down
        // the receiving of data from the ESP filling the serial buffer
        //Serial.write(c);
        
        // you got two newline characters in a row
        // that's the end of the HTTP request, so send a response
        if (buf.endsWith("\r\n\r\n")) {
          if (favicon == 0) sendHttpResponse(client);
         else {
          Serial.println("");
          Serial.println("Sende 404 not found Fehler....");
          client.print(F("HTTP/1.1 404 Not found\r\n Content-Type:text/html\r\n Connection:close\r\n\r\n"));    
          client.print(F("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"));
          client.print(F("<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>"));
         }
          break;
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (buf.endsWith("GET /H")) {
          Serial.println("Turn led ON");
          ledStatus = HIGH;
          digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
        }
        if (buf.endsWith("GET /L")) {
          Serial.println("Turn led OFF");
          ledStatus = LOW;
          digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
        }  
        if (buf.endsWith("/favicon")) {
          Serial.println("  -> favicon found");
          favicon = 1;
        }
        if (buf.endsWith("GET /?rel")) {
          Serial.println("  -> relay found");
          relay = 0;
        }
      }
    }
    
    // close the connection
    client.stop();
    Serial.println("Client disconnected");
  }
}
*/

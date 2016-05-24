
#include "SPI.h"
#include "WiFi.h"

char ssid[] = "jonghyun";     //  your network SSID (name) 
char pass[] = "86701240";     //  your network password 
String location = "Gangnam"; // your desired location


// initialize the library instance:
WiFiServer server(80);
WiFiClient client;

const unsigned long requestInterval = 60000;  // delay between requests (1 min)

IPAddress hostIp;
uint8_t ret;
boolean requested;                   // whether you've made a request since connecting
unsigned long lastAttemptTime = 0;            // last time you connected to the server, in milliseconds
String currentLine = "";        // string to hold the text from server
String tempString = "";         // string to hold temp
String humString = "";          // string to hold humidity
String timeString = "";         // string to hold timestamp
String pressureString = "";
boolean readingTemp = false;    // if you're currently reading the temp
boolean readingHum = false;     // if you're currently reading the humidity
boolean readingTime = false;    // if you're currently reading the timestamp
boolean readingPressure = false; 
int temp = 0;

void setup() {
  // reserve space for the strings:
  currentLine.reserve(100);
  tempString.reserve(10);
  humString.reserve(10);
  timeString.reserve(20);
  Serial.begin(115200);    

  delay(10);
  // Connect to an AP with WPA/WPA2 security
  Serial.println("Connecting to WiFi....");  
  WiFi.begin(ssid, pass);  // Use this if your wifi network requires a password
  //WiFi.begin(ssid);    // Use this if your wifi network is unprotected.

  server.begin();
  Serial.println("Connect success!");
  Serial.println("Waiting for DHCP address");
  // Wait for DHCP address
  while(WiFi.localIP() == INADDR_NONE) {
    Serial.print(".");
    delay(300);
  }

  Serial.println("\n");
  printWifiData();
  connectToServer();

}

void loop()
{
  if (client.connected()) {
    while (client.available()) {
      // read incoming bytes:
      char inChar = client.read();
      // add incoming byte to end of line:
      currentLine += inChar; 
      // if you get a newline, clear the line:
      //Serial.println("trying to parse...");

      if (inChar == '\n') {
        //Serial.print("clientReadLine = ");
        //Serial.println(currentLine);
        currentLine = "";
      } 
      // LOOKING FOR TEMPERATURE DATA
      // if the current line ends with <temperature value=, it will
      // be followed by the temp:
      if ( currentLine.endsWith("<temperature value=")) {
        // temperatue data is beginning. Clear the temp string:
        readingTemp = true; 
        tempString = "";
      }      

      // PULLING TEMPERATURE DATA
      // if you're currently reading the bytes for temperature,
      // add them to the temperature string:
      if (readingTemp) {
        if (inChar != 'm') { // if you see 'm', you're done reading temp
          tempString += inChar;
        } 
        else {
          readingTemp = false;

          Serial.print("-  Temperature: ");
          Serial.print(getInt(tempString)-273);
          Serial.println((char)176);    //degree symbol
        }
      }

      if ( currentLine.endsWith("<humidity value=")) {
        // Humidity reading is beginning. Clear the humidity string:
        readingHum = true; 
        humString = "";
      }

      if (readingHum) {
        if (inChar != 'u') {// if you see 'u', you're done reading humidity
          humString += inChar;
        } 
        else {
          readingHum = false;

          Serial.print("-  Humidity: ");
          Serial.print(getInt(humString));
          Serial.println((char)37);    //percent sign
        }
      }

      if ( currentLine.endsWith("<lastupdate value=")) {
        // timestamp is beginning. Clear the timestamp string:
        readingTime = true; 
        timeString = "";
      }

      if (readingTime) {
        if (inChar != '/') { // if you see '/', you're done reading timestamp
          timeString += inChar;
        } 
        else {

          readingTime = false;

          Serial.print("-  Last update: ");
          Serial.println(timeString.substring(2,timeString.length()-1));
        }
      }

      if ( currentLine.endsWith("<pressure value=")) {
        readingPressure = true; 
        pressureString = "";
      }      

      if (readingPressure) {
        if (inChar != 'u') { 
          pressureString += inChar;
        } 
        else {
          readingPressure = false;

          Serial.print("-  Pressure: ");
          Serial.print(getInt(pressureString));
          Serial.println("hPa"); 
        }

      }

      if ( currentLine.endsWith("</current>")) {
        delay(10000);
        client.stop(); 
        connectToServer();
        //Serial.println("Disconnected from Server.\n");
      }
    }   

  }
  else if (millis() - lastAttemptTime > requestInterval) {
    // if you're not connected, and request interval time have passed since
    // your last connection, then attempt to connect again to get new data:
    connectToServer();
  }  
}

void connectToServer() {
  Serial.println("connecting to server...");
  String content = "";
  if (client.connect(hostIp, 80)) {
    Serial.println("Connected! Making HTTP request to api.openweathermap.org for "+location+"...");
    //Serial.println("GET /data/2.5/weather?q="+location+"&mode=xml");
    // make HTTP GET request to Facebook:
    client.println("GET /data/2.5/weather?q="+location+"&mode=xml"); 
    // declare correct server
    client.print("HOST: api.openweathermap.org\n");
    client.println("User-Agent: launchpad-wifi");
    client.println("Connection: close");

    client.println();
    Serial.println("Weather information for "+location);
  }
  // note the time of this connect attempt:
  lastAttemptTime = millis();
}


void printHex(int num, int precision) {
  char tmp[16];
  char format[128];

  sprintf(format, "%%.%dX", precision);

  sprintf(tmp, format, num);
  Serial.print(tmp);
}

void printWifiData() {
  // print your WiFi shield's IP address:
  Serial.println();
  Serial.println("IP Address Information:");  
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];  
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printHex(mac[5], 2);
  Serial.print(":");
  printHex(mac[4], 2);
  Serial.print(":");
  printHex(mac[3], 2);
  Serial.print(":");
  printHex(mac[2], 2);
  Serial.print(":");
  printHex(mac[1], 2);
  Serial.print(":");
  printHex(mac[0], 2);
  Serial.println();
  // print your subnet mask:
  IPAddress subnet = WiFi.subnetMask();
  Serial.print("NetMask: ");
  Serial.println(subnet);

  // print your gateway address:
  IPAddress gateway = WiFi.gatewayIP();
  Serial.print("Gateway: ");
  Serial.println(gateway);

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  ret = WiFi.hostByName("api.openweathermap.org", hostIp);

  Serial.print("ret: ");
  Serial.println(ret);

  Serial.print("Host IP: ");
  Serial.println(hostIp);
  Serial.println("");
}

int getInt(String input){  // This function converts String to Integer.
  // This allows you to perform math on the string data we extracted from XML.
  int i = 2;

  while(input[i] != '"'){
    i++;
  }
  input = input.substring(2,i);
  char carray[20];
  //Serial.println(input);
  input.toCharArray(carray, sizeof(carray));
  //Serial.println(carray);
  temp = atoi(carray);
  //Serial.println(temp);
  return temp;
}


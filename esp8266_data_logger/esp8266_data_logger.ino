#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#include "DHT.h" // including the library of DHT11 temperature and humidity sensor
#define DHTTYPE DHT11 // DHT 11
#define dht_dpin 0
DHT dht(dht_dpin, DHTTYPE); 

#define ON_Board_LED 2  // On board LED, indicator when connecting to a wifi router


const char* ssid = "TIGO-55CB";   // Your wifi name
const char* password = "4NJ9ED802857"; // Your wifi password


//----------------------------------------Host & httpsPort
const char* host = "script.google.com";
const int httpsPort = 443;
//----------------------------------------

WiFiClientSecure client; // Create a WiFiClientSecure object

// Google spreadsheet script ID
String GAS_ID = "AKfycbwIXH3CmePujetx5NmHq51EvmDqnL_fhbdzPdX22QX8KkQI7Jva4v1zpI0TNs46t3A";

void setup() {
  dht.begin(); 
  Serial.begin(115200);

  Serial.println("Humidity and temperature\n\n");
  delay(700); 

  WiFi.begin(ssid, password); // Connect to your WiFi router
  Serial.println("");
    
  pinMode(ON_Board_LED,OUTPUT);     // On board LED port as output
  digitalWrite(ON_Board_LED, HIGH); // Turn off Led on board

  //----------------------------------------Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    //----------------------------------------Make LED flashing when connecting to the wifi router
    digitalWrite(ON_Board_LED, LOW);
    delay(200);
    digitalWrite(ON_Board_LED, HIGH);
    delay(200);
    //----------------------------------------
  }
  //----------------------------------------
  digitalWrite(ON_Board_LED, HIGH); // Turn off the LED when it is connected to the wifi router
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //----------------------------------------

  client.setInsecure();
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Serial.print("Current humidity = ");
  Serial.print(h);
  Serial.print("% ");
  Serial.print("temperature = ");
  Serial.print(t);
  Serial.println("C ");
  delay(800); 
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT11 sensor !");
    delay(500);
    return;
  }
  
  String Temp = "Temperature : " + String(t) + " °C";
  String Humi = "Humidity : " + String(h) + " %";

  Serial.println(Temp);
  Serial.println(Humi);

  
  sendData(t, h); // Call the sendData subroutine
  delay(5000);
}

// Subroutine for sending data to Google Sheets
void sendData(float tem, float hum) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  //----------------------------------------

  //----------------------------------------Processing data and sending data
  String string_temperature =  String(tem);
  String string_humidity =  String(hum);
  
  String url = "https://script.google.com/macros/s/" + GAS_ID + "/exec?temperature=" + string_temperature + "&humidity=" + string_humidity;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  //----------------------------------------

  //----------------------------------------Checking whether the data was sent successfully or not
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  //----------------------------------------
} 

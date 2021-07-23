
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <DHT.h>
#include "keys.h"




#define DHTPIN 2  // pin D4 on ESP8266
#define DHTTYPE DHT11   // dht-11




const char* ssid     = SSID;
const char* password = PWD;

const uint16_t port = 80; 
//443;

const char* host = "http://api.thingspeak.com/update?api_key=219PKOIIE9QMF1QA";
char hum[7];
char temp[7];
char url[256];


DHT dht(DHTPIN, DHTTYPE);
void setup() {
  Serial.begin(115200);

  // Connecting to WiFi

  Serial.println();
  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  // DHT start
  dht.begin();
}

void loop() {
  static bool wait = false;


  float h = dht.readHumidity();
  dtostrf(h,1,2,hum);
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature(false);
  dtostrf(t,1,2,temp);

   // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.println(t);

    sprintf(url,"%s&field1=%s&field2=%s",host,hum,temp);

    Serial.println(url);
    
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  HTTPClient http;

  http.begin(client,url);
  int respCode = http.GET();
  Serial.println(respCode);
  http.end();
 
  // Close the connection
  Serial.println("closing connection");

  if (wait) {
    delay(60000); // execute once every 5 minutes, don't flood remote service
  }
  wait = true;

  
}

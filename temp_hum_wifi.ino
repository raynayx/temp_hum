#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <DHT.h>
#include "keys.h"



//*************************GET ESP8266 Libraries**********************************
// http://arduino.esp8266.com/stable/package_esp8266com_index.json
//********************************************************************************


//***********************DHT-11***************************************************
#define DHTPIN 2  // pin D4 on ESP8266
#define DHTTYPE DHT11   // dht-11
//********************************************************************************

//**************************MQTT Setup********************************************
const char *mqttUsername = SECRET_MQTT_USERNAME;
const char *clientID = SECRET_MQTT_CLIENT_ID;
const char *mqttPass = SECRET_MQTT_PASSWORD;
const char* server = "mqtt3.thingspeak.com";
#define mqttPort 8883
// certificate
const char* PROGMEM thingspeak_cert_thumbprint = THUMB_PRINT;
//********************************************************************************

WiFiClientSecure client; 


int status = WL_IDLE_STATUS; 
long lastPublishMillis = 0;
int connectionDelay = 1;
int updateInterval = 15;
PubSubClient mqttClient(client);


DHT dht(DHTPIN, DHTTYPE);
void setup()
{
  Serial.begin(115200);

  // Connecting to WiFi
  Serial.print(F("Connecting to "));
  Serial.println(SSID);
  connectWifi();


  // Configure the MQTT client
  mqttClient.setServer( server, mqttPort ); 

  // Set the MQTT message handler function.
  mqttClient.setCallback(mqttSubscriptionCallback);

  // Set the buffer to handle the returned JSON.
  // NOTE: A buffer overflow of the message buffer will result in your callback not being invoked.
  mqttClient.setBufferSize(2048);


  // Use secure MQTT connections if defined.
  // Handle functionality differences of WiFiClientSecure library for different boards.
  client.setFingerprint(thingspeak_cert_thumbprint);
    


  // DHT start
  dht.begin();
}

void loop()
{
  float h = dht.readHumidity();
  // dtostrf(h,1,2,hum);
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature(false);
  // dtostrf(t,1,2,temp);


  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.println(t);

 // Reconnect to WiFi if it gets disconnected.
  if (WiFi.status() != WL_CONNECTED)
  {
      connectWifi();
  }
  
  // Connect if MQTT client is not connected and resubscribe to channel updates.
  if (!mqttClient.connected())
  {
     mqttConnect(); 
     mqttSubscribe(CHANNEL_ID);
  }
  
  // Call the loop to maintain connection to the server.
  mqttClient.loop(); 
  
  
  // Update ThingSpeak channel periodically. The update results in the message to the subscriber.
    mqttPublish( CHANNEL_ID, (String("field1=")+String(h)));
    mqttPublish( CHANNEL_ID, (String("field2=")+String(t)));
    delay(20000);
  
}



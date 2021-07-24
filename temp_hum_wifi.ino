#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include "keys.h"


//******************prototypes*****************************************************
void mqttSubscriptionCallback(char* topic, byte* payload, unsigned int length);
void mqttSubscribe(long subChannelID);
void mqttPublish(long pubChannelID, String message);
void connectWifi();
void mqttConnect();
//*********************************************************************************


//*************************GET ESP8266 Libraries**********************************
// http://arduino.esp8266.com/stable/package_esp8266com_index.json
//********************************************************************************


//***********************DHT-11***************************************************
#define DHTPIN 2  // pin D4 on ESP8266
#define DHTTYPE DHT11   // dht-11
//********************************************************************************


const char *mqttUsername = SECRET_MQTT_USERNAME;
const char *clientID = SECRET_MQTT_CLIENT_ID;
const char *mqttPass = SECRET_MQTT_PASSWORD;

// certificate
const char* PROGMEM thingspeak_cert_thumbprint = THUMB_PRINT;

#define mqttPort 8883
WiFiClientSecure client; 
const char* server = "mqtt3.thingspeak.com";

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
  // Set the buffer to handle the returned JSON. NOTE: A buffer overflow of the message buffer will result in your callback not being invoked.
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


// Function to handle messages from MQTT subscription.
void mqttSubscriptionCallback( char* topic, byte* payload, unsigned int length )
{
  // Print the details of the message that was received to the serial monitor.
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Subscribe to ThingSpeak channel for updates.
void mqttSubscribe( long subChannelID )
{
  String myTopic = "channels/"+String( subChannelID )+"/subscribe";
  mqttClient.subscribe(myTopic.c_str());
}

// Publish messages to a ThingSpeak channel.
void mqttPublish(long pubChannelID, String message)
{
  String topicString ="channels/" + String( pubChannelID ) + "/publish";
  mqttClient.publish( topicString.c_str(), message.c_str() );
}

// Connect to WiFi.
void connectWifi()
{
  Serial.print( "Connecting to Wi-Fi..." );
  // Loop until WiFi connection is successful
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    WiFi.begin( SSID, PWD );
    delay(connectionDelay*1000);
    Serial.println( WiFi.status() ); 
  } 
  Serial.println("Connected to Wi-Fi.");
}


// Connect to MQTT server.
void mqttConnect()
{
  // Loop until connected.
  while (!mqttClient.connected())
  {
    // Connect to the MQTT broker.
    if ( mqttClient.connect( clientID, mqttUsername, mqttPass ) )
    {
      Serial.print( "MQTT to " );
      Serial.print( server );
      Serial.print (" at port ");
      Serial.print( mqttPort );
      Serial.println( " successful." );
    }
    else
    {
      Serial.print( "MQTT connection failed, rc = " );
      // See https://pubsubclient.knolleary.net/api.html#state for the failure code explanation.
      Serial.print( mqttClient.state() );
      Serial.println( " Will try again in a few seconds" );
      delay( connectionDelay*1000 );
    }
  }
}
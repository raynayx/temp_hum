
#include <ESP8266WiFi.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include "keys.h"


//prototypes
void mqttSubscriptionCallback( char* topic, byte* payload, unsigned int length );
void mqttSubscribe( long subChannelID );
void mqttPublish(long pubChannelID, String message);
void connectWifi();
//****************************


// GET ESP8266 Libraries
// http://arduino.esp8266.com/stable/package_esp8266com_index.json

#define DHTPIN 2  // pin D4 on ESP8266
#define DHTTYPE DHT11   // dht-11


const char mqttUsername* = SECRET_MQTT_USERNAME;
const char clientID* = SECRET_MQTT_CLIENT_ID;
const char* mqttPass* = SECRET_MQTT_PASSWORD;

// certificate
const char * PROGMEM thingspeak_ca_cert = \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n" \
  "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
  "d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n" \
  "ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n" \
  "MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n" \
  "LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n" \
  "RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n" \
  "+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n" \
  "PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n" \
  "xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n" \
  "Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n" \
  "hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n" \
  "EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n" \
  "MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n" \
  "FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n" \
  "nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n" \
  "eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n" \
  "hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n" \
  "Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n" \
  "vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n" \
  "+OkuE6N36B9K\n" \
  "-----END CERTIFICATE-----\n";


  #define mqttPort 8883
  WiFiClientSecure client; 
  const char* server = "mqtt3.thingspeak.com";

  int status = WL_IDLE_STATUS; 
  long lastPublishMillis = 0;
  int connectionDelay = 1;
  int updateInterval = 15;
  PubSubClient mqttClient( client );

const char* ssid     = SSID;
const char* password = PWD;


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

  // setup MQTT
  connectWifi();
  // Configure the MQTT client
  mqttClient.setServer( server, mqttPort ); 
  // Set the MQTT message handler function.
  mqttClient.setCallback( mqttSubscriptionCallback );
  // Set the buffer to handle the returned JSON. NOTE: A buffer overflow of the message buffer will result in your callback not being invoked.
  mqttClient.setBufferSize( 2048 );
  // Use secure MQTT connections if defined.
  #ifdef USESECUREMQTT
    // Handle functionality differences of WiFiClientSecure library for different boards.
    #ifdef ESP8266BOARD
      client.setFingerprint(thingspeak_cert_thumbprint);
    #else
      client.setCACert(thingspeak_ca_cert);
    #endif
  #endif




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


 // Reconnect to WiFi if it gets disconnected.
  if (WiFi.status() != WL_CONNECTED) {
      connectWifi();
  }
  
  // Connect if MQTT client is not connected and resubscribe to channel updates.
  if (!mqttClient.connected()) {
     mqttConnect(); 
     mqttSubscribe( channelID );
  }
  
  // Call the loop to maintain connection to the server.
  mqttClient.loop(); 
  
  // Update ThingSpeak channel periodically. The update results in the message to the subscriber.
  if ( abs(millis() - lastPublishMillis) > updateInterval*1000) {
    mqttPublish( channelID, (String("field1=")+String(WiFi.RSSI())) );
    lastPublishMillis = millis();
  }
  
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
  #ifdef ESP8266BOARD
    while ( WiFi.waitForConnectResult() != WL_CONNECTED )
    {
    #else
    while ( WiFi.status() != WL_CONNECTED )
    {
    #endif
      WiFi.begin( ssid, pass );
      delay( connectionDelay*1000 );
      Serial.print( WiFi.status() ); 
    } 
    Serial.println( "Connected to Wi-Fi." );
    }
}

// Connect to MQTT server.
void mqttConnect()
{
  // Loop until connected.
  while ( !mqttClient.connected() )
  {
    // Connect to the MQTT broker.
    if ( mqttClient.connect( clientID, mqttUserName, mqttPass ) )
    {
      Serial.print( "MQTT to " );
      Serial.print( server );
      Serial.print (" at port ");
      Serial.print( mqttPort );
      Serial.println( " successful." );
    }else
    {
      Serial.print( "MQTT connection failed, rc = " );
      // See https://pubsubclient.knolleary.net/api.html#state for the failure code explanation.
      Serial.print( mqttClient.state() );
      Serial.println( " Will try again in a few seconds" );
      delay( connectionDelay*1000 );
    }
  }
}
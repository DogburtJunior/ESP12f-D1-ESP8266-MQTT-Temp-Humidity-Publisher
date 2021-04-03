#include <Arduino.h>

// Include the ESP8266 Library. This library is automatically provided by the ESP8266 Board Manager and does not need to be installed manually.
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include "PubSubClient.h"
#include "DHT.h"

#define DHTPIN D5
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// WiFi Router Login 
const char* SSID = "RaspberryPi";
const char* wifi_password = "1a2d3m4i5n";
 
// MQTT settings
const char* mqtt_server="192.168.4.1"; // IP of the MQTT Broker
const char* humidity_topic="home/DHT_Station/humidity";
const char* temperature_topic="home/DHT_Station/temperature";
const char* temperatureF_topic="home/DHT_Station/temperatureF";
const char* mqtt_username="esp_d1"; // MQTT Username
const char* mqtt_password="weather"; // MQTT password
const char* clientID="weather_station_client";

//Initialize WiFi & client
WiFiClient wifiClient;
PubSubClient client(wifiClient);


void connect_MQTT() 
{
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, wifi_password);
  // delay(500);

  // Wait for connection
  while(WiFi.status() != WL_CONNECTED)
  {
    // yield();
    delay(500);
    Serial.print(".");
  }

  // Confirm connection & local IP
  Serial.println("\tWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  delay(500);

  if(client.connect(clientID, mqtt_username, mqtt_password)) 
  {
    Serial.println("Connected to MQTT Broker!");
  }
  else 
  {
    Serial.println("Connection to MQTT Broker Failed.");
    Serial.print("rc = ");
    Serial.println(client.state());
  }
}

void publish_MQTT(float out, const char* topic)
{
  if(client.publish(topic, String(out).c_str())) {
    Serial.print("Topic Sent to: ");
    Serial.println(topic);
  }
  else {
    Serial.print(topic);
    Serial.println(" failed to send. Reconnecting...");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10);
    client.publish(topic, String(out).c_str());
  }
}


void setup() 
{
  Serial.begin(74880);
  client.setServer(mqtt_server, 1883);
  dht.begin();
}


void loop() 
{
  connect_MQTT();
  Serial.setTimeout(20000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float tf = dht.convertCtoF(t);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C");
  Serial.print("TemperatureF: ");
  Serial.print(tf);
  Serial.println(" *F");

  String hs = "Hum: "+String((float)h)+" % ";
  String ts = "Temp: "+String((float)t)+" C ";
  String tfs = "Temp: "+String((float)tf)+" F ";

  publish_MQTT(t, temperature_topic);
  publish_MQTT(tf, temperatureF_topic);
  publish_MQTT(h, humidity_topic);

  client.disconnect();
  delay(1000*15);

}


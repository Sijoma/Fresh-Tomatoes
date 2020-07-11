#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "***REMOVED***";
const char* password = "***REMOVED***";
const char* mqtt_server = "***REMOVED*** ";

#define DHTPIN D2     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

// MQTT Def
#define clientId "ESP_TempHumid"
#define humidity_topic "sensor/humidity"
#define temperature_topic "sensor/temperature"
#define timeBetweenMeasurements 3000

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(1000);
}

void setup() {
  // We start by connecting to a WiFi network
  Serial.begin(115200);
  setup_wifi();
  Serial.println(F(" Bootup"));
  // Start temp sensor
  dht.begin();
  client.setServer(mqtt_server, 1883);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID

    // Attempt to connect
    if (client.connect(clientId)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("sensors", clientId);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    Serial.println("Reconnecting");
    reconnect();
  }
  client.loop();
  // Wait a few seconds between measurements.
  delay(timeBetweenMeasurements);
  readTemperature();
}

void readTemperature() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  
  client.publish(humidity_topic, String(h).c_str(), true);
  client.publish(temperature_topic, String(t).c_str(), true);

  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);
  
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C Heat Index: "));
  Serial.print(hic);
  Serial.println(F("°C "));

}

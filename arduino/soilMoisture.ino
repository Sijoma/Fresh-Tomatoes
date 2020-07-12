#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "***";
const char* password = "***";
const char* mqtt_server = "***";

// MQTT Def
#define clientId "Capac_GroundHumid1"
#define humidity_topic "prometheus/job/ESP8266/instance/raisedBed/soil_moisture_value"
#define humidity_percent_topic "prometheus/job/ESP8266/instance/raisedBed/soil_moisture_percent"

// Sensor Setup
const int AirValue = 620;
const int WaterValue = 290;
int soilMoistureValue = 0;
int soilMoisturePercent = 0;
WiFiClient espClient; 
PubSubClient client(espClient);

unsigned long time_now = 0;
int timeBetweenMeasurements = 5000; 

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
  Serial.begin(9600);
  setup_wifi();
  Serial.println(F(" Bootup"));
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
  if((unsigned long)(millis() - time_now) > timeBetweenMeasurements) {
    time_now = millis(); 
 
    readSoilMoisture();
  }
  
}

void readSoilMoisture() {
  soilMoistureValue = analogRead(A0);
  Serial.println(soilMoistureValue);
  client.publish(humidity_topic, String(soilMoistureValue).c_str(), true);

  
  soilMoisturePercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);

  if(soilMoisturePercent > 100){
    client.publish(humidity_percent_topic, String(100).c_str(), true);
  } else if(soilMoisturePercent < 0){
    client.publish(humidity_percent_topic, String(0).c_str(), true);
  } else if(soilMoisturePercent > 0 && soilMoisturePercent <100){
    client.publish(humidity_percent_topic, String(soilMoisturePercent).c_str(), true);
  }
  Serial.print(F("Humidity: "));
  Serial.print(soilMoistureValue);
  Serial.print(F("Percent: "));
  Serial.println(soilMoisturePercent);
}
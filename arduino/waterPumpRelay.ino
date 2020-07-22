#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "***";
const char* password = "***";
const char* mqtt_server = "***";

// MQTT Def
#define clientId "WaterPumpRelay"
#define relay_topic "prometheus/job/ESP8266/instance/raisedBed/Pump"

WiFiClient espClient;
PubSubClient client(espClient);


const int RELAY_PIN = D7;
volatile byte relayState = LOW;

unsigned long time_now = 0;
int timeBetweenMeasurements = 10000; 

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
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  setup_wifi();
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
    if(relayState == LOW){
      relayState = HIGH;
      digitalWrite(RELAY_PIN, relayState);
      Serial.println("ON");
      client.publish(relay_topic, "0", true);
    } else if(relayState == HIGH){
      relayState = LOW;
      digitalWrite(RELAY_PIN, relayState);
      client.publish(relay_topic, "1", true);
      Serial.println("OFF");
    }
  }
 

}
package main

import(
	"net/http"
	MQTT "github.com/eclipse/paho.mqtt.golang"
	"fmt"
	"strconv"
)

var mqttClient MQTT.Client

func PumpOn(w http.ResponseWriter, req *http.Request) {
	keys, ok := req.URL.Query()["waterTime"]
	if !ok {
		fmt.Fprintf(w, "not a valid request\n")
		return
	}
	key := keys[0]
	minutes, _ := strconv.Atoi(key)
	if minutes > 10 {
		fmt.Fprintf(w, "Don't water longer than 10 minutes")
		return
	}
	pumpState := ""
	if minutes > 0 {
		pumpState = "On 🟢\n"
	} else {
		pumpState = "Off 🔴\n"
	}
	fmt.Fprintf(w, "Pump is "+ pumpState + "For this amount of minutes: " +key + " ⏱\n")
	seconds := 1000 * 60 * minutes
	messagePump(strconv.Itoa(seconds));
}

func messagePump(wateringTime string){
	mqttClient.Publish("devices/waterPump", 0, false, wateringTime)
}

func connectToBroker() (MQTT.Client) {
	opts := MQTT.NewClientOptions().AddBroker("tcp://mosquitto:1883")
	opts.SetClientID("go-backend-client")
	client := MQTT.NewClient(opts)
	if token := client.Connect(); token.Wait() && token.Error() != nil {
		panic(token.Error())
	} else {
			fmt.Printf("Connected to server\n")
	}
	return client
}

func main() {
	mqttClient = connectToBroker();

	http.HandleFunc("/pump", PumpOn)
	http.ListenAndServe(":5000", nil)
}
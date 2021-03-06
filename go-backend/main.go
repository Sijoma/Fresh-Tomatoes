package main

import (
	"fmt"
	"net/http"
	"strconv"

	MQTT "github.com/eclipse/paho.mqtt.golang"
)

var mqttClient MQTT.Client

func pumpOn(w http.ResponseWriter, req *http.Request) {
	keys, ok := req.URL.Query()["waterTime"]
	if !ok {
		fmt.Fprintf(w, "not a valid request\n")
		return
	}
	key := keys[0]
	minutes, _ := strconv.Atoi(key)
	if minutes > 30 {
		fmt.Fprintf(w, "Don't water longer than 30 minutes")
		return
	}
	pumpState := ""
	if minutes > 0 {
		pumpState = "On 🟢\n"
	} else {
		pumpState = "Off 🔴\n"
	}
	fmt.Fprintf(w, "Pump is "+pumpState+"For this amount of minutes: "+key+" ⏱\n")
	seconds := 1000 * 60 * minutes
	messagePump(strconv.Itoa(seconds))
}

func messagePump(wateringTime string) {
	mqttClient.Publish("devices/waterPump", 0, false, wateringTime)
}

func connectToBroker() MQTT.Client {
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
	mqttClient = connectToBroker()
	mux := http.NewServeMux()
	pumpHandler := http.HandlerFunc(pumpOn)
	mux.Handle("/pump", basicAuth(pumpHandler))
	http.ListenAndServe(":5000", mux)
}

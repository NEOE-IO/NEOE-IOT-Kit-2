/********************************************************************************
  Arduino-Sketch für das NEOE IOT-Kit 3, "Thermometer und Hygrometer mit 
  DHT22/AM2302 und NodeMCU. Kompatibel mit Arduino, MQTT und Node-RED" 
  https://www.neoe.io/products/neoe-iot-kit-3-modell-a
  Fragen und Anregungen bitte in unserer Facebook-Gruppe adressieren,
  damit die gesamte Community davon profitiert. 
  https://www.facebook.com/groups/neoe.io
  Datum der letzten Änderung: 11. Oktober, 2020
********************************************************************************/
 
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <DHT.h>

/* WLAN-Verbindungsdaten */ 
const char* ssid = "NAME DES WLAN-NETZWERKS"; // Anführungszeichen beibehalten
const char* password = "WLAN-PASSWORT"; // Anführungszeichen beibehalten, also z.B. so: "Geheim"

/* MQTT-Verbindungsdaten */ 
const char* mqttServer = "IP-ADRESSE DES MQTT-SERVERS"; // Anführungszeichen beibehalten, also z.B. so: "192.168.0.236"
const uint16_t mqttPort = 1883;
const char* clientId = "NEOE-IOT-KIT-3-1"; // Wenn mehrere "NEOE IOT-Kits 3" im Einsatz sind, einfach mit der letzten Zahl durchnummerieren

WiFiClient espClient;
PubSubClient client(espClient);

#define DHTPIN 5        // Pin, an dem der Sensor angeschlossen wurde, z.B. D1 = DHTPIN 5
#define DHTTYPE DHT22   // Typ des DHT-Sensors

int counter;

DHT dht(DHTPIN, DHTTYPE);

void setup_wifi() {
  delay(10);
  /* Mit WLAN verbinden */
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void reconnect() {
  /* Drei Verbindungsversuche sollten ausreichen, notfalls geht es im Offlinebetrieb weiter */
  counter = 1;
  while (!client.connected() && counter<=3) {
    if (client.connect(clientId)) {
      client.subscribe("inTopic");
      counter++;
    } 
  }
}

void setup() {
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  dht.begin();
}

void loop() {

  /* Falls MQTT-Verbindung nicht besteht, versuchen, diese wiederherzustellen */
  if (!client.connected()) reconnect();

  /* Daten vom Sensor lesen */
  float t = dht.readTemperature(); 
  float h = dht.readHumidity();

  /* Daten an den MQTT-Server senden */
  client.publish("outTemperature01", String(t).c_str());
  client.publish("outHumidity01", String(h).c_str());
  
  delay(1000);

}

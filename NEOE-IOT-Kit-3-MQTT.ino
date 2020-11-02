/**********************************************************************************************************************************
  Arduino-Sketch für das NEOE-IOT-Kit-3, "Thermometer und Hygrometer mit DHT22/AM2302 und NodeMCU. Arduino-Programmierung.
  MQTT-kompatibel zur Anbindung an Home Assistant."
  Dieser Arduino-Sketch wird in folgendem Tutorial verwendet:
  https://www.neoe.io/blogs/tutorials/thermometer-und-hygrometer-mqtt-kompatibel-aufbau-variante-breadboard
  Fragen und Anregungen bitte in unserer Facebook-Gruppe adressieren, damit die gesamte Community davon profitiert. 
  https://www.facebook.com/groups/neoe.io/
  Datum der letzten Änderung: 2. November, 2020
**********************************************************************************************************************************/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <DHT.h>
#include <Wire.h>
#include <hd44780.h>                       // Haupt hd44780 Header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c Expander i/o Class Header
#include <ArduinoJson.h>

// WLAN-Zugangsdaten hier hinterlegen
const char* ssid = "NAME DES WLAN NETZWERKS"; // Anführungszeichen beibehalten
const char* password = "WLAN-PASSWORT"; // Anführungszeichen beibehalten, also z.B. so: "Geheim"

// Die für den MQTT-Server erforderlichen Daten hier hinterlegen
const char* mqtt_client = "NEOE-IOT-KIT-3-1"; // Wenn mehrere "NEOE IOT-Kits 3" im Einsatz sind, einfach mit der letzten Zahl durchnummerieren
const char* mqttServer = "IP-ADRESSE DES MQTT-SERVERS"; // Anführungszeichen beibehalten, also z.B. so: "192.168.0.236"
const uint16_t mqtt_port = 1883;
const char* mqtt_user = "BENUTZERNAME"; // Anführungszeichen beibehalten
const char* mqtt_password = "PASSWORT"; // Anführungszeichen beibehalten, also z.B. so: "Geheim"

// MQTT-Topics für Home Assistant MQTT Auto Discovery
const char* mqtt_config_topic_temperature = "homeassistant/sensor/temperatur-schlafzimmer/config";  // Name des Zimmers bei Bedarf ändern
const char* mqtt_config_topic_humidity = "homeassistant/sensor/luftfeuchtigkeit-schlafzimmer/config";  // Name des Zimmers bei Bedarf ändern
const char* mqtt_state_topic = "homeassistant/sensor/temperatur-luftfeuchtigkeit-schlafzimmer/state";  // Name des Zimmers bei Bedarf ändern

// Speicher-Reservierung für JSON-Dokument, kann mithilfe von arduinojson.org/v6/assistant eventuell noch optimiert werden
StaticJsonDocument<512> doc_config_temperature;
StaticJsonDocument<512> doc_config_humidity;
StaticJsonDocument<512> doc_state;

char mqtt_config_data_temperature[512];
char mqtt_config_data_humidity[512];
char mqtt_state_data[512];

WiFiClient espClient;
PubSubClient client(espClient);
hd44780_I2Cexp lcd; // LCD Objekt deklarieren: auto locate & auto config Expander Chip

#define DHTPIN 14       // Pin, an dem der Sensor angeschlossen wurde, z.B. D5 = DHTPIN 14
#define DHTTYPE DHT22   // Typ des DHT-Sensors

DHT dht(DHTPIN, DHTTYPE);

// Alle 10 Sekunden eine MQTT-Nachricht senden
int delay_time = 10000;

// LCD Größe definieren
const int LCD_COLS = 16;
const int LCD_ROWS = 4;

// Funktion um Werte per MQTT zu übermitteln
void publishData(float p_temperature, float p_humidity) {
  doc_state["temperature"] = p_temperature;
  doc_state["humidity"] = p_humidity;
  serializeJson(doc_state, mqtt_state_data);
  client.publish(mqtt_state_topic, mqtt_state_data);
}

// Funktion zur WLAN-Verbindung
void setup_wifi() {
  delay(10);
  /* Mit WLAN verbinden */
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

// Funktion zur MQTT-Verbindung
void reconnect() {
  while (!client.connected()) {
    if (client.connect(mqtt_client, mqtt_user, mqtt_password)) {
    } else {
      delay(5000);
    }
  }
}

// Funktion für Home Assistant MQTT Auto Discovery - Temperatur
void configMqttTemperature() {
  doc_config_temperature["name"] = "Temperatur Schlafzimmer"; // Name des Zimmers bei Bedarf ändern
  doc_config_temperature["device_class"] = "temperature";
  doc_config_temperature["state_topic"] = mqtt_state_topic;
  doc_config_temperature["unit_of_measurement"] = "C";
  doc_config_temperature["value_template"] = "{{ value_json.temperature}}";
  serializeJson(doc_config_temperature, mqtt_config_data_temperature);
  client.publish(mqtt_config_topic_temperature, mqtt_config_data_temperature, true);
}

// Funktion für Home Assistant MQTT Auto Discovery - Luftfeuchtigkeit
void configMqttHumidity() {
  doc_config_humidity["name"] = "Luftfeuchtigkeit Schlafzimmer"; // Name des Zimmers bei Bedarf ändern
  doc_config_humidity["device_class"] = "humidity";
  doc_config_humidity["state_topic"] = mqtt_state_topic;
  doc_config_humidity["unit_of_measurement"] = "%";
  doc_config_humidity["value_template"] = "{{ value_json.humidity}}";
  serializeJson(doc_config_humidity, mqtt_config_data_humidity);
  client.publish(mqtt_config_topic_humidity, mqtt_config_data_humidity, true);
}

void setup() {
  lcd.begin(LCD_COLS, LCD_ROWS);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setBufferSize(512);
  dht.begin();
  if (!client.connected()) reconnect();
  configMqttTemperature();
  configMqttHumidity();
}

void loop() {

  // Falls MQTT-Verbindung nicht besteht, versuchen, diese wiederherzustellen
  if (!client.connected()) reconnect();

  // Daten vom Sensor lesen
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Daten an den MQTT-Server senden
  publishData(temperature, humidity);

  // Daten an LCD ausgeben
  lcd.setCursor(0, 0);
  lcd.print("Temp.  ");
  lcd.print(temperature);
  lcd.print(" C");
  lcd.setCursor(0, 2);
  lcd.print("Luftf. ");
  lcd.print(humidity);
  lcd.print(" %");

  delay(delay_time);

}

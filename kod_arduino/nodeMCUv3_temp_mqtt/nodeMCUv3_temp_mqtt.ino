#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <time.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// zmienne dla sensora temperatury
#define ONE_WIRE_BUS D3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

LiquidCrystal_I2C lcd(0x27, 16, 2); 

// WiFi i MQTT
const char* ssid = ""; //wifi name
const char* password = ""; // wifi pass
const char* mqtt_server = ""; // mqtt broker server address
const int mqtt_port = 8883;
const char* mqtt_user = "testowy";
const char* mqtt_password = "test";
const char* mqtt_topic = "test_topic";

// NTP do synchronizacji czasu (wymagane do TLS)
const char* ntp_server = "pool.ntp.org";
const long gmt_offset_sec = 0;
const int daylight_offset_sec = 0;

long lastMsg = 0;
const int msgInterval = 1500;

float currentTemperature = 10.0; // Początkowa temperatura
bool increasing = true;  // flaga

// Inicjalizacja klienta z certyfikatem
BearSSL::WiFiClientSecure espClient;
PubSubClient client(espClient);

// Certyfikat SSL brokera MQTT
static const char ca_cert[] PROGMEM = R"CA_MQTT_BROKER_CERT";

void setup() {
  Serial.begin(9600);
  connectToWiFi();
  sensors.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear(); 
  syncTime();
  espClient.setTrustAnchors(new BearSSL::X509List(ca_cert));  // Dodanie certyfikatu do zaufanych
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  connectToMQTT();
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int attemptCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Connecting to WiFi... Attempt: ");
    Serial.println(++attemptCount);
    Serial.print("WiFi status: ");
    Serial.println(WiFi.status());
  }
  Serial.println("Connected to WiFi");
}

void syncTime() {
  configTime(gmt_offset_sec, daylight_offset_sec, ntp_server);
  Serial.print("Synchronizing time...");
  while (time(nullptr) < 24 * 3600) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("Time synchronized");
}

void connectToMQTT() {
  while (!client.connected()) {
    String client_id = "ESP8266Client-" + String(WiFi.macAddress());
    Serial.printf("Connecting to MQTT Broker as %s...\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("Connected to MQTT broker");
      client.subscribe(mqtt_topic);
      client.publish(mqtt_topic, "Hi EMQX I'm ESP8266 ^^");
    } else {
      char err_buf[128];
      espClient.getLastSSLError(err_buf, sizeof(err_buf));
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.println(client.state());
      Serial.print("SSL error: ");
      Serial.println(err_buf);
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void loop() {
  if (!client.connected()) {
    connectToMQTT();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > msgInterval) {
    lastMsg = now;
    sensors.requestTemperatures();
    float temperature = sensors.getTempCByIndex(0);

    if (temperature != DEVICE_DISCONNECTED_C) {
      String message = String(temperature, 1);

      Serial.print("Publishing temperature: ");
      Serial.println(message);
      client.publish(mqtt_topic, message.c_str());

      // Wyświetl temperaturę na LCD
      lcd.setCursor(0, 0);
      lcd.print("Temperature: ");
      lcd.setCursor(0, 1);
      lcd.print(message);
      lcd.print(" C");
    } else {
      Serial.println("Failed to read temperature from DS18B20");
    }
  }
}

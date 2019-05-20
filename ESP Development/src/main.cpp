#include <Arduino.h>
//#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <WiFiCredentials.h>
#include <PubSubClient.h>

//pins
#define PIN_D3 0
#define PIN_D6 12

#define PIN_MOTION_IN PIN_D6

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;
WiFiClient espClient;
PubSubClient mqttClient(espClient);


void connectToWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  Serial.print("WiFi Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.printf(".");
  }
  Serial.println();
  Serial.printf("Connection to WiFi network %s established, IP-address: ", WIFI_SSID);
  Serial.println(WiFi.localIP());
}

void connectToMosquittoBroker() {
  mqttClient.setServer("192.168.0.180", 1883);
  Serial.printf("MQTT Connecting");
  for (int i = 0; i < 10; i++) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  mqttClient.connect("ESP8266Client");
  Serial.println("Connected (hopefully)");
}

/*
char* getLocalIPString() {
  IPAddress ip = WiFi.localIP();
  String ipString = String(ip[0], ip[1], )
  sprintf(ipString, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
}
*/

void setup() {
  // open serial interface for monitoring purposes
  Serial.begin(115200);
  Serial.println();
  // open I2C connection to BME @ address 0x76
  bme.begin(0x76);
  // configure motion sensor read pin
  pinMode(PIN_MOTION_IN, INPUT);

  // connect to WiFi Network
  connectToWifi();
  connectToMosquittoBroker();

  //char locIP[16] = "";
  //locIP = getLocalIPString();
  //printf("My local IP: %s\n", String(WiFi.localIP()));
}

// doesn't work at the moment
void detectMotion() {
  static bool motiondetectStillActive = false;
  if(digitalRead(PIN_MOTION_IN) && !motiondetectStillActive) {
    Serial.printf("MOTION DETECTED!\n");
    mqttClient.publish("sensors/1/motion", "MOTION DETECTED");
    motiondetectStillActive = true;
  } else if (digitalRead(PIN_MOTION_IN)) {
    motiondetectStillActive = false;
  }
}

void loop() {
  static int count = 0;
  if (count >= 5) {
    Serial.println("sending temperature to RPi");
    char temperature[10] = "";
    sprintf(temperature, "%.1f", bme.readTemperature());
    mqttClient.publish("sensors/1/temp", temperature);
    count = 0;
  }
  count++;

  Serial.printf("Temperature: %.1f°C\n", bme.readTemperature());
  Serial.printf("Altitude: %.0fm\n", bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.printf("Pressure: %.0fhPa\n", bme.readPressure() / 100.0F);
  detectMotion();
  Serial.println("");
  delay(1000);
}

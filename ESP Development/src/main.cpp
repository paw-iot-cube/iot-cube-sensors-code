#include <Arduino.h>
//#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "WiFiCredentials.h"
#include "readSensorValues.h"

#define MQTT_BROKER_IP "192.168.0.180"

//pins
#define PIN_D3 0
#define PIN_D6 12
#define FIRST_ACTUATOR_ADDRESS 20
#define HCSR_501 2
#define BME_280 10


#define PIN_ONE_WIRE D6



bool isDeviceSensor;
int deviceType = 0;

Adafruit_BME280 bme;
WiFiClient espClient;
PubSubClient mqttClient(espClient);


// prototypes
void connectToWifi();
void connectToMosquittoBroker(const char* brokerIP);
int readDeviceType();
void subscribeReceive(char* topic, byte* payload, unsigned int length);

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

  pinMode(D0, INPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(D7, INPUT);
  pinMode(D8, INPUT);

  int deviceType = readDeviceType();

  switch (deviceType) {
    case BME_280:
      bme.begin(0x76);
      break;
    case HCSR_501:
      pinMode(PIN_ONE_WIRE, INPUT);
      break;
    default:
      break;
  }
  Serial.printf("device Type %d\n", deviceType);


  // connect to WiFi Network
  connectToWifi();
  // connect to mosquitto broker
  connectToMosquittoBroker(MQTT_BROKER_IP);

  // TO DO
  //char locIP[16] = "";
  //locIP = getLocalIPString();
  //printf("My local IP: %s\n", String(WiFi.localIP()));
}

void loop() {
  mqttClient.loop();
  mqttClient.subscribe("topic");
  if (isDeviceSensor) {
    switch (deviceType) {
      case HCSR_501:
        readHCSR501(mqttClient, PIN_ONE_WIRE, "sensors/1/motion");
        break;
      case BME_280:
        readBME208(mqttClient, bme, "sensors/1/temp", "test", "test");
        break;

      default:
        break;
    }
  } else {
    switch (deviceType) {
      case 20:
        break;
      case 21:
        break;
      default:
        break;
    }
  }


  /*static int count = 0;
  if (count >= 5) {
    Serial.println("sending temperature to RPi");
    char temperature[10] = "";
    sprintf(temperature, "%.1f", bme.readTemperature());
    mqttClient.publish("sensors/1/temp", temperature);
    count = 0;
  }
  count++;


  readHCSR501();
  Serial.println("");
  delay(1000);*/
  delay(1000);
}

void connectToWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  Serial.print("WiFi Connecting");
  // sometimes stuck in while-loop; TO DO
  /*
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.printf(".");
  }
  */
  // simulating waiting for connection, hopefully established after 5s
  for (int i = 0; i < 10; i++) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.printf("Connection to WiFi network %s established, IP-address: ", WIFI_SSID);
  Serial.println(WiFi.localIP());
}

void connectToMosquittoBroker(const char* brokerIP) {
  mqttClient.setServer(brokerIP, 1883);
  Serial.printf("MQTT Connecting");
  for (int i = 0; i < 10; i++) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  mqttClient.connect("ESP8266Client");
  Serial.println("Connected (hopefully)");
  mqttClient.setCallback(subscribeReceive);
}

int readDeviceType()
{
  int deviceTypePins[5]= {D0,D5,D6,D7,D8};

  for (int i = 0; i < 5; i++) {
    if (digitalRead(deviceTypePins[i])) {
      deviceType+=pow(2,i);
    }
  }

  isDeviceSensor = (deviceType < FIRST_ACTUATOR_ADDRESS);

  return deviceType;
}

void subscribeReceive(char* topic, byte* payload, unsigned int length) {
  Serial.println("MQTT MESSAGE RECEIVED");
  Serial.printf("Topic: %s\n");
  Serial.print("Message: ");
  for(int i = 0; i < length; i ++)
  {
    Serial.print(char(payload[i]));
  }
  Serial.println();
}

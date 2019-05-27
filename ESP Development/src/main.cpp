#include <Arduino.h>
//#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <string.h>

#include "readSensorValues.h"
#include "settings.h"


#define DISCOVERY_DEVICE_TO_NODERED "discovery/device"
#define DISCOVERY_NODERED_TO_DEVICE "discovery/master"

// first actuator is at DIP-config 20
#define FIRST_ACTUATOR_ADDRESS 20

// DIP-config for different devices
#define ANALOG_IN 0
#define VEML_6070 1
#define HCSR_501 2
#define MAX_44009 3
#define BME_280 10

// pins
#define PIN_ONE_WIRE D6

// maximum length for client ID
#define CLIENT_ID_MAX_LENGTH 9


// true if device is a sensor
bool isDeviceSensor;

// specifies which device has been connected, 0-19: sensors, 20-31: actuators
int deviceType = 0;

// device ID for web interface and MQTT-communication (9 digits)
char deviceId[CLIENT_ID_MAX_LENGTH + 1] = "0";

// IP-address
char ipAddress[16] = "";

// true as long as no device ID has been received
bool waitForClientId = true;

// true if topic hasn't been generated yet
bool isTopicGenerationNeeded = true;

// sensor read interval in seconds
unsigned long sensorInterval = 10;


Adafruit_BME280 bme;
Adafruit_VEML6070 veml = Adafruit_VEML6070();
MAX44009 max44009;
WiFiClient espClient;
PubSubClient mqttClient(espClient);


// prototypes
void getLocalIPString();
void connectToWifi();
void connectToMosquittoBroker(const char* brokerIP);
void mqttHandshake(const char* deviceName);
int readDeviceType();
void discoveryReceiveId(char* topic, byte* payload, unsigned int length);
void configReceive(char* topic, byte* payload, unsigned int length);
void snooze();

void createDiscoveryMessage(char* messageBuffer, const char* name) {
  strcpy(messageBuffer, "{\"name\":\"");
  strcat(messageBuffer, name);
  strcat(messageBuffer, "\",\"ip\":\"");
  strcat(messageBuffer, ipAddress);
  strcat(messageBuffer, "\"}");
}

void setup() {
  // open serial interface for monitoring purposes
  Serial.begin(115200);
  Serial.println();

  pinMode(D0, INPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(D7, INPUT);
  pinMode(D8, INPUT);

  // connect to WiFi Network
  connectToWifi();
  getLocalIPString();

  int deviceType = readDeviceType();
  char discoveryMessage[100] = "";

  switch (deviceType) {
    case ANALOG_IN:
      createDiscoveryMessage(discoveryMessage, "ANALOG");
      break;
    case VEML_6070:
      veml.begin(VEML6070_1_T);
      createDiscoveryMessage(discoveryMessage, "VEML6070");
      break;
    case BME_280:
      bme.begin(0x76);
      createDiscoveryMessage(discoveryMessage, "BME280");
      break;
    case MAX_44009:
      Wire.begin();
      Serial.print("max44009: ");
      Serial.print(max44009.begin());
      Serial.println();
      createDiscoveryMessage(discoveryMessage, "MAX44009");
      break;
    case HCSR_501:
      pinMode(PIN_ONE_WIRE, INPUT);
      createDiscoveryMessage(discoveryMessage, "HCSR501");
      break;
    default:
      break;
  }
  Serial.printf("device Type %d\n", deviceType);



  // connect to mosquitto broker
  connectToMosquittoBroker(MQTT_BROKER_IP);
  // get client id
  mqttHandshake(discoveryMessage);

  // listen to config (sensor) or actuator topic
  if (isDeviceSensor) {
    mqttClient.setCallback(configReceive);
    char configTopic[40] = "config/interval/";
    strcat(configTopic, deviceId);
    mqttClient.subscribe(configTopic);
  } else {
    // TO DO
  }

  // TO DO
  //char locIP[16] = "";
  //locIP = getLocalIPString();
  //printf("My local IP: %s\n", String(WiFi.localIP()));
}

void loop() {
  if (!mqttClient.connected()) {
    // reconnect, TO DO
  }
  mqttClient.loop();
  if (isDeviceSensor) {
    switch (deviceType) {
      case ANALOG_IN:
        static char topicAnalog[30] = "sensors/analog/";
        if (isTopicGenerationNeeded) {
          strcat(topicAnalog, deviceId);
          isTopicGenerationNeeded = false;
        }
        readANALOG(mqttClient, A0, topicAnalog);
        break;
      case VEML_6070:
        static char topicUV[30] = "sensors/UV/";
        if (isTopicGenerationNeeded) {
          strcat(topicUV, deviceId);
          isTopicGenerationNeeded = false;
        }
        readVEML6070(mqttClient, veml, topicUV);
        break;
      case HCSR_501:
        static char topicMotion[30] = "sensors/motion/";
        if (isTopicGenerationNeeded) {
          strcat(topicMotion, deviceId);
          isTopicGenerationNeeded = false;
        }
        readHCSR501(mqttClient, PIN_ONE_WIRE, topicMotion);
        break;
      case MAX_44009:
        // TO DO
        //readMAX44009(mqttClient,max44009,"test");
        break;
      case BME_280:
        static char topicTemperature[30] = "sensors/temperature/";
        static char topicPressure[30] = "sensors/pressure/";
        static char topicHumidity[30] = "sensors/humidity/";

        if(isTopicGenerationNeeded) {
          strcat(topicTemperature, deviceId);
          strcat(topicPressure, deviceId);
          strcat(topicHumidity, deviceId);
          isTopicGenerationNeeded = false;
        }
        readBME208(mqttClient, bme, topicTemperature, topicPressure, topicHumidity);
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

  snooze();
}

void getLocalIPString() {
  IPAddress localIP = WiFi.localIP();
  sprintf(ipAddress, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
  Serial.printf("ip: %s\n", ipAddress);
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
  // simulating waiting for connection, hopefully established after 3s
  for (int i = 0; i < 6; i++) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.printf("Connection to WiFi network %s established, IP-address: ", WIFI_SSID);
  Serial.println(WiFi.localIP());
}

void connectToMosquittoBroker(const char* brokerIP) {
  mqttClient.setServer(brokerIP, 1883);
  // simulating waiting for connection, hopefully established after 3s
  Serial.printf("MQTT Connecting");
  for (int i = 0; i < 6; i++) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  mqttClient.connect("ESP8266Client");
  Serial.println("Connected");
}

void mqttHandshake(const char* deviceName) {
  mqttClient.setCallback(discoveryReceiveId);
  mqttClient.subscribe(DISCOVERY_NODERED_TO_DEVICE);

  mqttClient.publish(DISCOVERY_DEVICE_TO_NODERED, deviceName);

  while(waitForClientId){
    mqttClient.loop();
    Serial.printf(".");
    delay(500);
  }
  Serial.println();
}

int readDeviceType() {
  int deviceTypePins[5]= {D0,D5,D6,D7,D8};

  for (int i = 0; i < 5; i++) {
    Serial.printf("%d ", digitalRead(deviceTypePins[i]));
    if (digitalRead(deviceTypePins[i])) {
      deviceType+=pow(2,i);
    }
  }
  Serial.println();

  isDeviceSensor = (deviceType < FIRST_ACTUATOR_ADDRESS);

  return deviceType;
}

void discoveryReceiveId(char* topic, byte* payload, unsigned int length) {
  #ifdef DEBUG
  Serial.println("MQTT MESSAGE RECEIVED");
  Serial.printf("Topic: %s\n", topic);
  Serial.printf("Length: %d\n", length);
  Serial.print("Message: ");
  #endif

  for (unsigned int i = 0; i < length; i++)
  {
    if (i < CLIENT_ID_MAX_LENGTH) {
      deviceId[i] = char(payload[i]);
    } else {
      // TO DO
      // error: ID too long
    }
    Serial.print(char(payload[i]));
  }
  if(length > CLIENT_ID_MAX_LENGTH) {
    deviceId[CLIENT_ID_MAX_LENGTH] = '\0';
  } else {
    deviceId[length] = '\0';
  }
  Serial.println();
  waitForClientId = false;
}

void configReceive(char* topic, byte* payload, unsigned int length) {
  Serial.printf("config received, message: ");
  unsigned long intervalSeconds = 0;
  for (unsigned int i = 0; i < length; i++) {
    Serial.print(char(payload[i]));
    intervalSeconds += (char(payload[i]) - '0') * pow( 10, length - (i+1) );
  }
  Serial.println();
  Serial.printf("Interval in seconds: %lu\n", intervalSeconds);
  sensorInterval = intervalSeconds;
}

void snooze() {
  for (unsigned long i = 0; i < sensorInterval; i++) {
    mqttClient.loop();
    delay(1000);
  }
}

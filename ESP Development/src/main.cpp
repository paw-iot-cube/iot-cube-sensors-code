#include <Arduino.h>
//#include <Wire.h>
//#include <Adafruit_Sensor.h>
//#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <string.h>

#include <setActuators.h>
#include <controlLed.h>
#include <readSensorValues.h>
#include <settings.h>


#define DISCOVERY_DEVICE_TO_NODERED "discovery/device"
#define DISCOVERY_NODERED_TO_DEVICE "discovery/master"

// maximum length for client ID
#define CLIENT_ID_MAX_LENGTH 9

// DIP-pins
#define DIP_PIN_1_LSB D0
#define DIP_PIN_2 D5
#define DIP_PIN_3 D6
#define DIP_PIN_4 D7
#define DIP_PIN_5_MSB RX


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
bool isFirstIteration = true;

// sensor read interval in seconds
unsigned long sensorInterval = 10;

//inintialisation of sensors and clients
Adafruit_BME280 bme;
Adafruit_VEML6070 veml = Adafruit_VEML6070();
MAX44009 max44009;
UltraSonicDistanceSensor hcsr(PIN_TRIGGER_HCSR_04, PIN_ECHO_HCSR_04);
Adafruit_CCS811 ccs;
BlueDot_BME280_TSL2591 bluedotBme;
BlueDot_BME280_TSL2591 tsl2591;
DHT dht(PIN_ONE_WIRE, DHT22);
Adafruit_MPR121 mpr121 = Adafruit_MPR121();

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
void commandReceive(char* topic, byte* payload, unsigned int length);
void snooze(bool isBufferingEnabled, bool* buffer);
void createDiscoveryMessage(char* messageBuffer, const char* name);

void setup() {

  pinMode(RX, FUNCTION_3);

  pinMode(DIP_PIN_1_LSB, INPUT);
  pinMode(DIP_PIN_2, INPUT);
  pinMode(DIP_PIN_3, INPUT);
  pinMode(DIP_PIN_4, INPUT);
  pinMode(DIP_PIN_5_MSB, INPUT);

  #ifdef DEBUG
    // open serial interface for monitoring purposes
    Serial.begin(115200);
    Serial.println();
  #else
    // enable status LED
    pinMode(PIN_LED_RED, FUNCTION_3);
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_BLUE, FUNCTION_3);
    pinMode(PIN_LED_BLUE, OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);
  #endif

  setStatusLed(INITIALISATION);

  // connect to WiFi Network
  connectToWifi();
  getLocalIPString();

  deviceType = readDeviceType();
  char discoveryMessage[100] = "";

  switch (deviceType) {
    case ANALOG_IN:
      createDiscoveryMessage(discoveryMessage, "ANALOG");
      break;
    case VEML_6070:
      veml.begin(VEML6070_1_T);
      createDiscoveryMessage(discoveryMessage, "VEML6070");
      break;
    case HCSR_501:
      pinMode(PIN_ONE_WIRE, INPUT);
      createDiscoveryMessage(discoveryMessage, "HCSR501");
      break;
    case MAX_44009:
      Wire.begin();
      max44009.begin();
      createDiscoveryMessage(discoveryMessage, "MAX44009");
      break;
    case DHT_22:
      dht.begin();
      createDiscoveryMessage(discoveryMessage, "DHT22");
      break;
    case BME_280:
      bme.begin(0x76);
      createDiscoveryMessage(discoveryMessage, "BME280");
      break;
    case HCSR_04:
      createDiscoveryMessage(discoveryMessage, "HCSR04");
      break;

    case CCS_811:
      ccs.begin();
      createDiscoveryMessage(discoveryMessage, "CCS811");
      break;

    case BLUEDOT_BME_280_TSL_2591:
      bluedotBme.parameter.I2CAddress = 0x77;
      tsl2591.parameter.I2CAddress = 0x29;
      Wire.begin();
      tsl2591.parameter.gain = 0b01;
      tsl2591.parameter.integration = 0b000;
      tsl2591.config_TSL2591();
      bluedotBme.parameter.IIRfilter = 0b100;
      bluedotBme.parameter.sensorMode = 0b11;
      bluedotBme.parameter.humidOversampling = 0b101;
      bluedotBme.parameter.tempOversampling = 0b101;
      bluedotBme.parameter.pressOversampling = 0b101;
      bluedotBme.parameter.pressureSeaLevel = 1013.25;
      bluedotBme.parameter.tempOutsideCelsius = 15;
      bluedotBme.init_BME280();;
      tsl2591.init_TSL2591();
      createDiscoveryMessage(discoveryMessage, "BLUEDOT");
      break;

    case BUTTON:
      pinMode(PIN_ONE_WIRE, INPUT);
      createDiscoveryMessage(discoveryMessage, "BUTTON");
      break;

    case MPR_121:
      if (!mpr121.begin(0x5B)) {
        mpr121.begin(0x5A);
      }
      createDiscoveryMessage(discoveryMessage, "MPR121");
      break;

    case LED:
      pinMode(PIN_ONE_WIRE, OUTPUT);
      createDiscoveryMessage(discoveryMessage, "LED");
      break;
    default:
      setStatusLed(DIP_ERROR);
      break;
  }

  #ifdef DEBUG
    Serial.printf("device Type %d\n", deviceType);
    Serial.printf("discovery message: %s\n", discoveryMessage);
  #endif

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
  }
  else {
    mqttClient.setCallback(commandReceive);
    char commandTopic[40] = "actuators/";
    switch (deviceType) {
      case LED:
        strcat(commandTopic,"LED/");
        strcat(commandTopic,deviceId);
        mqttClient.subscribe(commandTopic);
        break;
      default:
        setStatusLed(DIP_ERROR);
        break;
    }
  }
  setStatusLed(C0NNECTED);
}

bool isBufferingNeeded(int deviceType) {
  switch(deviceType) {
    case HCSR_501:
    case BUTTON:
      return true;
      break;
    default:
      return false;
      break;
  }
}

void loop() {
  #ifdef DEBUG
    Serial.printf("ID: %s\n", deviceId);
  #endif
  if (!mqttClient.connected()) {
    connectToMosquittoBroker(MQTT_BROKER_IP);
  }

  static bool isBufferingEnabled = false;
  static bool booleanSignalBuffer = false;
  if (isFirstIteration && isBufferingNeeded(deviceType)) {
    isBufferingEnabled = true;
  }

  mqttClient.loop();
  if (isDeviceSensor){
    switch (deviceType) {
      case ANALOG_IN:
        static char topicAnalog[30] = "sensors/analog/";
        if (isFirstIteration) {
          strcat(topicAnalog, deviceId);
        }
        readANALOG(mqttClient, A0, topicAnalog);
        break;

      case VEML_6070:
        static char topicUV[30] = "sensors/UV/";
        if (isFirstIteration) {
          strcat(topicUV, deviceId);
        }
        readVEML6070(mqttClient, veml, topicUV);
        break;

      case HCSR_501:
        static char topicMotion[30] = "sensors/motion/";
        if (isFirstIteration) {
          strcat(topicMotion, deviceId);
        }
        readHCSR501(mqttClient, &booleanSignalBuffer, topicMotion);
        break;

      case MAX_44009:
        // TO DO
        readMAX44009(mqttClient,max44009,"test");
        break;

      case DHT_22:
        static char topicTemperatureDHT[30] = "sensors/temperature/";
        static char topicHumidityDHT[30] = "sensors/humidity/";
        if(isFirstIteration) {
          strcat(topicTemperatureDHT, deviceId);
          strcat(topicHumidityDHT, deviceId);
        }
        readDHT22(mqttClient, dht, topicTemperatureDHT, topicHumidityDHT);
        break;

      case BME_280:
        static char topicTemperature[30] = "sensors/temperature/";
        static char topicPressure[30] = "sensors/pressure/";
        static char topicHumidity[30] = "sensors/humidity/";

        if(isFirstIteration) {
          strcat(topicTemperature, deviceId);
          strcat(topicPressure, deviceId);
          strcat(topicHumidity, deviceId);
        }
        readBME208(mqttClient, bme, topicTemperature, topicPressure, topicHumidity);
        break;

      case HCSR_04:
        static char topicDistance[30] = "sensors/distance/";
        if(isFirstIteration) {
          strcat(topicDistance, deviceId);
        }
        readHCSR04(mqttClient, hcsr, topicDistance);
        break;

      case CCS_811:
        static char topicVOC[30] = "sensors/VOC/";
        static char topicCO2[30] = "sensors/CO2/";

        if(isFirstIteration) {
          strcat(topicVOC, deviceId);
          strcat(topicCO2, deviceId);
        }
        readCCS811(mqttClient, ccs, topicVOC, topicCO2);
        break;

      case BLUEDOT_BME_280_TSL_2591:
        static char topicTemperatureBluedot[30] = "sensors/temperature/";
        static char topicPressureBluedot[30] = "sensors/pressure/";
        static char topicHumidityBluedot[30] = "sensors/humidity/";
        static char topicIlluminanceBluedot [30] = "sensors/luminous/";

        if(isFirstIteration) {
          strcat(topicTemperatureBluedot, deviceId);
          strcat(topicPressureBluedot, deviceId);
          strcat(topicHumidityBluedot, deviceId);
          strcat(topicIlluminanceBluedot, deviceId);
        }
        readBLUEDOT(mqttClient,bluedotBme, tsl2591, topicTemperatureBluedot, topicPressureBluedot, topicHumidityBluedot, topicIlluminanceBluedot);
        break;

      case BUTTON:
        static char topicButton[30] = "sensors/button/";
        if (isFirstIteration) {
          strcat(topicButton, deviceId);
        }
        readBUTTON(mqttClient, &booleanSignalBuffer, topicButton);
        break;

      case MPR_121:
        static char topicTouched[30] = "sensors/E-field/";
        if(isFirstIteration) {
          strcat(topicTouched, deviceId);
        }
        readMPR121(mqttClient, mpr121, topicTouched);
        break;

      default:
        break;
    }
  }

  isFirstIteration = false;
  snooze(isBufferingEnabled, &booleanSignalBuffer);
}

void getLocalIPString() {
  IPAddress localIP = WiFi.localIP();
  sprintf(ipAddress, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
}

void connectToWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  #ifdef DEBUG
    Serial.print("WiFi Connecting");
  #endif
  // sometimes stuck in while-loop; TO DO
  int counterTimeout = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (counterTimeout > 40) {
      setStatusLed(WLAN_ERROR);
    }
    delay(500);
    #ifdef DEBUG
      Serial.printf(".");
    #endif
    counterTimeout++;
  }
  #ifdef DEBUG
    Serial.println();
    Serial.printf("Connection to WiFi network %s established, IP-address: ", WIFI_SSID);
    Serial.println(WiFi.localIP());
  #endif
}

String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

void connectToMosquittoBroker(const char* brokerIP) {

  mqttClient.setServer(brokerIP, 1883);

  char MQTT_Name[30] = "ESP_";
  strcat(MQTT_Name, WiFi.macAddress().c_str());

  #ifdef DEBUG
    Serial.printf("MQTT Name: %s\n", MQTT_Name);
    Serial.printf("MQTT Connecting");
  #endif

  if(!mqttClient.connect(MQTT_Name)){
    setStatusLed(MQTT_ERROR);
  }

  #ifdef DEBUG
    Serial.println();
    Serial.println("Connected");
  #endif
}

void mqttHandshake(const char* deviceName) {
  #ifdef DEBUG
    Serial.print("Executing MQTT-Handshake, waiting for ID");
  #endif

  mqttClient.setCallback(discoveryReceiveId);
  mqttClient.subscribe(DISCOVERY_NODERED_TO_DEVICE);

  mqttClient.publish(DISCOVERY_DEVICE_TO_NODERED, deviceName);
  int counterTimeout = 0;
  while(waitForClientId){
    if (counterTimeout > 40) {
      setStatusLed(ID_ERROR);
    }
    mqttClient.loop();
    #ifdef DEBUG
      Serial.printf(".");
    #endif
    delay(500);
    counterTimeout++;
  }

  mqttClient.unsubscribe(DISCOVERY_NODERED_TO_DEVICE);

  #ifdef DEBUG
    Serial.println();
    Serial.printf("received ID: %s\n", deviceId);
  #endif
}

int readDeviceType() {
  int deviceTypePins[5]= {
    DIP_PIN_1_LSB,
    DIP_PIN_2,
    DIP_PIN_3,
    DIP_PIN_4,
    DIP_PIN_5_MSB
  };

  #ifdef DEBUG
    Serial.print("DIP-configuration: ");
  #endif

  for (int i = 0; i < 5; i++) {
    #ifdef DEBUG
      Serial.printf("%d ", digitalRead(deviceTypePins[i]));
    #endif
    if (digitalRead(deviceTypePins[i])) {
      deviceType+=pow(2,i);
    }
  }

  #ifdef DEBUG
    Serial.println();
  #endif

  isDeviceSensor = (deviceType < FIRST_ACTUATOR_ADDRESS);

  return deviceType;
}

void discoveryReceiveId(char* topic, byte* payload, unsigned int length) {
  #ifdef VERBOSE
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
    #ifdef VERBOSE
      Serial.print(char(payload[i]));
    #endif
  }
  if(length > CLIENT_ID_MAX_LENGTH) {
    deviceId[CLIENT_ID_MAX_LENGTH] = '\0';
  } else {
    deviceId[length] = '\0';
  }
  #ifdef VERBOSE
    Serial.println();
  #endif
  waitForClientId = false;
}

void configReceive(char* topic, byte* payload, unsigned int length) {
  #ifdef VERBOSE
    Serial.println("MQTT MESSAGE RECEIVED");
    Serial.printf("Topic: %s\n", topic);
    Serial.printf("Length: %d\n", length);
    Serial.print("Message: ");
  #endif
  unsigned long intervalSeconds = 0;
  for (unsigned int i = 0; i < length; i++) {
    #ifdef VERBOSE
      Serial.print(char(payload[i]));
    #endif
    intervalSeconds += (char(payload[i]) - '0') * pow( 10, length - (i+1) );
  }
  #ifdef VERBOSE
    Serial.println();
  #endif

  #ifdef DEBUG
    Serial.printf("Interval configuration received. New interval in seconds: %lu\n", intervalSeconds);
  #endif
  sensorInterval = intervalSeconds;
}

void commandReceive(char* topic, byte* payload, unsigned int length){
  char payloadString[length+1];
  for(unsigned int i = 0; i < length; i++) {
    payloadString[i] = char(payload[i]);
  }
  payloadString[length] = '\0';
  #ifdef DEBUG
    Serial.printf("Command received: %s\n", payloadString);
  #endif

  switch (deviceType) {
    case LED:
      setLed(PIN_ONE_WIRE, (bool) atoi(payloadString));
      break;
    default:
      break;
  }
}

void snooze(bool isBufferingEnabled, bool* buffer) {
  for (unsigned long i = 0; i < sensorInterval; i++) {
    mqttClient.loop();
    if (isBufferingEnabled) {
      for (int j = 0; j < 10; j++) {
        mqttClient.loop();
        if (digitalRead(PIN_ONE_WIRE)) {
          *buffer = true;
        }
        delay(100);
      }
    } else {
      delay(1000);
    }
  }
}

void createDiscoveryMessage(char* messageBuffer, const char* name) {
  strcpy(messageBuffer, "{\"name\":\"");
  strcat(messageBuffer, name);
  strcat(messageBuffer, "\",\"ip\":\"");
  strcat(messageBuffer, ipAddress);
  strcat(messageBuffer, "\"}");
}

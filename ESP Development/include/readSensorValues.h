#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_VEML6070.h>
#include <PubSubClient.h>
#include <MAX44009.h>
#include <DHT.h>
#include <DHT_U.h>
#ifndef READSENSORVALUES_H
#define READSENSORVALUES_H
void readANALOG(PubSubClient mqtt, int pin, const char* topic);
void readVEML6070(PubSubClient mqtt, Adafruit_VEML6070 veml, const char* topic);
void readHCSR501(PubSubClient mqtt, int pin, const char* topic);
void readMAX44009(PubSubClient mqtt, MAX44009 max44009, const char* topic);
void readDHT22(PubSubClient mqtt,DHT dht, const char* tempTopic, const char* humidTopic);
void readBME208(PubSubClient mqtt, Adafruit_BME280 bme, const char* tempTopic, const char* pressTopic, const char* humidTopic);
#endif

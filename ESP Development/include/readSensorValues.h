#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_VEML6070.h>
#include <PubSubClient.h>
#include <MAX44009.h>
#ifndef READSENSORVALUES_H
#define READSENSORVALUES_H
void readHCSR501(PubSubClient mqtt, int pin, const char* topic);
void readANALOG(PubSubClient mqtt, int pin, const char* topic);
void readBME208(PubSubClient mqtt, Adafruit_BME280 bme, const char* tempTopic, const char* pressTopic, const char* humidTopic);
void readVEML6070(PubSubClient mqtt, Adafruit_VEML6070 veml, const char* topic);
void readMAX44009(PubSubClient mqtt, MAX44009 max44009, const char* topic);
#endif

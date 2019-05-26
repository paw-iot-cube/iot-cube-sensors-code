#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <PubSubClient.h>

#ifndef READSENSORVALUES_H
#define READSENSORVALUES_H

void readHCSR501(PubSubClient mqtt, int pin, const char* topic);
void readBME208(PubSubClient mqtt, Adafruit_BME280 bme, const char* tempTopic, const char* pressTopic, const char* humidTopic);

#endif

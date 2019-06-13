#ifndef SETACTUATORS_H
#define SETACTUATORS_H
#include <PubSubClient.h>

// first actuator is at DIP-config 20
#define FIRST_ACTUATOR_ADDRESS 20

// actuator DIP-configurations
#define LED 20

void setLed(int led, bool status);

#endif

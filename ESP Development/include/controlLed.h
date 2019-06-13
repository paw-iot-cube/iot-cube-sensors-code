#ifndef CONTROLLED_H
#define CONTROLLED_H

// colours for Led
#define LED_RED 1
#define LED_GREEN 2
#define LED_BLUE 3
#define LED_PURPLE 4
#define LED_WHITE 5
#define LED_OFF 6

//pins
#define PIN_LED_RED TX
#define PIN_LED_GREEN D3
#define PIN_LED_BLUE D8


// status codes
#define INITIALISATION 0
#define C0NNECTED 1
#define WLAN_ERROR 2
#define MQTT_ERROR 3
#define ID_ERROR 4
#define DIP_ERROR 5

void setStatusLed(int status);

#endif

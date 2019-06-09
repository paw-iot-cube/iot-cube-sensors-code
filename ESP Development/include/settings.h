#ifndef SETTINGS_H
#define SETTINGS_H

// uncomment to enable debug messages
#define DEBUG

// uncomment for detailed information on received MQTT-messages
//#define VERBOSE

// mqtt-broker IP
#define MQTT_BROKER_IP "192.168.0.180"
// TO DO: find another way of configuring this

// standard read intervals for different sensors in ms
#define STD_INTERVAL_MS_BME280 10000
#define STD_INTERVAL_MS_HCSR501 10000

// wifi credentials
#define WIFI_SSID "FRITZ!Box 7490"
#define WIFI_PASSWD "26489093048613568547"
// TO DO: find another way of configuring and storing these

#endif

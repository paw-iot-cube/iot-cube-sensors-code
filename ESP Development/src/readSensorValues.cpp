#include "readSensorValues.h"
#include "debug.h"

#define SEALEVELPRESSURE_HPA (1013.25)

void readHCSR501(PubSubClient mqtt, int pin, const char* topic) {
  static bool motionDetected;
  motionDetected = digitalRead(pin);
  mqtt.publish(topic, (motionDetected)?"1":"0");
  #ifdef DEBUG
    Serial.printf("motion detected: %d\n", motionDetected);
  #endif
  /*
  static bool motiondetectStillActive = false;
  if(digitalRead(pin) && !motiondetectStillActive) {
    Serial.printf("MOTION DETECTED!\n");
    //mqttClient.publish("sensors/1/motion", "MOTION DETECTED");
    motiondetectStillActive = true;
  } else if (digitalRead(pin)) {
    motiondetectStillActive = false;
  }
  */
}

void readBME208(PubSubClient mqtt, Adafruit_BME280 bme, const char* tempTopic, const char* pressTopic, const char* humidTopic) {
  static float temperature, pressure, humidity;
  temperature = bme.readTemperature();
  pressure = bme.readPressure();
  humidity = bme.readHumidity();

  char tempString[7] = "";
  sprintf(tempString, "%.2f", temperature);
  mqtt.publish(tempTopic, tempString);
  #ifdef DEBUG
    Serial.printf("Temperature: %.1f°C\n", temperature);
  #endif

  char pressString[7] = "";
  sprintf(pressString, "%.0f", pressure);
  mqtt.publish(pressTopic, pressString);
  #ifdef DEBUG
    Serial.printf("Pressure: %.2fhPa\n", pressure / 100.0F);
  #endif

  char humidString[7] = "";
  sprintf(humidString, "%.2f", humidity);
  mqtt.publish(humidTopic, humidString);
  #ifdef DEBUG
    Serial.printf("Humidity: %.2f%%\n", humidity);
  #endif

  /*
  Serial.printf("Temperature: %.1f°C\n", bme.readTemperature());
  Serial.printf("Altitude: %.0fm\n", bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.printf("Pressure: %.0fhPa\n", bme.readPressure() / 100.0F);
  */
}

void readANALOG(PubSubClient mqtt, int pin, const char* topic) {
  static float voltage = 0;
  voltage = ((analogRead(pin)/1024)*3.3);
  char voltageString[7] = "";
  sprintf(voltageString,"%f",voltage);
  mqtt.publish(topic, voltageString);
  #ifdef DEBUG
    Serial.printf("Analog Input: %.2fV\n",voltage);
  #endif
}

void readVEML6070(PubSubClient mqtt, Adafruit_VEML6070 veml, const char* topic) {
  static unsigned int uv;
  uv = veml.readUV();
  char uvString[7] = "";
  sprintf(uvString, "%u",uv);
  mqtt.publish(topic,uvString);
  #ifdef DEBUG
    Serial.printf("UVintensity : %u\n", uv);
  #endif
}


void readMAX44009(PubSubClient mqtt, MAX44009 max44009, const char* topic){
  static unsigned int light;
  light = max44009.get_lux();
  char lightString[7] = "";
  sprintf(lightString, "%u",light);
  mqtt.publish(topic,lightString);
  #ifdef DEBUG
    Serial.printf("Light : %u\n lumen", light);
  #endif
}

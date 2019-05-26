#include "readSensorValues.h"
#include "settings.h"

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

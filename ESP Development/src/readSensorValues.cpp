#include "readSensorValues.h"
#include "settings.h"

#define SEALEVELPRESSURE_HPA (1013.25)

void readANALOG(PubSubClient mqtt, int pin, const char* topic) {
  static float voltage = 0;
  voltage = ((analogRead(pin)/1024.0)*3.3);
  static char voltageString[7] = "";
  sprintf(voltageString,"%.2f",voltage);
  mqtt.publish(topic, voltageString);
  #ifdef DEBUG
    Serial.printf("Analog Input: %.2fV\n",voltage);
  #endif
}

void readVEML6070(PubSubClient mqtt, Adafruit_VEML6070 veml, const char* topic) {
  static unsigned int uv;
  static float uvIndex;
  uv = veml.readUV();
  uvIndex = (float) uv * (5.0/1300);
  static char uvString[7] = "";
  sprintf(uvString, "%.2f",uvIndex);
  mqtt.publish(topic,uvString);
  #ifdef DEBUG
    Serial.printf("UVintensity : %.2f\n", uvIndex);
  #endif
}

void readHCSR501(PubSubClient mqtt, int pin, const char* topic) {
  static bool motionDetected;
  motionDetected = digitalRead(pin);
  mqtt.publish(topic, (motionDetected)?"1":"0");
  #ifdef DEBUG
    Serial.printf("motion detected: %d\n", motionDetected);
  #endif
}

void readMAX44009(PubSubClient mqtt, MAX44009 max44009, const char* topic){
  static unsigned int light;
  light = max44009.get_lux();
  static char lightString[7] = "";
  sprintf(lightString, "%.2u",light);
  mqtt.publish(topic,lightString);
  #ifdef DEBUG
    Serial.printf("Light : %.2u\n lumen", light);
  #endif
}

void readDHT22(PubSubClient mqtt,DHT dht, const char* tempTopic, const char* humidTopic){
  static float humidity, temperature;
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  static char humidString [7] = "";
  sprintf(humidString, "%.2f", humidity);
  mqtt.publish(humidTopic, humidString);

  static char tempString [7] = "";
  sprintf(tempString, "%.2f", temperature);
  mqtt.publish(tempTopic, tempString);

  #ifdef DEBUG
    Serial.printf("Humidity: %.2f%%\n", humidity);
    Serial.printf("Temperature: %.1f°C\n", temperature);
  #endif
}

void readBME208(PubSubClient mqtt, Adafruit_BME280 bme, const char* tempTopic, const char* pressTopic, const char* humidTopic) {
  static float temperature, pressure, humidity;
  temperature = bme.readTemperature();
  pressure = bme.readPressure();
  humidity = bme.readHumidity();

  static char tempString[7] = "";
  sprintf(tempString, "%.2f", temperature);
  mqtt.publish(tempTopic, tempString);
  #ifdef DEBUG
    Serial.printf("Temperature: %.1f°C\n", temperature);
  #endif

  static char pressString[7] = "";
  sprintf(pressString, "%.0f", pressure);
  mqtt.publish(pressTopic, pressString);
  #ifdef DEBUG
    Serial.printf("Pressure: %.2fhPa\n", pressure / 100.0F);
  #endif

  static char humidString[7] = "";
  sprintf(humidString, "%.2f", humidity);
  mqtt.publish(humidTopic, humidString);
  #ifdef DEBUG
    Serial.printf("Humidity: %.2f%%\n", humidity);
  #endif
}

void readHCSR04(PubSubClient mqtt, UltraSonicDistanceSensor hcsr, const char* distanceTopic){
  static double distance;
  distance = hcsr.measureDistanceCm();
  char distanceString[32];
  snprintf(distanceString, sizeof(distanceString), "%.2g", distance);
  mqtt.publish(distanceTopic,distanceString);
  #ifdef DEBUG
    Serial.printf("Distance: %.2lf cm\n", distance);
  #endif
}

void readCCS811(PubSubClient mqtt, Adafruit_CCS811 ccs, const char* VOCTopic, const char* CO2Topic){
  static float VOC, CO2;
  if(!ccs.readData()){
    VOC = ccs.getTVOC();
    CO2 = ccs.geteCO2();
  }
  static char vocString[7] = "";
  sprintf(vocString, "%.2f", VOC);
  mqtt.publish(VOCTopic, vocString);
  #ifdef DEBUG
    Serial.printf("VOC: %.2f ppb\n", VOC);
  #endif

  static char co2String[7] = "";
  sprintf(co2String, "%.2f", CO2);
  mqtt.publish(CO2Topic, co2String);
  #ifdef DEBUG
    Serial.printf("CO2: %.2f ppb\n", CO2);
  #endif
}

void readBLUEDOT(PubSubClient mqtt, BlueDot_BME280_TSL2591 bluedotBme, BlueDot_BME280_TSL2591 tsl2591, const char* tempTopic, const char* pressTopic, const char* humidTopic, const char* illuminanceTopic){
  static float temperature, pressure, humidity, illuminance;
  temperature = bluedotBme.readTempC();
  pressure = (bluedotBme.readPressure())*100;
  humidity = bluedotBme.readHumidity();
  illuminance = tsl2591.readIlluminance_TSL2591();

  static char tempString[7] = "";
  sprintf(tempString, "%.2f", temperature);
  mqtt.publish(tempTopic, tempString);
  #ifdef DEBUG
    Serial.printf("Temperature: %.1f°C\n", temperature);
  #endif

  static char pressString[7] = "";
  sprintf(pressString, "%.0f", pressure);
  mqtt.publish(pressTopic, pressString);
  #ifdef DEBUG
    Serial.printf("Pressure: %.2fhPa\n", pressure);
  #endif

  static char humidString[7] = "";
  sprintf(humidString, "%.2f", humidity);
  mqtt.publish(humidTopic, humidString);
  #ifdef DEBUG
    Serial.printf("Humidity: %.2f%%\n", humidity);
  #endif

  static char illuminanceString[7] = "";
  sprintf(illuminanceString, "%.2f", illuminance);
  mqtt.publish(illuminanceTopic, illuminanceString);
  #ifdef DEBUG
    Serial.printf("Illuminance: %.2f lux \n", illuminance);
  #endif
}

void readBUTTON(PubSubClient mqtt, int pin, const char* buttonTopic){
  static bool isButtonPressed;
  isButtonPressed = digitalRead(pin);
  #ifdef DEBUG
    Serial.printf("topic: %s, isButtonPressed: %d\n", buttonTopic, isButtonPressed);
  #endif
  static bool status = mqtt.publish(buttonTopic, (isButtonPressed)?"1":"0");
  #ifdef DEBUG
    Serial.printf("Button pressed: %d\n", isButtonPressed);
    Serial.printf("Send message: %d\n", status);
  #endif
}

void readMPR121(PubSubClient mqtt, Adafruit_MPR121 mpr121, const char* touchTopic){
  static uint16_t lasttouched, currtouched;
  currtouched = mpr121.touched();
  #ifdef DEBUG
  for (uint8_t i=0; i<12; i++) {
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" touched");
    }
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" released");
    }
  }
  Serial.printf("Wert: %u",currtouched);
  lasttouched = currtouched;
  #endif DEBUG
  static char touchedString[6] = "";
  sprintf(touchedString,"%u", currtouched);
  mqtt.publish(touchTopic,touchedString);
}

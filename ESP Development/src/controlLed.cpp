#include <controlLed.h>
#include <Arduino.h>
#include <settings.h>

// private functions
void setLedColour(int led);

void setLedColour(int led) {
  #ifndef DEBUG
  switch (led) {
    case LED_RED:
      digitalWrite(PIN_LED_GREEN, LOW);
      digitalWrite(PIN_LED_BLUE, LOW);
      digitalWrite(PIN_LED_RED, HIGH);
      break;
    case LED_GREEN:
      digitalWrite(PIN_LED_GREEN, HIGH);
      digitalWrite(PIN_LED_BLUE, LOW);
      digitalWrite(PIN_LED_RED, LOW);
      break;
    case LED_BLUE:
      digitalWrite(PIN_LED_GREEN, LOW);
      digitalWrite(PIN_LED_BLUE, HIGH);
      digitalWrite(PIN_LED_RED, LOW);
      break;
    case LED_PURPLE:
      digitalWrite(PIN_LED_GREEN, LOW);
      digitalWrite(PIN_LED_BLUE, HIGH);
      digitalWrite(PIN_LED_RED, HIGH);
      break;
    case LED_WHITE:
      digitalWrite(PIN_LED_GREEN, HIGH);
      digitalWrite(PIN_LED_BLUE, HIGH);
      digitalWrite(PIN_LED_RED, HIGH);
      break;
    case LED_OFF:
      digitalWrite(PIN_LED_GREEN, LOW);
      digitalWrite(PIN_LED_BLUE, LOW);
      digitalWrite(PIN_LED_RED, LOW);
    default:
    break;
  }
  #endif
}

void setStatusLed(int status)
{
  switch (status) {
    case INITIALISATION:
      #ifdef DEBUG
        Serial.println("[STATUS] Initialisation started");
      #else
        setLedColour(LED_BLUE);
      #endif
      break;

    case C0NNECTED:
      #ifdef DEBUG
        Serial.println("[STATUS] Initialisation completed");
      #else
        setLedColour(LED_GREEN);
      #endif
      break;

    case WLAN_ERROR:
      #ifdef DEBUG
        Serial.println("[ERROR] WiFi connection failed");
        while(true) {
          delay(10000);
        }
      #else
        while (true) {
          setLedColour(LED_PURPLE);
          delay(1250);
          setLedColour(LED_OFF);
          delay(1250);
        }
      #endif
      break;

    case MQTT_ERROR:
      #ifdef DEBUG
        Serial.println("[ERROR] MQTT connection failed");
        while(true) {
          delay(10000);
        }
      #else
        while (true) {
          setLedColour(LED_PURPLE);
          delay(70);
          setLedColour(LED_OFF);
          delay(70);
        }
      #endif
      break;

    case ID_ERROR:
      #ifdef DEBUG
        Serial.println("[ERROR] timeout while waiting for ID");
        while(true) {
          delay(10000);
        }
      #else
        while (true) {
          setLedColour(LED_RED);
          delay(70);
          setLedColour(LED_OFF);
          delay(70);
        }
      #endif
      break;

    case DIP_ERROR:
      #ifdef DEBUG
        Serial.println("[ERROR] unknown DIP-configuration");
        while(true) {
          delay(10000);
        }
      #else
        while (true) {
          setLedColour(LED_RED);
          delay(1250);
          setLedColour(LED_OFF);
          delay(1250);
        }
      #endif
      break;

    default:
      break;
  }
}

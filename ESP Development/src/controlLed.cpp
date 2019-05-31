#include <controlLed.h>
#include <Arduino.h>


void setLed(int led) {
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
      setLed(LED_BLUE);
      break;

    case C0NNECTED:
      setLed(LED_GREEN);
      break;

    case WLAN_ERROR:
      while (true) {
        setLed(LED_PURPLE);
        delay(1000);
        setLed(LED_OFF);
        delay(1000);
      }
      break;

    case MQTT_ERROR:
      while (true) {
        setLed(LED_PURPLE);
        delay(300);
        setLed(LED_OFF);
        delay(300);
      }
      break;

    case ID_ERROR:
      while (true) {
        setLed(LED_RED);
        delay(300);
        setLed(LED_RED);
        delay(300);
      }
      break;

    case DIP_ERROR:
      while (true) {
        setLed(LED_RED);
        delay(1000);
        setLed(LED_RED);
        delay(1000);
      }
      break;

    default:
      break;
  }
}

#include <controlLed.h>
#include <Arduino.h>


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
      setLedColour(LED_BLUE);
      break;

    case C0NNECTED:
      setLedColour(LED_GREEN);
      break;

    case WLAN_ERROR:
      while (true) {
        setLedColour(LED_PURPLE);
        delay(1250);
        setLedColour(LED_OFF);
        delay(1250);
      }
      break;

    case MQTT_ERROR:
      while (true) {
        setLedColour(LED_PURPLE);
        delay(70);
        setLedColour(LED_OFF);
        delay(70);
      }
      break;

    case ID_ERROR:
      while (true) {
        setLedColour(LED_RED);
        delay(70);
        setLedColour(LED_OFF);
        delay(70);
      }
      break;

    case DIP_ERROR:
      while (true) {
        setLedColour(LED_RED);
        delay(1250);
        setLedColour(LED_OFF);
        delay(1250);
      }
      break;

    default:
      break;
  }
}

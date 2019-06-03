#include <setActuators.h>

void setLed(int led, bool status) {
  digitalWrite(led, status);
}

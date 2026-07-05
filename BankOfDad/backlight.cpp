#include "backlight.h"

void backlightInit(uint8_t pin) {
  pinMode(pin, OUTPUT);
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  ledcAttach(pin, 1000, 10);
  ledcWrite(pin, 1023);
#else
  ledcSetup(0, 1000, 10);
  ledcAttachPin(pin, 0);
  ledcWrite(0, 1023);
#endif
}

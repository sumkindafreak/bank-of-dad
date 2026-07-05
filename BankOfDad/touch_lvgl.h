#pragma once

#include <TAMC_GT911.h>

/** Pass DISPLAY_ROTATION from lvgl_port.h */
void touchLvglInit(TAMC_GT911 &touch, uint16_t width, uint16_t height, uint8_t displayRotation);

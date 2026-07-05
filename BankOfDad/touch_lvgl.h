#pragma once

#include <TAMC_GT911.h>

enum Rotation {
  ROTATION_RIGHT = 1,
};

void touchLvglInit(TAMC_GT911 &touch, uint16_t width, uint16_t height, Rotation rotation);

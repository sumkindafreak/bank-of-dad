#pragma once

#include <lvgl.h>
#include <Arduino_GFX_Library.h>

#define SCREEN_W 800
#define SCREEN_H 480
#define DISPLAY_ROTATION 1

extern Arduino_RGB_Display *gfx;

bool lvglPortInit(Arduino_RGB_Display *panel, Arduino_ESP32RGBPanel *rgbPanel);
void lvglPortLoop();

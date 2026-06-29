#pragma once

#include "lv_conf.h"
#include <lvgl.h>
#include <Arduino_GFX_Library.h>

/* Physical panel dimensions (used by the .ino for GT911 init) */
#define SCREEN_W  800
#define SCREEN_H  480

/* GFX rotation passed to Arduino_RGB_Display constructor.
   We do NOT rotate at the GFX layer — LVGL handles the 90° rotation
   in software, giving a 480×800 portrait logical display.              */
#define DISPLAY_ROTATION  0

/* Logical LVGL display size after 90° rotation */
#define DISP_W  480
#define DISP_H  800

/* Pointer to the Arduino_GFX panel; used by the .ino for width()/height() */
extern Arduino_GFX *gfx;

/**
 * Initialise LVGL, create the display driver, allocate PSRAM buffers,
 * and apply 90° software rotation.
 *
 * @param panel     Pointer to the Arduino_RGB_Display (800×480)
 * @param rgbpanel  Pointer to the underlying Arduino_ESP32RGBPanel
 * @return true on success, false if PSRAM allocation fails
 */
bool lvglPortInit(Arduino_RGB_Display *panel, Arduino_ESP32RGBPanel *rgbpanel);

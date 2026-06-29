#pragma once

#define LV_CONF_INCLUDE_SIMPLE
#include <lvgl.h>
#include <TAMC_GT911.h>

/* Rotation constants passed to touchLvglInit() to match the display rotation */
typedef enum {
    ROTATION_NONE  = 0,  /* landscape, no rotation */
    ROTATION_RIGHT = 1,  /* 90° CW  — portrait with right edge at top */
    ROTATION_DOWN  = 2,  /* 180°    — landscape upside-down             */
    ROTATION_LEFT  = 3   /* 90° CCW — portrait with left edge at top   */
} TouchRotation;

/**
 * Register the GT911 as an LVGL pointer input device.
 *
 * @param dev      TAMC_GT911 instance (already instantiated in .ino)
 * @param dispW    Logical display width  (DISP_W = 480 after 90° rotation)
 * @param dispH    Logical display height (DISP_H = 800 after 90° rotation)
 * @param rotation How touch coordinates map to the rotated display
 */
void touchLvglInit(TAMC_GT911 &dev, int dispW, int dispH, TouchRotation rotation);

#pragma once

#include "lv_conf.h"
#include <lvgl.h>

/* Names prefixed TOUCH_ROT_* — TAMC_GT911.h also defines ROTATION_RIGHT etc. */
typedef enum {
    TOUCH_ROT_NONE   = 0,
    TOUCH_ROT_CW_90  = 1,  /* 90° CW  — portrait, right edge at top */
    TOUCH_ROT_180    = 2,
    TOUCH_ROT_CCW_90 = 3   /* 90° CCW — portrait, left edge at top */
} TouchRotation;

#include <TAMC_GT911.h>

/**
 * Register the GT911 as an LVGL pointer input device.
 *
 * @param dev      TAMC_GT911 instance (already instantiated in .ino)
 * @param dispW    Logical display width  (DISP_W = 480 after 90° rotation)
 * @param dispH    Logical display height (DISP_H = 800 after 90° rotation)
 * @param rotation How touch coordinates map to the rotated display
 */
void touchLvglInit(TAMC_GT911 &dev, int dispW, int dispH, TouchRotation rotation);

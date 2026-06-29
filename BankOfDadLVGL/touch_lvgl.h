#pragma once
/* touch_lvgl.h v2.2.1 — must use TOUCH_ROT_* (NOT ROTATION_RIGHT) */

#include "lv_conf.h"
#include <lvgl.h>

/* Define BEFORE TAMC_GT911.h — that library macros ROTATION_RIGHT etc. */
typedef enum {
    TOUCH_ROT_NONE   = 0,
    TOUCH_ROT_CW_90  = 1,
    TOUCH_ROT_180    = 2,
    TOUCH_ROT_CCW_90 = 3
} TouchRotation;

#include <TAMC_GT911.h>

void touchLvglInit(TAMC_GT911 &dev, int dispW, int dispH, TouchRotation rotation);

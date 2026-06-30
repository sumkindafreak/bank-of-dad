#pragma once
/* Header only — implementation is in BankOfDadLVGL.ino */

#include "lv_conf.h"
#include <lvgl.h>

typedef enum {
    TOUCH_ROT_NONE   = 0,
    TOUCH_ROT_CW_90  = 1,
    TOUCH_ROT_180    = 2,
    TOUCH_ROT_CCW_90 = 3
} TouchRotation;

#include <TAMC_GT911.h>

void touchLvglInit(TAMC_GT911 &dev, int dispW, int dispH, TouchRotation rotation);

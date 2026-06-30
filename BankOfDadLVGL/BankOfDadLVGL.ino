/*
 * BANK OF DAD v2.2.2 — LVGL 9 retro family banking terminal
 * JC8048W550C | ESP32-S3 | 800x480 RGB | GT911 | 480x800 portrait UI
 *
 * Arduino IDE: ESP32S3 Dev Module | OPI PSRAM | 16MB Flash | QIO 80MHz
 *
 * LVGL config — copy lv_conf.h to BOTH:
 *   BankOfDadLVGL/lv_conf.h
 *   Documents/Arduino/libraries/lv_conf.h   (see arduino-libraries/lv_conf.h)
 *
 * Touch + RGB sync code lives in this .ino file (build v2.2.2).
 * Run FIX-BUILD-NOW.bat in this folder if compile still fails.
 */

#define BANK_OF_DAD_BUILD 222

#define LV_CONF_INCLUDE_SIMPLE
#include "lv_conf.h"

#include <stdint.h>
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "app.h"
#include "lvgl_port.h"
#include "rgb_sync.h"

/* Touch enum BEFORE TAMC_GT911.h (that library defines ROTATION_RIGHT macros) */
typedef enum {
    TOUCH_ROT_NONE   = 0,
    TOUCH_ROT_CW_90  = 1,
    TOUCH_ROT_180    = 2,
    TOUCH_ROT_CCW_90 = 3
} TouchRotation;

#include <TAMC_GT911.h>

/* ================================================================
   RGB SYNC (was rgb_sync.cpp)
   ================================================================ */
static bool s_rgbChild = false;
static bool s_rgbAdmin = false;
static bool s_rgbError = false;

void rgbSyncSetChildActive(bool on) { s_rgbChild = on; s_rgbError = false; }
void rgbSyncSetAdminActive(bool on) { s_rgbAdmin = on; s_rgbError = false; }
void rgbSyncSetError(bool on)       { s_rgbError = on; }

void rgbSyncTick(uint32_t nowMs) {
    (void)nowMs;
    (void)s_rgbChild;
    (void)s_rgbAdmin;
    (void)s_rgbError;
}

/* ================================================================
   TOUCH / GT911 (was touch_lvgl.cpp)
   Uses TOUCH_ROT_* — NOT ROTATION_RIGHT (macro in TAMC_GT911.h)
   Uses points[0]   — NOT tp[0]
   ================================================================ */
static TAMC_GT911   *s_touchDev = nullptr;
static TouchRotation s_touchRot = TOUCH_ROT_CW_90;

static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data) {
    (void)indev;
    if (!s_touchDev) return;

    s_touchDev->read();

    if (s_touchDev->isTouched && s_touchDev->touches > 0) {
        int px = s_touchDev->points[0].x;
        int py = s_touchDev->points[0].y;
        int lx, ly;

        switch (s_touchRot) {
            case TOUCH_ROT_CW_90:
                lx = py;
                ly = (SCREEN_W - 1) - px;
                break;
            case TOUCH_ROT_CCW_90:
                lx = (SCREEN_H - 1) - py;
                ly = px;
                break;
            case TOUCH_ROT_180:
                lx = (SCREEN_W - 1) - px;
                ly = (SCREEN_H - 1) - py;
                break;
            default:
                lx = px;
                ly = py;
                break;
        }

        data->point.x = (lv_coord_t)lx;
        data->point.y = (lv_coord_t)ly;
        data->state   = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

void touchLvglInit(TAMC_GT911 &dev, int dispW, int dispH, TouchRotation rotation) {
    dev.begin();
    s_touchDev = &dev;
    s_touchRot = rotation;

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read_cb);

    Serial.printf("touch: GT911 ok, rot=%d, logical %dx%d\n",
                  (int)rotation, dispW, dispH);
}

/* ================================================================
   HARDWARE
   ================================================================ */
#define TFT_BL 2
#define BOUNCE_BUFFER (800 * 20)

TAMC_GT911 touchDev(19, 20, 18, 38, SCREEN_W, SCREEN_H);

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    40, 41, 39, 42,
    45, 48, 47, 21, 14,
    5, 6, 7, 15, 16, 4,
    8, 3, 46, 9, 1,
    0, 8, 4, 8,
    0, 8, 4, 8,
    1, 16000000, false, 0, 0, BOUNCE_BUFFER);

Arduino_RGB_Display *panel = new Arduino_RGB_Display(
    800, 480, rgbpanel, DISPLAY_ROTATION, false);

static void backlightOn() {
    pinMode(TFT_BL, OUTPUT);
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcAttach(TFT_BL, 1000, 10);
    ledcWrite(TFT_BL, 1023);
#else
    ledcSetup(0, 1000, 10);
    ledcAttachPin(TFT_BL, 0);
    ledcWrite(0, 1023);
#endif
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("BANK OF DAD v2.2.2 // LVGL 9");

    if (!psramFound() || ESP.getPsramSize() < 700000) {
        Serial.println("Enable OPI PSRAM + 16MB + QIO 80MHz");
        while (1) delay(2000);
    }
    Serial.printf("PSRAM: %u bytes free\n", (unsigned)ESP.getFreePsram());

    backlightOn();

    if (!lvglPortInit(panel, rgbpanel)) {
        Serial.println("LVGL port init failed");
        while (1) delay(1000);
    }

    touchLvglInit(touchDev, DISP_W, DISP_H, TOUCH_ROT_CW_90);
    Serial.printf("Display logical %ux%u portrait\n", DISP_W, DISP_H);

    appSetup();
}

void loop() {
    appLoop();
    delay(5);
}

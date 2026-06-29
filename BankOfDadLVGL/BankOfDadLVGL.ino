/*
 * BANK OF DAD v2.2 — LVGL 9 retro family banking terminal
 * JC8048W550C | ESP32-S3 | 800x480 RGB | GT911 | rotated 90° (480x800 UI)
 *
 * Modules: model, ui_lvgl, ui_platform, ui_fx, platform, touch_lvgl, rgb_sync
 *
 * Arduino IDE board settings:
 *   ESP32S3 Dev Module | OPI PSRAM | 16MB Flash | QIO 80MHz | USB CDC On Boot
 *
 * LVGL config (both required):
 *   BankOfDadLVGL/lv_conf.h
 *   Documents/Arduino/libraries/lv_conf.h  (copy from repo arduino-libraries/lv_conf.h)
 */

#define LV_CONF_INCLUDE_SIMPLE
#include "lv_conf.h"

#include <Arduino_GFX_Library.h>
#include "app.h"
#include "lvgl_port.h"
#include "touch_lvgl.h"

#define TFT_BL 2
/* 20-line bounce buffer stops RGB DMA drift on ESP32-S3 RGB panels */
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
    800, 480, rgbpanel, DISPLAY_ROTATION, false /* flush batched in lvgl_port */);

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
  Serial.println("BANK OF DAD v2.2 // LVGL 9");

  if (!psramFound() || ESP.getPsramSize() < 700000) {
    Serial.println("Enable OPI PSRAM + 16MB + QIO 80MHz");
    while (1) delay(2000);
  }
  Serial.printf("PSRAM: %u bytes free\n", (unsigned)ESP.getFreePsram());

  backlightOn();

  if (!lvglPortInit(panel, rgbpanel)) {
    Serial.println("LVGL port init failed — check PSRAM / bounce buffer");
    while (1) delay(1000);
  }

  /* TOUCH_ROT_CW_90 — do not use ROTATION_RIGHT (macro in TAMC_GT911.h) */
  touchLvglInit(touchDev, DISP_W, DISP_H, TOUCH_ROT_CW_90);
  Serial.printf("Display logical %ux%u (portrait)\n", DISP_W, DISP_H);

  appSetup();
}

void loop() {
  appLoop();
  delay(5);
}

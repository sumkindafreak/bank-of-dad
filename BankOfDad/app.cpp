#include "app.h"
#include "model.h"
#include "platform.h"
#include "ui_lvgl.h"
#include "lvgl_port.h"

void appSetup() {
  modelInit();
  platformInit();
  uiLvglInit();
}

void appLoop() {
  platformTick();
  modelWebServe();
  uiLvglOnTick();
  lvglPortLoop();
}

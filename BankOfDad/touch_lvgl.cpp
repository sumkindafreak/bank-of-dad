#include "touch_lvgl.h"

#include <lvgl.h>

static TAMC_GT911 *s_touch = nullptr;
static uint16_t s_w = 0;
static uint16_t s_h = 0;
static Rotation s_rot = ROTATION_RIGHT;

static void touchReadCb(lv_indev_t *indev, lv_indev_data_t *data) {
  (void)indev;
  data->state = LV_INDEV_STATE_RELEASED;

  if (!s_touch) return;

  s_touch->read();
  if (!s_touch->isTouched) return;

  TP_Point p = s_touch->points[0];
  int32_t x = p.x;
  int32_t y = p.y;

  if (s_rot == ROTATION_RIGHT) {
    int32_t tx = y;
    int32_t ty = s_w - 1 - x;
    x = tx;
    y = ty;
  }

  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x >= (int32_t)s_w) x = s_w - 1;
  if (y >= (int32_t)s_h) y = s_h - 1;

  data->point.x = x;
  data->point.y = y;
  data->state = LV_INDEV_STATE_PRESSED;
}

void touchLvglInit(TAMC_GT911 &touch, uint16_t width, uint16_t height, Rotation rotation) {
  s_touch = &touch;
  s_w = width;
  s_h = height;
  s_rot = rotation;

  touch.begin();
  touch.setRotation(rotation);

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchReadCb);
}

/* touch_lvgl.cpp v2.2.1 — uses points[0], not tp[0] */
#include "touch_lvgl.h"
#include "lvgl_port.h"

static TAMC_GT911   *s_dev   = nullptr;
static int           s_dispW = 480;
static int           s_dispH = 800;
static TouchRotation s_rot   = TOUCH_ROT_CW_90;

static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data) {
    (void)indev;
    if (!s_dev) return;

    s_dev->read();

    if (s_dev->isTouched && s_dev->touches > 0) {
        int px = s_dev->points[0].x;
        int py = s_dev->points[0].y;

        int lx, ly;
        switch (s_rot) {
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
    s_dev   = &dev;
    s_dispW = dispW;
    s_dispH = dispH;
    s_rot   = rotation;

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read_cb);

    Serial.printf("touch_lvgl: GT911 registered, rotation=%d, logical %dx%d\n",
                  (int)rotation, dispW, dispH);
}

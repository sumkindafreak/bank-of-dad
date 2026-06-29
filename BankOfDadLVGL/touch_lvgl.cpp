#include "touch_lvgl.h"
#include "lvgl_port.h"   /* SCREEN_W, SCREEN_H */

/* Stored by the read callback */
static TAMC_GT911 *s_dev      = nullptr;
static int         s_dispW    = 480;
static int         s_dispH    = 800;
static TouchRotation s_rot    = ROTATION_RIGHT;

/* -----------------------------------------------------------------------
   Coordinate transform for ROTATION_RIGHT (90° CW):
     Physical GT911 gives (px, py) in [0..SCREEN_W-1, 0..SCREEN_H-1]
                                   = [0..799, 0..479]
     After 90° CW to portrait (DISP_W=480, DISP_H=800):
       logical_x = py                     (0..479 → 0..479)
       logical_y = (SCREEN_W - 1) - px    (0..799 → 0..799, inverted)

   If touches feel mirrored on your specific panel:
     — swap x/y or negate: adjust the two lines marked CALIBRATE below.
     — print raw p.x / p.y to Serial and correlate with screen corners.
   ----------------------------------------------------------------------- */
static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data) {
    if (!s_dev) return;

    s_dev->read();

    if (s_dev->isTouched && s_dev->touches > 0) {
        int px = s_dev->tp[0].x;
        int py = s_dev->tp[0].y;

        int lx, ly;
        switch (s_rot) {
            case ROTATION_RIGHT:          /* 90° CW — portrait, right side up */
                lx = py;                  /* CALIBRATE if needed */
                ly = (SCREEN_W - 1) - px; /* CALIBRATE if needed */
                break;
            case ROTATION_LEFT:           /* 90° CCW */
                lx = (SCREEN_H - 1) - py;
                ly = px;
                break;
            case ROTATION_DOWN:           /* 180° */
                lx = (SCREEN_W - 1) - px;
                ly = (SCREEN_H - 1) - py;
                break;
            default:                      /* ROTATION_NONE */
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

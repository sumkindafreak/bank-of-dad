#include "lvgl_port.h"
#include <esp_heap_caps.h>

Arduino_GFX *gfx = nullptr;

/* Partial render buffer: 20 lines × 800 pixels × 2 bytes (RGB565).
   Two buffers allow LVGL to render into one while the other is being flushed. */
#define LV_BUF_LINES  20
#define LV_BUF_BYTES  (SCREEN_W * LV_BUF_LINES * sizeof(uint16_t))

static uint8_t *lvBuf1 = nullptr;
static uint8_t *lvBuf2 = nullptr;
static lv_display_t *lvDisp = nullptr;

/* -----------------------------------------------------------------------
   Flush callback — called by LVGL when a rectangular region is ready.
   Coordinates are in physical (pre-rotation) display space.
   ----------------------------------------------------------------------- */
static void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = (uint32_t)(area->x2 - area->x1 + 1);
    uint32_t h = (uint32_t)(area->y2 - area->y1 + 1);
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
    lv_display_flush_ready(disp);
}

bool lvglPortInit(Arduino_RGB_Display *panel, Arduino_ESP32RGBPanel *rgbpanel) {
    gfx = panel;

    /* Route large malloc calls to PSRAM so LVGL widget allocations
       don't exhaust the ~300 KB internal DRAM.                         */
    heap_caps_malloc_extmem_enable(8192);

    lv_init();

    /* Use Arduino millis() as the LVGL tick source — no ISR needed.   */
    lv_tick_set_cb((lv_tick_get_cb_t)millis);

    /* Allocate render buffers in PSRAM */
    lvBuf1 = (uint8_t *)ps_malloc(LV_BUF_BYTES);
    lvBuf2 = (uint8_t *)ps_malloc(LV_BUF_BYTES);
    if (!lvBuf1 || !lvBuf2) {
        Serial.println("lvgl_port: ps_malloc failed for display buffers");
        return false;
    }

    /* Create display with physical dimensions, then tell LVGL to rotate
       90° in software → logical display becomes DISP_W × DISP_H (480×800). */
    lvDisp = lv_display_create(SCREEN_W, SCREEN_H);
    lv_display_set_flush_cb(lvDisp, disp_flush);
    lv_display_set_buffers(lvDisp, lvBuf1, lvBuf2, LV_BUF_BYTES,
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_rotation(lvDisp, LV_DISPLAY_ROTATION_90);

    Serial.printf("lvgl_port: display %ux%u logical (rotated from %ux%u physical)\n",
                  lv_display_get_horizontal_resolution(lvDisp),
                  lv_display_get_vertical_resolution(lvDisp),
                  SCREEN_W, SCREEN_H);
    return true;
}

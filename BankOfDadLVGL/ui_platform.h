#pragma once

#define LV_CONF_INCLUDE_SIMPLE
#include <lvgl.h>
#include "lvgl_port.h"
#include "model.h"

#define UI_HDR_H   70
#define UI_BTN_H   85
#define UI_BTN_W   (DISP_W - 40)
#define UI_BTN_X   20
#define UI_BTN_GAP 14
#define UI_KEY_W   136
#define UI_KEY_H   90
#define UI_KEY_GAP 8

#define UI_C_BG      lv_color_hex(0x001100)
#define UI_C_PANEL   lv_color_hex(0x003300)
#define UI_C_TEXT    lv_color_hex(0x33FF33)
#define UI_C_DIM     lv_color_hex(0x1A991A)
#define UI_C_GOOD    lv_color_hex(0x00FF66)
#define UI_C_BAD     lv_color_hex(0xFF3333)
#define UI_C_WARN    lv_color_hex(0xFFCC00)
#define UI_C_ACCENT  lv_color_hex(0x00AA00)
#define UI_C_BLUE    lv_color_hex(0x33CCFF)
#define UI_C_CYAN    lv_color_hex(0x66FFFF)
#define UI_C_PRESSED lv_color_hex(0x005500)

#define UI_F14 (&lv_font_montserrat_14)
#define UI_F16 (&lv_font_montserrat_16)
#define UI_F20 (&lv_font_montserrat_20)
#define UI_F24 (&lv_font_montserrat_24)
#define UI_F28 (&lv_font_montserrat_28)
#define UI_F36 (&lv_font_montserrat_36)

void uiPlatformInitTheme();
void uiPlatformLoadScreen(lv_obj_t *scr);
lv_obj_t *uiPlatformMakeScreen(const char *title, lv_color_t hdrCol);
lv_obj_t *uiPlatformMakeBtn(lv_obj_t *parent, int x, int y, int w, int h,
                            const char *text, lv_color_t bgCol,
                            lv_event_cb_t cb, void *ud);
int uiPlatformAddBtn(lv_obj_t *scr, int y, const char *text, lv_color_t col,
                     lv_event_cb_t cb, void *ud);
void uiPlatformBuildKeypad(lv_obj_t *parent, int yTop, lv_event_cb_t cb);
lv_obj_t *uiPlatformMakeScrollList(lv_obj_t *parent, int y, int h);

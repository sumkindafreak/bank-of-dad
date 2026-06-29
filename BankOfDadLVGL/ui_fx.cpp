#include "ui_fx.h"
#include "ui_platform.h"
#include "lvgl_port.h"
#include <string.h>

static lv_obj_t *s_overlay = nullptr;
static lv_obj_t *s_popup = nullptr;
static uint32_t  s_popupUntil = 0;

void uiFxInit() {
    s_overlay = nullptr;
    s_popup = nullptr;
    s_popupUntil = 0;
}

void uiFxApplyCrtOverlay(lv_obj_t *parent) {
    if (!parent) return;
    lv_obj_t *scan = lv_obj_create(parent);
    lv_obj_set_size(scan, DISP_W, DISP_H);
    lv_obj_set_pos(scan, 0, 0);
    lv_obj_remove_flag(scan, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(scan, LV_OPA_10, 0);
    lv_obj_set_style_bg_color(scan, lv_color_hex(0x003300), 0);
    lv_obj_set_style_border_width(scan, 0, 0);
    lv_obj_move_foreground(scan);
    s_overlay = scan;
}

static void popupScaleAnim(void *obj, int32_t v) {
    lv_obj_set_style_transform_scale((lv_obj_t *)obj, v, 0);
}

void uiFxShowAchievement(const char *title, const char *subtitle) {
    if (s_popup) {
        lv_obj_delete(s_popup);
        s_popup = nullptr;
    }
    s_popup = lv_obj_create(lv_layer_top());
    lv_obj_set_size(s_popup, DISP_W - 40, 140);
    lv_obj_align(s_popup, LV_ALIGN_TOP_MID, 0, 90);
    lv_obj_set_style_bg_color(s_popup, UI_C_PANEL, 0);
    lv_obj_set_style_border_color(s_popup, UI_C_GOOD, 0);
    lv_obj_set_style_border_width(s_popup, 3, 0);
    lv_obj_set_style_radius(s_popup, 8, 0);

    lv_obj_t *t = lv_label_create(s_popup);
    lv_label_set_text(t, title ? title : "ACHIEVEMENT");
    lv_obj_set_style_text_color(t, UI_C_GOOD, 0);
    lv_obj_set_style_text_font(t, UI_F24, 0);
    lv_obj_align(t, LV_ALIGN_TOP_MID, 0, 16);

    lv_obj_t *s = lv_label_create(s_popup);
    lv_label_set_text(s, subtitle ? subtitle : "");
    lv_obj_set_style_text_color(s, UI_C_TEXT, 0);
    lv_obj_set_style_text_font(s, UI_F16, 0);
    lv_obj_align(s, LV_ALIGN_TOP_MID, 0, 56);

    lv_obj_set_style_transform_scale(s_popup, 256, 0);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, s_popup);
    lv_anim_set_values(&a, 200, 256);
    lv_anim_set_time(&a, 250);
    lv_anim_set_exec_cb(&a, popupScaleAnim);
    lv_anim_start(&a);

    s_popupUntil = millis() + 2800;
}

void uiFxTick(uint32_t nowMs) {
    if (s_popup && s_popupUntil > 0 && nowMs >= s_popupUntil) {
        lv_obj_delete(s_popup);
        s_popup = nullptr;
        s_popupUntil = 0;
    }
}

#include "ui_platform.h"

void uiPlatformInitTheme() {
    lv_theme_t *th = lv_theme_default_init(
        lv_display_get_default(),
        UI_C_TEXT, UI_C_BG, true,
        UI_F16);
    lv_display_set_theme(lv_display_get_default(), th);
}

void uiPlatformLoadScreen(lv_obj_t *scr) {
    lv_screen_load_anim(scr, LV_SCR_LOAD_ANIM_FADE_IN, 120, 0, true);
}

lv_obj_t *uiPlatformMakeScreen(const char *title, lv_color_t hdrCol) {
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(scr, UI_C_BG, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);
    lv_obj_set_style_border_width(scr, 0, 0);

    lv_obj_t *hdr = lv_obj_create(scr);
    lv_obj_set_size(hdr, DISP_W, UI_HDR_H);
    lv_obj_set_pos(hdr, 0, 0);
    lv_obj_remove_flag(hdr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(hdr, hdrCol, 0);
    lv_obj_set_style_bg_opa(hdr, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(hdr, 0, 0);
    lv_obj_set_style_radius(hdr, 0, 0);
    lv_obj_set_style_pad_left(hdr, 16, 0);

    lv_obj_t *lbl = lv_label_create(hdr);
    lv_label_set_text(lbl, title);
    lv_obj_set_style_text_color(lbl, UI_C_TEXT, 0);
    lv_obj_set_style_text_font(lbl, UI_F28, 0);
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 0);

    return scr;
}

lv_obj_t *uiPlatformMakeBtn(lv_obj_t *parent, int x, int y, int w, int h,
                            const char *text, lv_color_t bgCol,
                            lv_event_cb_t cb, void *ud) {
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_style_bg_color(btn, bgCol, 0);
    lv_obj_set_style_bg_color(btn, UI_C_PRESSED, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn, 6, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_border_width(btn, 2, 0);
    lv_obj_set_style_border_color(btn, UI_C_DIM, 0);
    lv_obj_set_style_pad_all(btn, 4, 0);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_color(lbl, UI_C_TEXT, 0);
    lv_obj_set_style_text_font(lbl, UI_F20, 0);
    lv_obj_center(lbl);

    if (cb) lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, ud);
    return btn;
}

int uiPlatformAddBtn(lv_obj_t *scr, int y, const char *text, lv_color_t col,
                     lv_event_cb_t cb, void *ud) {
    uiPlatformMakeBtn(scr, UI_BTN_X, y, UI_BTN_W, UI_BTN_H, text, col, cb, ud);
    return y + UI_BTN_H + UI_BTN_GAP;
}

void uiPlatformBuildKeypad(lv_obj_t *parent, int yTop, lv_event_cb_t cb) {
    static const char *keys[12] = {
        "1","2","3","4","5","6","7","8","9","*","0","#"
    };
    for (int i = 0; i < 12; i++) {
        int col = i % 3;
        int row = i / 3;
        int kx = UI_BTN_X + col * (UI_KEY_W + UI_KEY_GAP);
        int ky = yTop + row * (UI_KEY_H + UI_KEY_GAP);
        uiPlatformMakeBtn(parent, kx, ky, UI_KEY_W, UI_KEY_H, keys[i], UI_C_PANEL, cb, (void *)keys[i]);
    }
    lv_obj_t *hint = lv_label_create(parent);
    lv_label_set_text(hint, "* Back / Clear     # Enter");
    lv_obj_set_style_text_color(hint, UI_C_DIM, 0);
    lv_obj_set_style_text_font(hint, UI_F14, 0);
    lv_obj_set_pos(hint, UI_BTN_X, yTop + 4 * (UI_KEY_H + UI_KEY_GAP) + 4);
}

lv_obj_t *uiPlatformMakeScrollList(lv_obj_t *parent, int y, int h) {
    lv_obj_t *list = lv_obj_create(parent);
    lv_obj_set_size(list, UI_BTN_W, h);
    lv_obj_set_pos(list, UI_BTN_X, y);
    lv_obj_set_style_bg_color(list, UI_C_PANEL, 0);
    lv_obj_set_style_border_color(list, UI_C_DIM, 0);
    lv_obj_set_style_border_width(list, 1, 0);
    lv_obj_set_style_pad_all(list, 8, 0);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_AUTO);
    return list;
}

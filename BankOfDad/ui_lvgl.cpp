#include "ui_lvgl.h"
#include "config.h"
#include "model.h"
#include "platform.h"

#include <lvgl.h>

static lv_obj_t *s_statusLabel = nullptr;
static unsigned long s_lastRefreshMs = 0;

void uiLvglInit() {
  lv_obj_t *scr = lv_screen_active();
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);

  lv_obj_t *title = lv_label_create(scr);
  lv_label_set_text(title, "BANK OF DAD");
  lv_obj_set_style_text_color(title, lv_color_hex(0xE74C3C), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 24);

  s_statusLabel = lv_label_create(scr);
  lv_obj_set_width(s_statusLabel, 440);
  lv_label_set_long_mode(s_statusLabel, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_color(s_statusLabel, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(s_statusLabel, &lv_font_montserrat_16, 0);
  lv_obj_align(s_statusLabel, LV_ALIGN_CENTER, 0, 20);

  lv_obj_t *hint = lv_label_create(scr);
  lv_label_set_text(hint,
                    "Connect phone to BankOfDad hotspot\n"
                    "Open http://192.168.4.1/wifi to join home network\n"
                    "OTA password: BANKOFDAD");
  lv_obj_set_style_text_color(hint, lv_color_hex(0x888888), 0);
  lv_obj_set_style_text_font(hint, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -24);
}

void uiLvglOnTick() {
  if (!s_statusLabel) return;
  if (millis() - s_lastRefreshMs < 1000) return;
  s_lastRefreshMs = millis();

  char wifi[96];
  char hint[64];
  modelWifiStatusText(wifi, sizeof(wifi));
  modelFormatWebHint(hint, sizeof(hint));

  char line[160];
  snprintf(line, sizeof(line), "%s\n%s\nOTA: %s (home WiFi only)",
           wifi, hint, platformOtaReady() ? "ready" : "waiting for home WiFi");
  lv_label_set_text(s_statusLabel, line);
}

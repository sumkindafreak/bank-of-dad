#pragma once

#define LV_CONF_INCLUDE_SIMPLE
#include <lvgl.h>

void uiFxInit();
void uiFxApplyCrtOverlay(lv_obj_t *parent);
void uiFxShowAchievement(const char *title, const char *subtitle);
void uiFxTick(uint32_t nowMs);

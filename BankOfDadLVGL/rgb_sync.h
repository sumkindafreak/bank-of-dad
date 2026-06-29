#pragma once

#include <Arduino.h>

void rgbSyncSetChildActive(bool on);
void rgbSyncSetAdminActive(bool on);
void rgbSyncSetError(bool on);
void rgbSyncTick(uint32_t nowMs);

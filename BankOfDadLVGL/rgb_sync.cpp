#include <stdint.h>
#include <Arduino.h>
#include "rgb_sync.h"

static bool s_child = false;
static bool s_admin = false;
static bool s_error = false;

void rgbSyncSetChildActive(bool on) { s_child = on; s_error = false; }
void rgbSyncSetAdminActive(bool on) { s_admin = on; s_error = false; }
void rgbSyncSetError(bool on)       { s_error = on; }

void rgbSyncTick(uint32_t nowMs) {
    (void)nowMs;
    (void)s_child;
    (void)s_admin;
    (void)s_error;
}

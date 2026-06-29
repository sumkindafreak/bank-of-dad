/*
 * app.cpp — Bank of Dad v2.2 application shell
 * Wires model, platform, UI theme/FX, and LVGL screens.
 */

#include "app.h"
#include "model.h"
#include "platform.h"
#include "ui_platform.h"
#include "ui_fx.h"
#include "ui_lvgl.h"
#include "rgb_sync.h"

void appSetup() {
    bool fresh = modelLoadAll();
    platformInit();
    uiPlatformInitTheme();
    uiFxInit();
    uiLvglInit();

    if (fresh) {
        for (int i = 0; i < BANK_ACCOUNT_COUNT; i++)
            modelAddHistory(i, "Account opened");
    }

    Serial.println("Bank of Dad v2.2 ready");
    Serial.println("Admin PIN: 9999");
    Serial.println("Child PINs: Nixon 1111, Oakley 2222, Hadley 3333, Maddison 4444");
}

void appLoop() {
    uint32_t now = millis();
    uiLvglLoop();
    platformLoop();
    modelTick(now);
    uiFxTick(now);
    rgbSyncTick(now);
}

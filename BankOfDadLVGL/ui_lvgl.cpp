/*
 * ui_lvgl.cpp — Bank of Dad v2.2 LVGL screens
 * JC8048W550C / ESP32-S3 / 480x800 portrait UI
 */

#include "ui_lvgl.h"
#include "model.h"
#include "ui_platform.h"
#include "ui_fx.h"
#include "rgb_sync.h"
#include "storage.h"
#include "lvgl_port.h"

enum AmountPurpose : uint8_t {
    AMOUNT_DEPOSIT = 0,
    AMOUNT_WITHDRAW,
    AMOUNT_BONUS,
    AMOUNT_FINE
};

static void draw_home();
static void draw_pin(bool isAdmin);
static void draw_account();
static void draw_history();
static void draw_goals();
static void draw_chores();
static void draw_achievements();
static void draw_leaderboard();
static void draw_shop();
static void draw_stats();
static void draw_notifications();
static void draw_admin_menu();
static void draw_admin_select();
static void draw_admin_action();
static void draw_approvals();
static void draw_dad_tax();
static void draw_amount(AmountPurpose purpose);
static void draw_admin_diag();
static void draw_message(const char *title, const char *line1,
                         const char *line2, bool isError, MsgReturn ret);

static MsgReturn s_msgReturn = MSG_RET_HOME;
static lv_obj_t *s_pinBoxes[4]   = {nullptr, nullptr, nullptr, nullptr};
static lv_obj_t *s_pinDigits[4]  = {nullptr, nullptr, nullptr, nullptr};
static lv_obj_t *s_pinProgress   = nullptr;
static lv_obj_t *s_amountLabel   = nullptr;
static AmountPurpose s_amountPurpose = AMOUNT_DEPOSIT;

static void _go_home(void *)           { draw_home(); }
static void _go_account(void *)        { draw_account(); }
static void _go_admin_menu(void *)     { draw_admin_menu(); }
static void _go_admin_select(void *)   { draw_admin_select(); }
static void _go_admin_action(void *)   { draw_admin_action(); }
static void _go_approvals(void *)      { draw_approvals(); }
static void _go_history(void *)        { draw_history(); }
static void _go_goals(void *)          { draw_goals(); }
static void _go_chores(void *)         { draw_chores(); }
static void _go_achievements(void *)   { draw_achievements(); }
static void _go_leaderboard(void *)    { draw_leaderboard(); }
static void _go_dad_tax(void *)        { draw_dad_tax(); }
static void _go_deposit(void *)        { draw_amount(AMOUNT_DEPOSIT); }
static void _go_withdraw(void *)       { draw_amount(AMOUNT_WITHDRAW); }
static void _go_bonus(void *)          { draw_amount(AMOUNT_BONUS); }
static void _go_fine(void *)           { draw_amount(AMOUNT_FINE); }
static void _go_pin_child(void *)      { draw_pin(false); }
static void _go_pin_admin(void *)      { draw_pin(true); }
static void _go_shop(void *)           { draw_shop(); }
static void _go_stats(void *)          { draw_stats(); }
static void _go_notifications(void *) { draw_notifications(); }
static void _go_admin_diag(void *)     { draw_admin_diag(); }

static void _go_admin_select_tax(void *) {
    g_bank.g_taxPath = true;
    draw_admin_select();
}

static void _go_admin_select_act(void *) {
    g_bank.g_taxPath = false;
    draw_admin_select();
}

static int unreadNotificationCount() {
    int n = 0;
    for (uint8_t i = 0; i < g_bank.notificationCount; i++)
        if (g_bank.notifications[i].unread) n++;
    return n;
}

/* ================================================================
   HOME SCREEN
   ================================================================ */
static void on_account_btn(lv_event_t *e) {
    g_bank.selectedAccount = (int)(intptr_t)lv_event_get_user_data(e);
    lv_async_call(_go_pin_child, NULL);
}

static void draw_home() {
    g_bank.currentScreen   = SCR_HOME;
    g_bank.selectedAccount = -1;
    g_bank.adminMode       = false;
    memset(g_bank.enteredPin, 0, sizeof(g_bank.enteredPin));
    rgbSyncSetChildActive(false);
    rgbSyncSetAdminActive(false);

    lv_obj_t *scr = uiPlatformMakeScreen("BANK OF DAD v2.2", UI_C_ACCENT);

    lv_obj_t *sub = lv_label_create(scr);
    lv_label_set_text(sub, "RETRO FAMILY BANKING TERMINAL");
    lv_obj_set_style_text_color(sub, UI_C_TEXT, 0);
    lv_obj_set_style_text_font(sub, UI_F20, 0);
    lv_obj_set_pos(sub, UI_BTN_X, UI_HDR_H + 18);

    int y = UI_HDR_H + 60;
    for (int i = 0; i < BANK_MAX_ACCOUNTS; i++) {
        if (!g_bank.accounts[i].active) continue;
        char label[24];
        snprintf(label, sizeof(label), "%s %s", g_bank.accounts[i].avatar, g_bank.accounts[i].name);
        y = uiPlatformAddBtn(scr, y, label, UI_C_PANEL, on_account_btn, (void *)(intptr_t)i);
    }

    uiPlatformAddBtn(scr, DISP_H - UI_BTN_H - 20, "DAD ADMIN", UI_C_BAD,
                     [](lv_event_t *) { lv_async_call(_go_pin_admin, NULL); }, NULL);

    uiFxApplyCrtOverlay(scr);
    uiPlatformLoadScreen(scr);
}

/* ================================================================
   PIN SCREEN
   ================================================================ */
static void refreshPinDisplay() {
    int n = strlen(g_bank.enteredPin);
    for (int i = 0; i < 4; i++) {
        if (!s_pinBoxes[i] || !s_pinDigits[i]) continue;
        bool filled = i < n;
        bool active = (i == n) && (n < 4);

        lv_obj_set_style_bg_color(s_pinBoxes[i], filled ? UI_C_PANEL : UI_C_BG, 0);
        lv_obj_set_style_bg_opa(s_pinBoxes[i], LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(s_pinBoxes[i],
                                      filled ? UI_C_GOOD : (active ? UI_C_WARN : UI_C_DIM), 0);
        lv_obj_set_style_border_width(s_pinBoxes[i], active ? 4 : 2, 0);

        lv_label_set_text(s_pinDigits[i], filled ? "*" : "-");
        lv_obj_set_style_text_color(s_pinDigits[i],
                                    filled ? UI_C_GOOD : (active ? UI_C_WARN : UI_C_DIM), 0);
    }
    if (s_pinProgress) {
        char buf[24];
        snprintf(buf, sizeof(buf), "%d of 4 digits entered", n);
        lv_label_set_text(s_pinProgress, buf);
    }
}

static void on_pin_key(lv_event_t *e) {
    const char *key = (const char *)lv_event_get_user_data(e);
    if (!key) return;

    if (key[0] == '*') {
        int n = strlen(g_bank.enteredPin);
        if (n > 0) {
            g_bank.enteredPin[n - 1] = '\0';
            refreshPinDisplay();
        } else {
            lv_async_call(_go_home, NULL);
        }
        return;
    }
    if (key[0] == '#') {
        if (strlen(g_bank.enteredPin) != 4) {
            draw_message("PIN INCOMPLETE", "Enter all 4 digits", "Then press #", true, MSG_RET_PIN);
            return;
        }

        bool ok = false;
        if (g_bank.currentScreen == SCR_ADMIN_PIN)
            ok = modelValidateAdminPin(g_bank.enteredPin);
        else
            ok = modelValidateChildPin(g_bank.selectedAccount, g_bank.enteredPin);

        if (ok) {
            if (g_bank.currentScreen == SCR_ADMIN_PIN) {
                rgbSyncSetAdminActive(true);
                lv_async_call(_go_admin_menu, NULL);
            } else {
                modelRecordLogin(g_bank.selectedAccount);
                rgbSyncSetChildActive(true);
                lv_async_call(_go_account, NULL);
            }
        } else {
            MsgReturn ret = (g_bank.currentScreen == SCR_ADMIN_PIN) ? MSG_RET_ADMIN_PIN : MSG_RET_PIN;
            memset(g_bank.enteredPin, 0, sizeof(g_bank.enteredPin));
            refreshPinDisplay();
            draw_message("WRONG PIN", "Try again", "", true, ret);
        }
        return;
    }
    if (strlen(g_bank.enteredPin) < 4) {
        int n = strlen(g_bank.enteredPin);
        g_bank.enteredPin[n] = key[0];
        g_bank.enteredPin[n + 1] = '\0';
        refreshPinDisplay();
    }
}

static void draw_pin(bool isAdmin) {
    g_bank.adminMode = isAdmin;
    memset(g_bank.enteredPin, 0, sizeof(g_bank.enteredPin));
    g_bank.currentScreen = isAdmin ? SCR_ADMIN_PIN : SCR_PIN;

    lv_obj_t *scr = uiPlatformMakeScreen(isAdmin ? "ADMIN PIN" : "ENTER PIN",
                                         isAdmin ? UI_C_BAD : UI_C_ACCENT);

    int y = UI_HDR_H + 10;

    if (!isAdmin && g_bank.selectedAccount >= 0) {
        char who[32];
        snprintf(who, sizeof(who), "%s  %s",
                 g_bank.accounts[g_bank.selectedAccount].avatar,
                 g_bank.accounts[g_bank.selectedAccount].name);
        lv_obj_t *whoLbl = lv_label_create(scr);
        lv_label_set_text(whoLbl, who);
        lv_obj_set_style_text_color(whoLbl, UI_C_CYAN, 0);
        lv_obj_set_style_text_font(whoLbl, UI_F24, 0);
        lv_obj_set_pos(whoLbl, UI_BTN_X, y);
        y += 34;
    }

    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "ENTER YOUR 4-DIGIT PIN");
    lv_obj_set_style_text_color(hint, UI_C_TEXT, 0);
    lv_obj_set_style_text_font(hint, UI_F20, 0);
    lv_obj_set_pos(hint, UI_BTN_X, y);
    y += 36;

    lv_obj_t *pinPanel = lv_obj_create(scr);
    lv_obj_set_size(pinPanel, UI_BTN_W, 118);
    lv_obj_set_pos(pinPanel, UI_BTN_X, y);
    lv_obj_remove_flag(pinPanel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(pinPanel, UI_C_PANEL, 0);
    lv_obj_set_style_bg_opa(pinPanel, LV_OPA_60, 0);
    lv_obj_set_style_border_color(pinPanel, UI_C_DIM, 0);
    lv_obj_set_style_border_width(pinPanel, 2, 0);
    lv_obj_set_style_radius(pinPanel, 8, 0);
    lv_obj_set_style_pad_all(pinPanel, 0, 0);

    const int boxW = 88;
    const int boxH = 88;
    const int boxGap = 10;
    const int rowW = 4 * boxW + 3 * boxGap;
    const int panelPadX = (UI_BTN_W - rowW) / 2;

    for (int i = 0; i < 4; i++) {
        int bx = panelPadX + i * (boxW + boxGap);
        s_pinBoxes[i] = lv_obj_create(pinPanel);
        lv_obj_set_size(s_pinBoxes[i], boxW, boxH);
        lv_obj_set_pos(s_pinBoxes[i], bx, 14);
        lv_obj_remove_flag(s_pinBoxes[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(s_pinBoxes[i], UI_C_BG, 0);
        lv_obj_set_style_bg_opa(s_pinBoxes[i], LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(s_pinBoxes[i], UI_C_DIM, 0);
        lv_obj_set_style_border_width(s_pinBoxes[i], 2, 0);
        lv_obj_set_style_radius(s_pinBoxes[i], 8, 0);

        s_pinDigits[i] = lv_label_create(s_pinBoxes[i]);
        lv_label_set_text(s_pinDigits[i], "-");
        lv_obj_set_style_text_color(s_pinDigits[i], UI_C_DIM, 0);
        lv_obj_set_style_text_font(s_pinDigits[i], UI_F36, 0);
        lv_obj_center(s_pinDigits[i]);
    }

    y += 130;
    s_pinProgress = lv_label_create(scr);
    lv_label_set_text(s_pinProgress, "0 of 4 digits entered");
    lv_obj_set_style_text_color(s_pinProgress, UI_C_WARN, 0);
    lv_obj_set_style_text_font(s_pinProgress, UI_F20, 0);
    lv_obj_align(s_pinProgress, LV_ALIGN_TOP_MID, 0, y);

    y += 36;
    uiPlatformBuildKeypad(scr, y, on_pin_key);

    lv_obj_t *keyHint = lv_label_create(scr);
    lv_label_set_text(keyHint, "* Delete digit   # Submit PIN");
    lv_obj_set_style_text_color(keyHint, UI_C_TEXT, 0);
    lv_obj_set_style_text_font(keyHint, UI_F16, 0);
    lv_obj_set_pos(keyHint, UI_BTN_X, y + 4 * (UI_KEY_H + UI_KEY_GAP) + 8);

    refreshPinDisplay();
    uiPlatformLoadScreen(scr);
}

/* ================================================================
   ACCOUNT SCREEN
   ================================================================ */
static void draw_account() {
    g_bank.currentScreen = SCR_ACCOUNT;
    ChildAccount &a = g_bank.accounts[g_bank.selectedAccount];

    lv_obj_t *scr = uiPlatformMakeScreen(a.name, UI_C_ACCENT);

    lv_obj_t *balLbl = lv_label_create(scr);
    lv_label_set_text(balLbl, "Balance");
    lv_obj_set_style_text_color(balLbl, UI_C_TEXT, 0);
    lv_obj_set_style_text_font(balLbl, UI_F20, 0);
    lv_obj_set_pos(balLbl, UI_BTN_X, UI_HDR_H + 10);

    lv_obj_t *balVal = lv_label_create(scr);
    lv_label_set_text(balVal, modelMoneyText(a.balance).c_str());
    lv_obj_set_style_text_color(balVal, UI_C_GOOD, 0);
    lv_obj_set_style_text_font(balVal, UI_F36, 0);
    lv_obj_set_pos(balVal, UI_BTN_X, UI_HDR_H + 36);

    char savBuf[48];
    snprintf(savBuf, sizeof(savBuf), "Savings %s", modelMoneyText(a.savingsBalance).c_str());
    lv_obj_t *savLbl = lv_label_create(scr);
    lv_label_set_text(savLbl, savBuf);
    lv_obj_set_style_text_color(savLbl, UI_C_CYAN, 0);
    lv_obj_set_style_text_font(savLbl, UI_F16, 0);
    lv_obj_set_pos(savLbl, UI_BTN_X, UI_HDR_H + 82);

    char xpBuf[56];
    snprintf(xpBuf, sizeof(xpBuf), "Level %d   XP %ld   Streak %u",
             modelLevelFromXp(a.xp), a.xp, a.currentStreak);
    lv_obj_t *xpLbl = lv_label_create(scr);
    lv_label_set_text(xpLbl, xpBuf);
    lv_obj_set_style_text_color(xpLbl, UI_C_DIM, 0);
    lv_obj_set_style_text_font(xpLbl, UI_F16, 0);
    lv_obj_set_pos(xpLbl, UI_BTN_X, UI_HDR_H + 106);

    int bw = (UI_BTN_W - UI_BTN_GAP) / 2;
    int y0 = UI_HDR_H + 132;
    uiPlatformMakeBtn(scr, UI_BTN_X, y0, bw, UI_BTN_H, "HISTORY", UI_C_PANEL,
                      [](lv_event_t *) { lv_async_call(_go_history, NULL); }, NULL);
    uiPlatformMakeBtn(scr, UI_BTN_X + bw + UI_BTN_GAP, y0, bw, UI_BTN_H, "GOALS", UI_C_PANEL,
                      [](lv_event_t *) { lv_async_call(_go_goals, NULL); }, NULL);

    int y1 = y0 + UI_BTN_H + UI_BTN_GAP;
    uiPlatformMakeBtn(scr, UI_BTN_X, y1, bw, UI_BTN_H, "CHORES", UI_C_PANEL,
                      [](lv_event_t *) { lv_async_call(_go_chores, NULL); }, NULL);
    uiPlatformMakeBtn(scr, UI_BTN_X + bw + UI_BTN_GAP, y1, bw, UI_BTN_H, "BADGES", UI_C_PANEL,
                      [](lv_event_t *) { lv_async_call(_go_achievements, NULL); }, NULL);

    int y2 = y1 + UI_BTN_H + UI_BTN_GAP;
    uiPlatformMakeBtn(scr, UI_BTN_X, y2, bw, UI_BTN_H, "SHOP", UI_C_PANEL,
                      [](lv_event_t *) { lv_async_call(_go_shop, NULL); }, NULL);
    uiPlatformMakeBtn(scr, UI_BTN_X + bw + UI_BTN_GAP, y2, bw, UI_BTN_H, "STATS", UI_C_PANEL,
                      [](lv_event_t *) { lv_async_call(_go_stats, NULL); }, NULL);

    int y3 = y2 + UI_BTN_H + UI_BTN_GAP;
    char alertsLbl[16] = "ALERTS";
    int unread = unreadNotificationCount();
    if (unread > 0) snprintf(alertsLbl, sizeof(alertsLbl), "ALERTS (%d)", unread);
    uiPlatformMakeBtn(scr, UI_BTN_X, y3, bw, UI_BTN_H, alertsLbl, UI_C_WARN,
                      [](lv_event_t *) { lv_async_call(_go_notifications, NULL); }, NULL);
    uiPlatformMakeBtn(scr, UI_BTN_X + bw + UI_BTN_GAP, y3, bw, UI_BTN_H, "LEADER", UI_C_BLUE,
                      [](lv_event_t *) { lv_async_call(_go_leaderboard, NULL); }, NULL);

    uiPlatformAddBtn(scr, DISP_H - UI_BTN_H - 16, "LOGOUT", UI_C_BAD,
                     [](lv_event_t *) { lv_async_call(_go_home, NULL); }, NULL);
    uiPlatformLoadScreen(scr);
}

/* ================================================================
   HISTORY SCREEN
   ================================================================ */
static void draw_history() {
    g_bank.currentScreen = SCR_HISTORY;
    lv_obj_t *scr = uiPlatformMakeScreen("HISTORY", UI_C_ACCENT);

    char sub[56];
    if (storageReady())
        snprintf(sub, sizeof(sub), "%s — SD transaction log", g_bank.accounts[g_bank.selectedAccount].name);
    else
        snprintf(sub, sizeof(sub), "%s — recent activity", g_bank.accounts[g_bank.selectedAccount].name);

    lv_obj_t *subLbl = lv_label_create(scr);
    lv_label_set_text(subLbl, sub);
    lv_obj_set_style_text_color(subLbl, UI_C_DIM, 0);
    lv_obj_set_style_text_font(subLbl, UI_F16, 0);
    lv_obj_set_pos(subLbl, UI_BTN_X, UI_HDR_H + 12);

    int listH = DISP_H - UI_HDR_H - 52 - UI_BTN_H - 32;
    lv_obj_t *list = uiPlatformMakeScrollList(scr, UI_HDR_H + 44, listH);
    bool any = false;

    if (storageReady()) {
        String sdLines[20];
        int n = storageReadHistory(g_bank.accounts[g_bank.selectedAccount].name, sdLines, 20);
        for (int i = 0; i < n; i++) {
            lv_obj_t *row = lv_label_create(list);
            lv_label_set_text(row, sdLines[i].c_str());
            lv_obj_set_style_text_color(row, UI_C_TEXT, 0);
            lv_obj_set_style_text_font(row, UI_F20, 0);
            any = true;
        }
    } else {
        for (int i = 0; i < BANK_HISTORY_RAM; i++) {
            int idx = (g_bank.historyIndex[g_bank.selectedAccount] + i) % BANK_HISTORY_RAM;
            if (g_bank.historyLog[g_bank.selectedAccount][idx].length() > 0) {
                lv_obj_t *row = lv_label_create(list);
                lv_label_set_text(row, g_bank.historyLog[g_bank.selectedAccount][idx].c_str());
                lv_obj_set_style_text_color(row, UI_C_TEXT, 0);
                lv_obj_set_style_text_font(row, UI_F20, 0);
                any = true;
            }
        }
    }

    if (!any) {
        lv_obj_t *none = lv_label_create(list);
        lv_label_set_text(none, "No history yet");
        lv_obj_set_style_text_color(none, UI_C_DIM, 0);
        lv_obj_set_style_text_font(none, UI_F24, 0);
    }

    uiPlatformAddBtn(scr, DISP_H - UI_BTN_H - 16, "BACK", UI_C_BAD,
                     [](lv_event_t *) { lv_async_call(_go_account, NULL); }, NULL);
    uiPlatformLoadScreen(scr);
}

/* ================================================================
   GOALS SCREEN
   ================================================================ */
static void draw_goals() {
    g_bank.currentScreen = SCR_GOALS;
    ChildAccount &a = g_bank.accounts[g_bank.selectedAccount];
    long saved = a.savingsBalance;
    long target = a.goalTarget;
    int percent = (target > 0) ? (int)((saved * 100) / target) : 0;
    if (percent > 100) percent = 100;

    lv_obj_t *scr = uiPlatformMakeScreen("SAVINGS GOAL", UI_C_ACCENT);

    lv_obj_t *name = lv_label_create(scr);
    lv_label_set_text(name, a.goalName[0] ? a.goalName : "Savings goal");
    lv_obj_set_style_text_color(name, UI_C_TEXT, 0);
    lv_obj_set_style_text_font(name, UI_F28, 0);
    lv_obj_set_pos(name, UI_BTN_X, UI_HDR_H + 16);

    auto addInfo = [&](int y, const char *lbl, lv_color_t col, const char *val) {
        lv_obj_t *l = lv_label_create(scr);
        char buf[64];
        snprintf(buf, sizeof(buf), "%s%s", lbl, val);
        lv_label_set_text(l, buf);
        lv_obj_set_style_text_color(l, col, 0);
        lv_obj_set_style_text_font(l, UI_F20, 0);
        lv_obj_set_pos(l, UI_BTN_X, y);
    };

    addInfo(UI_HDR_H + 72, "Target:  ", UI_C_TEXT, modelMoneyText(target).c_str());
    addInfo(UI_HDR_H + 108, "Saved:   ", UI_C_GOOD, modelMoneyText(saved).c_str());

    lv_obj_t *bar = lv_bar_create(scr);
    lv_obj_set_size(bar, UI_BTN_W, 36);
    lv_obj_set_pos(bar, UI_BTN_X, UI_HDR_H + 156);
    lv_obj_set_style_bg_color(bar, UI_C_PANEL, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, UI_C_GOOD, LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar, 6, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 6, LV_PART_INDICATOR);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, percent, LV_ANIM_ON);

    char pct[8];
    snprintf(pct, sizeof(pct), "%d%%", percent);
    lv_obj_t *pctLbl = lv_label_create(scr);
    lv_label_set_text(pctLbl, pct);
    lv_obj_set_style_text_color(pctLbl, UI_C_WARN, 0);
    lv_obj_set_style_text_font(pctLbl, UI_F28, 0);
    lv_obj_align(pctLbl, LV_ALIGN_TOP_MID, 0, UI_HDR_H + 206);

    if (target > saved && a.weeklyEarned > 0) {
        long remaining = target - saved;
        uint32_t weeks = (uint32_t)((remaining + a.weeklyEarned - 1) / a.weeklyEarned);
        char est[40];
        snprintf(est, sizeof(est), "Est. %u weeks at current pace", weeks);
        lv_obj_t *estLbl = lv_label_create(scr);
        lv_label_set_text(estLbl, est);
        lv_obj_set_style_text_color(estLbl, UI_C_DIM, 0);
        lv_obj_set_style_text_font(estLbl, UI_F16, 0);
        lv_obj_set_pos(estLbl, UI_BTN_X, UI_HDR_H + 252);
    }

    uiPlatformAddBtn(scr, DISP_H - UI_BTN_H - 16, "BACK", UI_C_BAD,
                     [](lv_event_t *) { lv_async_call(_go_account, NULL); }, NULL);
    uiPlatformLoadScreen(scr);
}

/* ================================================================
   CHORES SCREEN
   ================================================================ */
static void on_chore_btn(lv_event_t *e) {
    int i = (int)(intptr_t)lv_event_get_user_data(e);
    if (modelRequestChore(g_bank.selectedAccount, i)) {
        static char titleBuf[32];
        strncpy(titleBuf, g_bank.chores[i].title, 31);
        titleBuf[31] = '\0';
        lv_async_call([](void *) {
            draw_message("JOB REQUESTED", titleBuf, "Waiting for Dad", false, MSG_RET_CHORES);
        }, NULL);
    } else {
        lv_async_call([](void *) {
            draw_message("NOT AVAILABLE", "Already pending", "", true, MSG_RET_CHORES);
        }, NULL);
    }
}

static void draw_chores() {
    g_bank.currentScreen = SCR_CHORES;
    lv_obj_t *scr = uiPlatformMakeScreen("CHORES", UI_C_ACCENT);

    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "Tap a job to request it");
    lv_obj_set_style_text_color(hint, UI_C_DIM, 0);
    lv_obj_set_style_text_font(hint, UI_F16, 0);
    lv_obj_set_pos(hint, UI_BTN_X, UI_HDR_H + 12);

    int listH = DISP_H - UI_HDR_H - 52 - UI_BTN_H - 32;
    lv_obj_t *list = uiPlatformMakeScrollList(scr, UI_HDR_H + 44, listH);

    for (int i = 0; i < g_bank.choreCount; i++) {
        if (!g_bank.chores[i].enabled || g_bank.chores[i].title[0] == '\0') continue;

        char lbl[48];
        if (g_bank.chores[i].pendingChild == g_bank.selectedAccount)
            snprintf(lbl, sizeof(lbl), "PENDING");
        else if (g_bank.chores[i].pendingChild >= 0)
            snprintf(lbl, sizeof(lbl), "TAKEN");
        else
            snprintf(lbl, sizeof(lbl), "%s  %s", g_bank.chores[i].title,
                     modelMoneyText(g_bank.chores[i].rewardPence).c_str());

        lv_color_t col = UI_C_PANEL;
        if (g_bank.chores[i].pendingChild == g_bank.selectedAccount) col = UI_C_WARN;
        else if (g_bank.chores[i].pendingChild >= 0) col = UI_C_BAD;

        uiPlatformMakeBtn(list, 0, 0, UI_BTN_W - 16, UI_BTN_H - 8, lbl, col, on_chore_btn, (void *)(intptr_t)i);
    }

    uiPlatformAddBtn(scr, DISP_H - UI_BTN_H - 16, "BACK", UI_C_BAD,
                     [](lv_event_t *) { lv_async_call(_go_account, NULL); }, NULL);
    uiPlatformLoadScreen(scr);
}

/* ================================================================
   ACHIEVEMENTS SCREEN
   ================================================================ */
static void draw_achievements() {
    g_bank.currentScreen = SCR_ACHIEVEMENTS;
    modelCheckAchievements(g_bank.selectedAccount);
    lv_obj_t *scr = uiPlatformMakeScreen("BADGES", UI_C_ACCENT);

    int listH = DISP_H - UI_HDR_H - 40 - UI_BTN_H - 32;
    lv_obj_t *list = uiPlatformMakeScrollList(scr, UI_HDR_H + 24, listH);

    for (int i = 0; i < ACH_COUNT; i++) {
        bool earned = modelHasAchievement(g_bank.selectedAccount, (AchievementId)i);
        lv_obj_t *row = lv_obj_create(list);
        lv_obj_set_size(row, UI_BTN_W - 16, 64);
        lv_obj_set_style_bg_color(row, earned ? UI_C_GOOD : UI_C_PANEL, 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(row, 6, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *chk = lv_label_create(row);
        lv_label_set_text(chk, earned ? "[X]" : "[ ]");
        lv_obj_set_style_text_font(chk, UI_F24, 0);
        lv_obj_set_style_text_color(chk, UI_C_TEXT, 0);
        lv_obj_align(chk, LV_ALIGN_LEFT_MID, 8, 0);

        lv_obj_t *lbl = lv_label_create(row);
        lv_label_set_text(lbl, modelAchievementName((AchievementId)i));
        lv_obj_set_style_text_font(lbl, UI_F20, 0);
        lv_obj_set_style_text_color(lbl, UI_C_TEXT, 0);
        lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 56, 0);
    }

    uiPlatformAddBtn(scr, DISP_H - UI_BTN_H - 16, "BACK", UI_C_BAD,
                     [](lv_event_t *) { lv_async_call(_go_account, NULL); }, NULL);
    uiPlatformLoadScreen(scr);
}

/* ================================================================
   LEADERBOARD SCREEN
   ================================================================ */
static void draw_leaderboard() {
    g_bank.currentScreen = SCR_LEADERBOARD;
    lv_obj_t *scr = uiPlatformMakeScreen("TOP SAVERS", UI_C_ACCENT);

    int order[BANK_MAX_ACCOUNTS];
    int count = 0;
    for (int i = 0; i < BANK_MAX_ACCOUNTS; i++) {
        if (g_bank.accounts[i].active) order[count++] = i;
    }

    for (int i = 0; i < count - 1; i++)
        for (int j = i + 1; j < count; j++)
            if (g_bank.accounts[order[j]].balance > g_bank.accounts[order[i]].balance) {
                int t = order[i];
                order[i] = order[j];
                order[j] = t;
            }

    int listH = DISP_H - UI_HDR_H - 40 - UI_BTN_H - 32;
    lv_obj_t *list = uiPlatformMakeScrollList(scr, UI_HDR_H + 24, listH);

    for (int i = 0; i < count; i++) {
        int a = order[i];
        lv_obj_t *row = lv_obj_create(list);
        lv_obj_set_size(row, UI_BTN_W - 16, 72);
        lv_obj_set_style_bg_color(row, i == 0 ? UI_C_WARN : UI_C_PANEL, 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(row, 6, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);

        char pos[4];
        snprintf(pos, sizeof(pos), "%d.", i + 1);
        lv_obj_t *posLbl = lv_label_create(row);
        lv_label_set_text(posLbl, pos);
        lv_obj_set_style_text_font(posLbl, UI_F24, 0);
        lv_obj_set_style_text_color(posLbl, UI_C_TEXT, 0);
        lv_obj_align(posLbl, LV_ALIGN_LEFT_MID, 8, 0);

        lv_obj_t *nameLbl = lv_label_create(row);
        lv_label_set_text(nameLbl, g_bank.accounts[a].name);
        lv_obj_set_style_text_font(nameLbl, UI_F24, 0);
        lv_obj_set_style_text_color(nameLbl, UI_C_TEXT, 0);
        lv_obj_align(nameLbl, LV_ALIGN_LEFT_MID, 48, 0);

        lv_obj_t *balLbl = lv_label_create(row);
        lv_label_set_text(balLbl, modelMoneyText(g_bank.accounts[a].balance).c_str());
        lv_obj_set_style_text_font(balLbl, UI_F20, 0);
        lv_obj_set_style_text_color(balLbl, UI_C_GOOD, 0);
        lv_obj_align(balLbl, LV_ALIGN_RIGHT_MID, -8, 0);
    }

    uiPlatformAddBtn(scr, DISP_H - UI_BTN_H - 16, "BACK", UI_C_BAD,
                     [](lv_event_t *) { lv_async_call(_go_account, NULL); }, NULL);
    uiPlatformLoadScreen(scr);
}

/* ================================================================
   SHOP SCREEN
   ================================================================ */
static void on_shop_btn(lv_event_t *e) {
    int i = (int)(intptr_t)lv_event_get_user_data(e);
    if (modelPurchaseShopItem(g_bank.selectedAccount, i)) {
        static char itemBuf[24];
        strncpy(itemBuf, g_bank.shop[i].title, 23);
        itemBuf[23] = '\0';
        lv_async_call([](void *) {
            draw_message("PURCHASED", itemBuf, "Enjoy your reward!", false, MSG_RET_SHOP);
        }, NULL);
    } else {
        lv_async_call([](void *) {
            draw_message("CANNOT BUY", "Not enough funds", "", true, MSG_RET_SHOP);
        }, NULL);
    }
}

static void draw_shop() {
    g_bank.currentScreen = SCR_SHOP;
    lv_obj_t *scr = uiPlatformMakeScreen("REWARD SHOP", UI_C_ACCENT);

    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "Spend pocket money on rewards");
    lv_obj_set_style_text_color(hint, UI_C_DIM, 0);
    lv_obj_set_style_text_font(hint, UI_F16, 0);
    lv_obj_set_pos(hint, UI_BTN_X, UI_HDR_H + 12);

    int listH = DISP_H - UI_HDR_H - 52 - UI_BTN_H - 32;
    lv_obj_t *list = uiPlatformMakeScrollList(scr, UI_HDR_H + 44, listH);

    for (int i = 0; i < g_bank.shopCount; i++) {
        if (!g_bank.shop[i].enabled || g_bank.shop[i].title[0] == '\0') continue;
        char lbl[40];
        snprintf(lbl, sizeof(lbl), "%s  %s", g_bank.shop[i].title,
                 modelMoneyText(g_bank.shop[i].pricePence).c_str());
        uiPlatformMakeBtn(list, 0, 0, UI_BTN_W - 16, UI_BTN_H - 8, lbl, UI_C_PANEL,
                          on_shop_btn, (void *)(intptr_t)i);
    }

    uiPlatformAddBtn(scr, DISP_H - UI_BTN_H - 16, "BACK", UI_C_BAD,
                     [](lv_event_t *) { lv_async_call(_go_account, NULL); }, NULL);
    uiPlatformLoadScreen(scr);
}

/* ================================================================
   STATS SCREEN
   ================================================================ */
static void draw_stats() {
    g_bank.currentScreen = SCR_STATS;
    ChildAccount &a = g_bank.accounts[g_bank.selectedAccount];
    lv_obj_t *scr = uiPlatformMakeScreen("STATISTICS", UI_C_ACCENT);

    int listH = DISP_H - UI_HDR_H - 24 - UI_BTN_H - 32;
    lv_obj_t *list = uiPlatformMakeScrollList(scr, UI_HDR_H + 16, listH);

    auto addStat = [&](const char *line) {
        lv_obj_t *l = lv_label_create(list);
        lv_label_set_text(l, line);
        lv_obj_set_style_text_color(l, UI_C_TEXT, 0);
        lv_obj_set_style_text_font(l, UI_F20, 0);
    };

    char buf[64];
    snprintf(buf, sizeof(buf), "Lifetime earned: %s", modelMoneyText(a.lifetimeEarned).c_str());
    addStat(buf);
    snprintf(buf, sizeof(buf), "Current balance: %s", modelMoneyText(a.balance).c_str());
    addStat(buf);
    snprintf(buf, sizeof(buf), "Money spent: %s", modelMoneyText(a.moneySpent).c_str());
    addStat(buf);
    snprintf(buf, sizeof(buf), "Money saved: %s", modelMoneyText(a.savingsBalance).c_str());
    addStat(buf);
    snprintf(buf, sizeof(buf), "Weekly earnings: %s", modelMoneyText(a.weeklyEarned).c_str());
    addStat(buf);
    snprintf(buf, sizeof(buf), "Monthly earnings: %s", modelMoneyText(a.monthlyEarned).c_str());
    addStat(buf);
    snprintf(buf, sizeof(buf), "Chores completed: %u", a.choresCompleted);
    addStat(buf);
    snprintf(buf, sizeof(buf), "Longest streak: %u days", a.longestStreak);
    addStat(buf);
    snprintf(buf, sizeof(buf), "Family Dad tax: %s",
             modelMoneyText(g_bank.config.totalDadTaxCollected).c_str());
    addStat(buf);

    uiPlatformAddBtn(scr, DISP_H - UI_BTN_H - 16, "BACK", UI_C_BAD,
                     [](lv_event_t *) { lv_async_call(_go_account, NULL); }, NULL);
    uiPlatformLoadScreen(scr);
}

/* ================================================================
   NOTIFICATIONS SCREEN
   ================================================================ */
static void draw_notifications() {
    g_bank.currentScreen = SCR_NOTIFICATIONS;
    modelMarkNotificationsRead();
    lv_obj_t *scr = uiPlatformMakeScreen("ALERTS", UI_C_ACCENT);

    int listH = DISP_H - UI_HDR_H - 40 - UI_BTN_H - 32;
    lv_obj_t *list = uiPlatformMakeScrollList(scr, UI_HDR_H + 24, listH);

    if (g_bank.notificationCount == 0) {
        lv_obj_t *none = lv_label_create(list);
        lv_label_set_text(none, "No notifications");
        lv_obj_set_style_text_color(none, UI_C_DIM, 0);
        lv_obj_set_style_text_font(none, UI_F24, 0);
    } else {
        for (int i = (int)g_bank.notificationCount - 1; i >= 0; i--) {
            lv_obj_t *row = lv_label_create(list);
            lv_label_set_text(row, g_bank.notifications[i].text);
            lv_obj_set_style_text_color(row, g_bank.notifications[i].unread ? UI_C_WARN : UI_C_TEXT, 0);
            lv_obj_set_style_text_font(row, UI_F20, 0);
        }
    }

    uiPlatformAddBtn(scr, DISP_H - UI_BTN_H - 16, "BACK", UI_C_BAD,
                     [](lv_event_t *) { lv_async_call(_go_account, NULL); }, NULL);
    uiPlatformLoadScreen(scr);
}

/* ================================================================
   ADMIN MENU
   ================================================================ */
static void draw_admin_menu() {
    g_bank.currentScreen = SCR_ADMIN_MENU;
    g_bank.adminMode = true;

    lv_obj_t *scr = uiPlatformMakeScreen("DAD ADMIN", UI_C_BAD);

    char hint[80];
    snprintf(hint, sizeof(hint), "Web: %s  ->  192.168.4.1", BANK_WIFI_AP_SSID);
    lv_obj_t *wifiLbl = lv_label_create(scr);
    lv_label_set_text(wifiLbl, hint);
    lv_obj_set_style_text_color(wifiLbl, UI_C_DIM, 0);
    lv_obj_set_style_text_font(wifiLbl, UI_F14, 0);
    lv_obj_set_pos(wifiLbl, UI_BTN_X, UI_HDR_H + 6);

    char sdHint[56];
    snprintf(sdHint, sizeof(sdHint), "%s%s", storageStatusText(),
             storageReady() ? " — history + backup" : "");
    lv_obj_t *sdLbl = lv_label_create(scr);
    lv_label_set_text(sdLbl, sdHint);
    lv_obj_set_style_text_color(sdLbl, storageReady() ? UI_C_GOOD : UI_C_DIM, 0);
    lv_obj_set_style_text_font(sdLbl, UI_F14, 0);
    lv_obj_set_pos(sdLbl, UI_BTN_X, UI_HDR_H + 26);

    int y = UI_HDR_H + 52;
    y = uiPlatformAddBtn(scr, y, "EDIT MONEY", UI_C_GOOD,
                         [](lv_event_t *) { lv_async_call(_go_admin_select_act, NULL); }, NULL);
    y = uiPlatformAddBtn(scr, y, "APPROVALS", UI_C_WARN,
                         [](lv_event_t *) { lv_async_call(_go_approvals, NULL); }, NULL);
    y = uiPlatformAddBtn(scr, y, "DAD TAX", UI_C_BAD,
                         [](lv_event_t *) { lv_async_call(_go_admin_select_tax, NULL); }, NULL);
    y = uiPlatformAddBtn(scr, y, "DIAGNOSTICS", UI_C_PANEL,
                         [](lv_event_t *) { lv_async_call(_go_admin_diag, NULL); }, NULL);

    uiPlatformAddBtn(scr, DISP_H - UI_BTN_H - 16, "HOME", UI_C_PANEL,
                     [](lv_event_t *) { lv_async_call(_go_home, NULL); }, NULL);
    uiPlatformLoadScreen(scr);
}

/* ================================================================
   ADMIN SELECT
   ================================================================ */
static void on_admin_select_btn(lv_event_t *e) {
    g_bank.selectedAccount = (int)(intptr_t)lv_event_get_user_data(e);
    if (g_bank.g_taxPath) lv_async_call(_go_dad_tax, NULL);
    else lv_async_call(_go_admin_action, NULL);
}

static void draw_admin_select() {
    g_bank.currentScreen = SCR_ADMIN_SELECT;
    lv_obj_t *scr = uiPlatformMakeScreen("SELECT CHILD", UI_C_BAD);

    int y = UI_HDR_H + 40;
    for (int i = 0; i < BANK_MAX_ACCOUNTS; i++) {
        if (!g_bank.accounts[i].active) continue;
        y = uiPlatformAddBtn(scr, y, g_bank.accounts[i].name, UI_C_PANEL,
                             on_admin_select_btn, (void *)(intptr_t)i);
    }

    uiPlatformAddBtn(scr, DISP_H - UI_BTN_H - 16, "BACK", UI_C_BAD,
                     [](lv_event_t *) { lv_async_call(_go_admin_menu, NULL); }, NULL);
    uiPlatformLoadScreen(scr);
}

/* ================================================================
   ADMIN ACTION
   ================================================================ */
static void draw_admin_action() {
    g_bank.currentScreen = SCR_ADMIN_ACTION;
    g_bank.g_taxPath = false;

    lv_obj_t *scr = uiPlatformMakeScreen(g_bank.accounts[g_bank.selectedAccount].name, UI_C_BAD);

    lv_obj_t *bal = lv_label_create(scr);
    lv_label_set_text(bal, modelMoneyText(g_bank.accounts[g_bank.selectedAccount].balance).c_str());
    lv_obj_set_style_text_color(bal, UI_C_GOOD, 0);
    lv_obj_set_style_text_font(bal, UI_F28, 0);
    lv_obj_set_pos(bal, UI_BTN_X, UI_HDR_H + 16);

    int bw = (UI_BTN_W - UI_BTN_GAP) / 2;
    int y = UI_HDR_H + 72;
    uiPlatformMakeBtn(scr, UI_BTN_X, y, bw, UI_BTN_H, "DEPOSIT", UI_C_GOOD,
                      [](lv_event_t *) { lv_async_call(_go_deposit, NULL); }, NULL);
    uiPlatformMakeBtn(scr, UI_BTN_X + bw + UI_BTN_GAP, y, bw, UI_BTN_H, "WITHDRAW", UI_C_BAD,
                      [](lv_event_t *) { lv_async_call(_go_withdraw, NULL); }, NULL);

    int y1 = y + UI_BTN_H + UI_BTN_GAP;
    uiPlatformMakeBtn(scr, UI_BTN_X, y1, bw, UI_BTN_H, "BONUS", UI_C_WARN,
                      [](lv_event_t *) { lv_async_call(_go_bonus, NULL); }, NULL);
    uiPlatformMakeBtn(scr, UI_BTN_X + bw + UI_BTN_GAP, y1, bw, UI_BTN_H, "FINE", UI_C_BAD,
                      [](lv_event_t *) { lv_async_call(_go_fine, NULL); }, NULL);

    uiPlatformAddBtn(scr, DISP_H - UI_BTN_H - 16, "BACK", UI_C_PANEL,
                     [](lv_event_t *) { lv_async_call(_go_admin_select_act, NULL); }, NULL);
    uiPlatformLoadScreen(scr);
}

/* ================================================================
   APPROVALS SCREEN
   ================================================================ */
static void on_approve_btn(lv_event_t *e) {
    int i = (int)(intptr_t)lv_event_get_user_data(e);
    int child = g_bank.chores[i].pendingChild;
    if (child < 0) return;

    long reward = g_bank.chores[i].rewardPence;
    uint32_t maskBefore = g_bank.accounts[child].achievementMask;
    modelApproveChore(i);
    uint32_t maskAfter = g_bank.accounts[child].achievementMask;
    if (maskAfter != maskBefore) {
        for (int a = 0; a < ACH_COUNT; a++) {
            if (!(maskBefore & (1UL << a)) && (maskAfter & (1UL << a))) {
                uiFxShowAchievement("ACHIEVEMENT!", modelAchievementName((AchievementId)a));
                break;
            }
        }
    }

    static char nameBuf[16];
    strncpy(nameBuf, g_bank.accounts[child].name, 15);
    nameBuf[15] = '\0';
    static String rewardStr;
    rewardStr = "+" + modelMoneyText(reward);

    lv_async_call([](void *) {
        draw_message("APPROVED", nameBuf, rewardStr.c_str(), false, MSG_RET_ADMIN_MENU);
    }, NULL);
}

static void on_reject_btn(lv_event_t *e) {
    int i = (int)(intptr_t)lv_event_get_user_data(e);
    modelRejectChore(i);
    lv_async_call(_go_approvals, NULL);
}

static void draw_approvals() {
    g_bank.currentScreen = SCR_APPROVALS;
    lv_obj_t *scr = uiPlatformMakeScreen("APPROVALS", UI_C_BAD);

    int listH = DISP_H - UI_HDR_H - 40 - UI_BTN_H - 32;
    lv_obj_t *list = uiPlatformMakeScrollList(scr, UI_HDR_H + 24, listH);
    bool found = false;

    for (int i = 0; i < g_bank.choreCount; i++) {
        if (g_bank.chores[i].pendingChild < 0) continue;
        found = true;
        int child = g_bank.chores[i].pendingChild;

        lv_obj_t *row = lv_obj_create(list);
        lv_obj_set_size(row, UI_BTN_W - 16, UI_BTN_H + 8);
        lv_obj_set_style_bg_color(row, UI_C_PANEL, 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(row, 6, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);

        char lbl[48];
        snprintf(lbl, sizeof(lbl), "%s: %s", g_bank.accounts[child].name, g_bank.chores[i].title);
        lv_obj_t *title = lv_label_create(row);
        lv_label_set_text(title, lbl);
        lv_obj_set_style_text_font(title, UI_F16, 0);
        lv_obj_set_style_text_color(title, UI_C_TEXT, 0);
        lv_obj_set_pos(title, 8, 4);

        int half = (UI_BTN_W - 32 - UI_BTN_GAP) / 2;
        uiPlatformMakeBtn(row, 8, 36, half, 44, "APPROVE", UI_C_GOOD, on_approve_btn, (void *)(intptr_t)i);
        uiPlatformMakeBtn(row, 8 + half + UI_BTN_GAP, 36, half, 44, "REJECT", UI_C_BAD,
                          on_reject_btn, (void *)(intptr_t)i);
    }

    if (!found) {
        lv_obj_t *none = lv_label_create(list);
        lv_label_set_text(none, "No pending approvals");
        lv_obj_set_style_text_color(none, UI_C_DIM, 0);
        lv_obj_set_style_text_font(none, UI_F24, 0);
    }

    uiPlatformAddBtn(scr, DISP_H - UI_BTN_H - 16, "BACK", UI_C_BAD,
                     [](lv_event_t *) { lv_async_call(_go_admin_menu, NULL); }, NULL);
    uiPlatformLoadScreen(scr);
}

/* ================================================================
   DAD TAX SCREEN
   ================================================================ */
static void draw_dad_tax() {
    g_bank.currentScreen = SCR_DAD_TAX;
    ChildAccount &a = g_bank.accounts[g_bank.selectedAccount];
    long tax = (a.balance * g_bank.config.dadTaxPercent) / 100;

    lv_obj_t *scr = uiPlatformMakeScreen("DAD TAX", UI_C_BAD);

    lv_obj_t *name = lv_label_create(scr);
    lv_label_set_text(name, a.name);
    lv_obj_set_style_text_color(name, UI_C_TEXT, 0);
    lv_obj_set_style_text_font(name, UI_F28, 0);
    lv_obj_set_pos(name, UI_BTN_X, UI_HDR_H + 16);

    auto addRow = [&](int y, const char *lbl, lv_color_t col, const char *val) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s%s", lbl, val);
        lv_obj_t *l = lv_label_create(scr);
        lv_label_set_text(l, buf);
        lv_obj_set_style_text_color(l, col, 0);
        lv_obj_set_style_text_font(l, UI_F20, 0);
        lv_obj_set_pos(l, UI_BTN_X, y);
    };

    addRow(UI_HDR_H + 72, "Balance:  ", UI_C_TEXT, modelMoneyText(a.balance).c_str());
    char taxLbl[16];
    snprintf(taxLbl, sizeof(taxLbl), "Tax (%u%%): -", g_bank.config.dadTaxPercent);
    addRow(UI_HDR_H + 108, taxLbl, UI_C_BAD, modelMoneyText(tax).c_str());
    addRow(UI_HDR_H + 144, "After:    ", UI_C_GOOD, modelMoneyText(a.balance - tax).c_str());

    if (tax <= 0 || !g_bank.config.dadTaxEnabled) {
        lv_obj_t *zero = lv_label_create(scr);
        lv_label_set_text(zero, g_bank.config.dadTaxEnabled ? "Nothing to tax!" : "Dad tax is disabled");
        lv_obj_set_style_text_color(zero, UI_C_WARN, 0);
        lv_obj_set_style_text_font(zero, UI_F24, 0);
        lv_obj_set_pos(zero, UI_BTN_X, UI_HDR_H + 200);
        uiPlatformAddBtn(scr, DISP_H - UI_BTN_H - 16, "BACK", UI_C_PANEL,
                         [](lv_event_t *) { lv_async_call(_go_admin_menu, NULL); }, NULL);
    } else {
        uiPlatformAddBtn(scr, UI_HDR_H + 220, "APPLY TAX", UI_C_BAD,
                         [](lv_event_t *) {
                             long t = 0;
                             if (modelApplyDadTax(g_bank.selectedAccount, &t)) {
                                 static String msg;
                                 msg = "-" + modelMoneyText(t);
                                 lv_async_call([](void *) {
                                     draw_message("TAX APPLIED", msg.c_str(),
                                                  modelMoneyText(g_bank.accounts[g_bank.selectedAccount].balance).c_str(),
                                                  false, MSG_RET_ADMIN_MENU);
                                 }, NULL);
                             }
                         }, NULL);
        uiPlatformAddBtn(scr, UI_HDR_H + 220 + UI_BTN_H + UI_BTN_GAP, "CANCEL", UI_C_PANEL,
                         [](lv_event_t *) { lv_async_call(_go_admin_menu, NULL); }, NULL);
    }

    uiPlatformLoadScreen(scr);
}

/* ================================================================
   AMOUNT SCREEN
   ================================================================ */
static void refreshAmountLabel() {
    if (!s_amountLabel) return;
    long p = atol(g_bank.amountInput);
    String s = "Amount  " + modelMoneyText(p);
    lv_label_set_text(s_amountLabel, s.c_str());
}

static const char *amountScreenTitle(AmountPurpose p) {
    switch (p) {
        case AMOUNT_DEPOSIT:  return "DEPOSIT";
        case AMOUNT_WITHDRAW: return "WITHDRAW";
        case AMOUNT_BONUS:    return "BONUS";
        case AMOUNT_FINE:     return "FINE";
        default:              return "AMOUNT";
    }
}

static lv_color_t amountScreenColor(AmountPurpose p) {
    switch (p) {
        case AMOUNT_DEPOSIT:
        case AMOUNT_BONUS: return UI_C_GOOD;
        case AMOUNT_WITHDRAW:
        case AMOUNT_FINE:  return UI_C_BAD;
        default:           return UI_C_ACCENT;
    }
}

static void on_amount_key(lv_event_t *e) {
    const char *key = (const char *)lv_event_get_user_data(e);
    if (!key) return;

    if (key[0] == '*') {
        lv_async_call(_go_admin_action, NULL);
        return;
    }
    if (key[0] == '#') {
        long pennies = atol(g_bank.amountInput);
        if (pennies <= 0) {
            lv_async_call([](void *) {
                draw_message("NO AMOUNT", "Enter value", "", true, MSG_RET_ADMIN_ACTION);
            }, NULL);
            return;
        }

        bool ok = false;
        switch (s_amountPurpose) {
            case AMOUNT_DEPOSIT:
                ok = modelDeposit(g_bank.selectedAccount, pennies);
                break;
            case AMOUNT_WITHDRAW:
                ok = modelWithdraw(g_bank.selectedAccount, pennies);
                break;
            case AMOUNT_BONUS:
                ok = modelAwardBonus(g_bank.selectedAccount, pennies);
                break;
            case AMOUNT_FINE:
                ok = modelApplyFine(g_bank.selectedAccount, pennies);
                break;
        }

        if (!ok) {
            lv_async_call([](void *) {
                draw_message("FAILED", "Check balance", "", true, MSG_RET_ADMIN_ACTION);
            }, NULL);
            return;
        }

        static bool wasCredit;
        static String l1, l2;
        wasCredit = (s_amountPurpose == AMOUNT_DEPOSIT || s_amountPurpose == AMOUNT_BONUS);
        l1 = (wasCredit ? "+" : "-") + modelMoneyText(pennies);
        l2 = "Bal " + modelMoneyText(g_bank.accounts[g_bank.selectedAccount].balance);
        lv_async_call([](void *) {
            draw_message(wasCredit ? "CREDIT OK" : "DEBIT OK", l1.c_str(), l2.c_str(), false, MSG_RET_ADMIN_MENU);
        }, NULL);
        return;
    }

    int n = strlen(g_bank.amountInput);
    if (n < 6) {
        g_bank.amountInput[n] = key[0];
        g_bank.amountInput[n + 1] = '\0';
        refreshAmountLabel();
    }
}

static void draw_amount(AmountPurpose purpose) {
    g_bank.currentScreen = SCR_AMOUNT;
    s_amountPurpose = purpose;
    memset(g_bank.amountInput, 0, sizeof(g_bank.amountInput));

    lv_obj_t *scr = uiPlatformMakeScreen(amountScreenTitle(purpose), amountScreenColor(purpose));

    s_amountLabel = lv_label_create(scr);
    lv_label_set_text(s_amountLabel, "Amount  GBP 0.00");
    lv_obj_set_style_text_color(s_amountLabel, UI_C_TEXT, 0);
    lv_obj_set_style_text_font(s_amountLabel, UI_F28, 0);
    lv_obj_set_pos(s_amountLabel, UI_BTN_X, UI_HDR_H + 18);

    lv_obj_t *hint2 = lv_label_create(scr);
    lv_label_set_text(hint2, "Enter pennies  (e.g. 250 = GBP 2.50)");
    lv_obj_set_style_text_color(hint2, UI_C_DIM, 0);
    lv_obj_set_style_text_font(hint2, UI_F14, 0);
    lv_obj_set_pos(hint2, UI_BTN_X, UI_HDR_H + 60);

    uiPlatformBuildKeypad(scr, UI_HDR_H + 90, on_amount_key);
    uiPlatformLoadScreen(scr);
}

/* ================================================================
   ADMIN DIAGNOSTICS
   ================================================================ */
static void draw_admin_diag() {
    g_bank.currentScreen = SCR_ADMIN_DIAG;
    lv_obj_t *scr = uiPlatformMakeScreen("DIAGNOSTICS", UI_C_BAD);

    int listH = DISP_H - UI_HDR_H - 40 - UI_BTN_H - 32;
    lv_obj_t *list = uiPlatformMakeScrollList(scr, UI_HDR_H + 24, listH);

    auto addLine = [&](const char *line) {
        lv_obj_t *l = lv_label_create(list);
        lv_label_set_text(l, line);
        lv_obj_set_style_text_color(l, UI_C_TEXT, 0);
        lv_obj_set_style_text_font(l, UI_F20, 0);
    };

    char buf[72];
    snprintf(buf, sizeof(buf), "Storage v%u", g_bank.config.storageVersion);
    addLine(buf);
    snprintf(buf, sizeof(buf), "Chores: %u  Shop: %u", g_bank.choreCount, g_bank.shopCount);
    addLine(buf);
    snprintf(buf, sizeof(buf), "Dad tax total: %s",
             modelMoneyText(g_bank.config.totalDadTaxCollected).c_str());
    addLine(buf);
    snprintf(buf, sizeof(buf), "Interest paid: %s",
             modelMoneyText(g_bank.config.totalInterestPaid).c_str());
    addLine(buf);
    snprintf(buf, sizeof(buf), "Tax %u%% %s  Interest %u%% %s",
             g_bank.config.dadTaxPercent, g_bank.config.dadTaxEnabled ? "ON" : "OFF",
             g_bank.config.interestRatePercent, g_bank.config.interestEnabled ? "ON" : "OFF");
    addLine(buf);
    addLine(storageStatusText());
    snprintf(buf, sizeof(buf), "Free heap: %u", (unsigned)ESP.getFreeHeap());
    addLine(buf);
    snprintf(buf, sizeof(buf), "Free PSRAM: %u", (unsigned)ESP.getFreePsram());
    addLine(buf);

    uiPlatformAddBtn(scr, DISP_H - UI_BTN_H - 16, "BACK", UI_C_BAD,
                     [](lv_event_t *) { lv_async_call(_go_admin_menu, NULL); }, NULL);
    uiPlatformLoadScreen(scr);
}

/* ================================================================
   MESSAGE SCREEN
   ================================================================ */
static void on_message_ok(lv_event_t *e) {
    (void)e;
    switch (s_msgReturn) {
        case MSG_RET_PIN:          lv_async_call(_go_pin_child, NULL); break;
        case MSG_RET_ADMIN_PIN:    lv_async_call(_go_pin_admin, NULL); break;
        case MSG_RET_ACCOUNT:      lv_async_call(_go_account, NULL); break;
        case MSG_RET_CHORES:       lv_async_call(_go_chores, NULL); break;
        case MSG_RET_ADMIN_MENU:   lv_async_call(_go_admin_menu, NULL); break;
        case MSG_RET_ADMIN_ACTION: lv_async_call(_go_admin_action, NULL); break;
        case MSG_RET_SHOP:         lv_async_call(_go_shop, NULL); break;
        case MSG_RET_STATS:        lv_async_call(_go_stats, NULL); break;
        default:                   lv_async_call(_go_home, NULL); break;
    }
}

static void draw_message(const char *title, const char *line1,
                         const char *line2, bool isError, MsgReturn ret) {
    g_bank.currentScreen = SCR_MESSAGE;
    s_msgReturn = ret;
    lv_color_t col = isError ? UI_C_BAD : UI_C_GOOD;
    lv_obj_t *scr = uiPlatformMakeScreen(isError ? "ERROR" : "COMPLETE", col);

    lv_obj_t *t = lv_label_create(scr);
    lv_label_set_text(t, title);
    lv_obj_set_style_text_color(t, col, 0);
    lv_obj_set_style_text_font(t, UI_F28, 0);
    lv_obj_set_pos(t, UI_BTN_X, UI_HDR_H + 40);

    lv_obj_t *l1 = lv_label_create(scr);
    lv_label_set_text(l1, line1);
    lv_obj_set_style_text_color(l1, UI_C_TEXT, 0);
    lv_obj_set_style_text_font(l1, UI_F24, 0);
    lv_obj_set_pos(l1, UI_BTN_X, UI_HDR_H + 110);

    if (line2 && line2[0]) {
        lv_obj_t *l2 = lv_label_create(scr);
        lv_label_set_text(l2, line2);
        lv_obj_set_style_text_color(l2, UI_C_TEXT, 0);
        lv_obj_set_style_text_font(l2, UI_F24, 0);
        lv_obj_set_pos(l2, UI_BTN_X, UI_HDR_H + 162);
    }

    uiPlatformAddBtn(scr, DISP_H - UI_BTN_H - 16, "OK", col, on_message_ok, NULL);
    uiPlatformLoadScreen(scr);
}

/* ================================================================
   Public entry points
   ================================================================ */
void uiLvglInit() {
    draw_home();
}

void uiLvglLoop() {
    lv_timer_handler();
}

void uiLvglShowHome() {
    draw_home();
}

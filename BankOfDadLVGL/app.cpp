/*
 * app.cpp — Bank of Dad full LVGL 9 UI
 * Target: JC8048W550C / ESP32-S3 / 480×800 logical portrait display
 *
 * All screens are created fresh on each navigation; the old screen is
 * deleted automatically by lv_screen_load_anim (auto_del = true).
 * Event callbacks use lv_async_call() to defer screen transitions so
 * that the current event completes cleanly before the screen is torn down.
 */

#include "app.h"
#include "lvgl_port.h"   /* DISP_W, DISP_H */
#include "storage.h"

/* ================================================================
   DATA
   ================================================================ */
Account accounts[ACCOUNT_COUNT] = {
    {"Nixon",    "1111", 1250, 5000,  40, false, true,  false},
    {"Oakley",   "2222",  800, 3000,  20, false, false, false},
    {"Hadley",   "3333", 1575, 4500,  65, false, true,  false},
    {"Maddison", "4444", 2200, 6000, 120, false, true,  false}
};

Chore chores[CHORE_COUNT] = {
    {"Feed Pet",    100, -1},
    {"Tidy Room",   200, -1},
    {"Vacuum",      300, -1},
    {"Garden Help", 500, -1}
};

String historyLog[ACCOUNT_COUNT][HISTORY_COUNT];
int    historyIndex[ACCOUNT_COUNT] = {0, 0, 0, 0};

Screen currentScreen  = SCR_HOME;
int    selectedAccount = -1;
bool   adminMode      = false;
bool   depositMode    = true;
char   enteredPin[5]  = "";
char   amountInput[7] = "";

Preferences prefs;
WebServer   webServer(80);

/* ================================================================
   COLOUR PALETTE  (Gore FX: black / red / dark-grey accents)
   ================================================================ */
#define C_BG        lv_color_hex(0x000000)
#define C_PANEL     lv_color_hex(0x2A2A2A)
#define C_TEXT      lv_color_hex(0xFFFFFF)
#define C_GOOD      lv_color_hex(0x27AE60)
#define C_BAD       lv_color_hex(0xC0392B)
#define C_WARN      lv_color_hex(0xE67E22)
#define C_ACCENT    lv_color_hex(0xE74C3C)   /* header red */
#define C_BLUE      lv_color_hex(0x2980B9)
#define C_CYAN      lv_color_hex(0x1ABC9C)
#define C_PRESSED   lv_color_hex(0x555555)

/* ================================================================
   FONT SHORTHANDS
   ================================================================ */
#define F14  (&lv_font_montserrat_14)
#define F16  (&lv_font_montserrat_16)
#define F20  (&lv_font_montserrat_20)
#define F24  (&lv_font_montserrat_24)
#define F28  (&lv_font_montserrat_28)
#define F36  (&lv_font_montserrat_36)

/* ================================================================
   LAYOUT CONSTANTS  (480 wide × 800 tall portrait)
   ================================================================ */
#define HDR_H      70          /* header bar height */
#define BTN_H      85          /* standard button height */
#define BTN_W     (DISP_W-40)  /* standard button width (440) */
#define BTN_X      20          /* left margin for standard buttons */
#define BTN_GAP    14          /* gap between stacked buttons */
#define KEY_W     136          /* keypad key width  (3 cols in 440px) */
#define KEY_H      90          /* keypad key height */
#define KEY_GAP     8          /* keypad key gap */

/* ================================================================
   FORWARD DECLARATIONS
   ================================================================ */
static void draw_home();
static void draw_pin(bool isAdmin);
static void draw_account();
static void draw_history();
static void draw_goals();
static void draw_chores();
static void draw_achievements();
static void draw_leaderboard();
static void draw_admin_menu();
static void draw_admin_select();
static void draw_admin_action();
static void draw_approvals();
static void draw_dad_tax();
static void draw_amount(bool isDeposit);
static void draw_message(const char *title, const char *line1,
                         const char *line2, bool isError, MsgReturn ret);

/* ================================================================
   UTILITY FUNCTIONS
   ================================================================ */
static String moneyText(long pennies) {
    long pounds = pennies / 100;
    int  pence  = (int)abs(pennies % 100);
    String s = "GBP ";
    s += pounds;
    s += ".";
    if (pence < 10) s += "0";
    s += pence;
    return s;
}

static void addHistory(int acct, String entry) {
    historyLog[acct][historyIndex[acct]] = entry;
    if (++historyIndex[acct] >= HISTORY_COUNT) historyIndex[acct] = 0;
    storageAppendHistory(accounts[acct].name, entry.c_str());
}

static void storageBackupAll() {
    StorageAccountSnap snap[ACCOUNT_COUNT];
    for (int i = 0; i < ACCOUNT_COUNT; i++) {
        snap[i] = {accounts[i].name, accounts[i].balance, accounts[i].xp,
                   accounts[i].firstDeposit, accounts[i].firstTen,
                   accounts[i].firstTwentyFive};
    }
    storageBackupAccounts(snap, ACCOUNT_COUNT);
}

static void updateAchievements(int acct) {
    if (accounts[acct].balance >= 1000) accounts[acct].firstTen = true;
    if (accounts[acct].balance >= 2500) accounts[acct].firstTwentyFive = true;
}

static void saveAccount(int i) {
    prefs.begin("bank", false);
    String k = String(i);
    prefs.putLong(("bal" + k).c_str(), accounts[i].balance);
    prefs.putLong(("xp"  + k).c_str(), accounts[i].xp);
    prefs.putBool(("fd"  + k).c_str(), accounts[i].firstDeposit);
    prefs.putBool(("f10" + k).c_str(), accounts[i].firstTen);
    prefs.putBool(("f25" + k).c_str(), accounts[i].firstTwentyFive);
    prefs.end();
    storageBackupAll();
}

static void loadAccounts() {
    prefs.begin("bank", true);
    for (int i = 0; i < ACCOUNT_COUNT; i++) {
        String k = String(i);
        accounts[i].balance         = prefs.getLong(("bal" + k).c_str(), accounts[i].balance);
        accounts[i].xp              = prefs.getLong(("xp"  + k).c_str(), accounts[i].xp);
        accounts[i].firstDeposit    = prefs.getBool(("fd"  + k).c_str(), accounts[i].firstDeposit);
        accounts[i].firstTen        = prefs.getBool(("f10" + k).c_str(), accounts[i].firstTen);
        accounts[i].firstTwentyFive = prefs.getBool(("f25" + k).c_str(), accounts[i].firstTwentyFive);
    }
    prefs.end();
}

static void saveChores() {
    prefs.begin("bank", false);
    for (int i = 0; i < CHORE_COUNT; i++) {
        String k = String(i);
        prefs.putString(("ct" + k).c_str(), chores[i].title);
        prefs.putLong  (("cr" + k).c_str(), chores[i].reward);
    }
    prefs.end();
}

static void loadChores() {
    prefs.begin("bank", true);
    for (int i = 0; i < CHORE_COUNT; i++) {
        String k     = String(i);
        String title = prefs.getString(("ct" + k).c_str(), chores[i].title);
        title.toCharArray(chores[i].title, 32);
        chores[i].reward = prefs.getLong(("cr" + k).c_str(), chores[i].reward);
    }
    prefs.end();
}

/* ================================================================
   WEB ADMIN — Chore Editor  (same logic as CYD version)
   ================================================================ */
static String buildChorePage(bool saved) {
    String html =
        "<!DOCTYPE html><html><head>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>Bank of Dad</title><style>"
        "body{font-family:sans-serif;max-width:480px;margin:0 auto;padding:16px;"
          "background:#111;color:#eee}"
        "h1{color:#e74c3c;margin:0 0 2px}"
        "p.sub{color:#888;margin:0 0 16px;font-size:.9em}"
        ".chore{background:#222;padding:12px;margin:8px 0;border-radius:8px}"
        "label{display:block;margin-bottom:4px;font-size:.85em;color:#aaa}"
        "input[type=text],input[type=number]{"
          "width:100%;box-sizing:border-box;padding:8px;"
          "background:#333;color:#fff;border:1px solid #555;"
          "border-radius:4px;margin-bottom:8px}"
        "button{width:100%;padding:12px;background:#e74c3c;color:#fff;"
          "border:none;border-radius:8px;font-size:1em;cursor:pointer;margin-top:8px}"
        "button:hover{background:#c0392b}"
        ".ok{padding:10px;background:#27ae60;border-radius:6px;"
          "margin-bottom:12px;text-align:center}"
        ".warn{font-size:.8em;color:#e67e22;margin-top:4px}"
        "</style></head><body>"
        "<h1>Bank of Dad</h1>"
        "<p class='sub'>Chore Editor &mdash; changes take effect immediately</p>";

    if (saved) html += "<div class='ok'>&#10003; Changes saved!</div>";
    html += "<form method='POST' action='/update'>";

    for (int i = 0; i < CHORE_COUNT; i++) {
        char buf[12];
        dtostrf(chores[i].reward / 100.0f, 1, 2, buf);
        String pending;
        if (chores[i].pendingChild >= 0)
            pending = "<p class='warn'>&#9888; Pending: " +
                      String(accounts[chores[i].pendingChild].name) + "</p>";
        html += "<div class='chore'>"
                "<label>Chore " + String(i + 1) + " &mdash; Name</label>"
                "<input type='text' name='t" + String(i) + "' value=\"" +
                String(chores[i].title) + "\" maxlength='31' required>"
                "<label>Reward (GBP)</label>"
                "<input type='number' name='r" + String(i) +
                "' step='0.01' min='0' value='" + String(buf) + "'>" +
                pending + "</div>";
    }
    html += "<button type='submit'>Save Chores</button></form></body></html>";
    return html;
}

static void setupWebServer() {
    webServer.on("/", HTTP_GET, []() {
        webServer.send(200, "text/html", buildChorePage(webServer.hasArg("saved")));
    });
    webServer.on("/update", HTTP_POST, []() {
        for (int i = 0; i < CHORE_COUNT; i++) {
            String tk = "t" + String(i);
            String rk = "r" + String(i);
            if (webServer.hasArg(tk)) {
                String v = webServer.arg(tk);
                v.trim();
                if (v.length() > 0 && v.length() < 32) {
                    strncpy(chores[i].title, v.c_str(), 31);
                    chores[i].title[31] = '\0';
                }
            }
            if (webServer.hasArg(rk)) {
                float p = webServer.arg(rk).toFloat();
                if (p >= 0.0f) chores[i].reward = (long)(p * 100.0f + 0.5f);
            }
        }
        saveChores();
        webServer.sendHeader("Location", "/?saved=1");
        webServer.send(302, "text/plain", "");
    });
    webServer.onNotFound([]() {
        webServer.sendHeader("Location", "/");
        webServer.send(302, "text/plain", "");
    });
}

/* ================================================================
   SCREEN BUILDER HELPERS
   ================================================================ */

/* Load a new screen, deleting the previous one automatically */
static void loadScreen(lv_obj_t *scr) {
    lv_screen_load_anim(scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
}

/* Create a blank screen with black background and a coloured header bar.
   Returns the screen object; content should be added relative to it.   */
static lv_obj_t *makeScreen(const char *title, lv_color_t hdrCol) {
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(scr, C_BG, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);
    lv_obj_set_style_border_width(scr, 0, 0);

    /* Header bar */
    lv_obj_t *hdr = lv_obj_create(scr);
    lv_obj_set_size(hdr, DISP_W, HDR_H);
    lv_obj_set_pos(hdr, 0, 0);
    lv_obj_remove_flag(hdr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(hdr, hdrCol, 0);
    lv_obj_set_style_bg_opa(hdr, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(hdr, 0, 0);
    lv_obj_set_style_radius(hdr, 0, 0);
    lv_obj_set_style_pad_left(hdr, 16, 0);
    lv_obj_set_style_pad_ver(hdr, 0, 0);

    lv_obj_t *lbl = lv_label_create(hdr);
    lv_label_set_text(lbl, title);
    lv_obj_set_style_text_color(lbl, C_TEXT, 0);
    lv_obj_set_style_text_font(lbl, F28, 0);
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 0);

    return scr;
}

/* Create a styled button on parent at absolute position */
static lv_obj_t *makeBtn(lv_obj_t *parent, int x, int y, int w, int h,
                          const char *text, lv_color_t bgCol,
                          lv_event_cb_t cb, void *ud) {
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_style_bg_color(btn, bgCol, 0);
    lv_obj_set_style_bg_color(btn, C_PRESSED, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn, 10, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_border_width(btn, 2, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x555555), 0);
    lv_obj_set_style_pad_all(btn, 4, 0);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_color(lbl, C_TEXT, 0);
    lv_obj_set_style_text_font(lbl, F24, 0);
    lv_obj_center(lbl);

    if (cb) lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, ud);
    return btn;
}

/* Convenience: standard full-width button starting at y, returns bottom y */
static int addBtn(lv_obj_t *scr, int y, const char *text, lv_color_t col,
                  lv_event_cb_t cb, void *ud) {
    makeBtn(scr, BTN_X, y, BTN_W, BTN_H, text, col, cb, ud);
    return y + BTN_H + BTN_GAP;
}

/* ================================================================
   ASYNC NAVIGATION WRAPPERS
   All screen transitions go through lv_async_call() so the current
   LVGL event completes before the screen is replaced.
   ================================================================ */
static void _go_home(void*)           { draw_home(); }
static void _go_account(void*)        { draw_account(); }
static void _go_admin_menu(void*)     { draw_admin_menu(); }
static void _go_admin_select(void*)   { draw_admin_select(); }
static void _go_admin_action(void*)   { draw_admin_action(); }
static void _go_approvals(void*)      { draw_approvals(); }
static void _go_history(void*)        { draw_history(); }
static void _go_goals(void*)          { draw_goals(); }
static void _go_chores(void*)         { draw_chores(); }
static void _go_achievements(void*)   { draw_achievements(); }
static void _go_leaderboard(void*)    { draw_leaderboard(); }
static void _go_dad_tax(void*)        { draw_dad_tax(); }
static void _go_deposit(void*)        { draw_amount(true); }
static void _go_withdraw(void*)       { draw_amount(false); }
static void _go_pin_child(void*)      { draw_pin(false); }
static void _go_pin_admin(void*)      { draw_pin(true); }

/* ================================================================
   HOME SCREEN
   ================================================================ */
static void on_account_btn(lv_event_t *e) {
    selectedAccount = (int)(intptr_t)lv_event_get_user_data(e);
    lv_async_call(_go_pin_child, NULL);
}

static void draw_home() {
    currentScreen  = SCR_HOME;
    selectedAccount = -1;
    adminMode      = false;
    memset(enteredPin, 0, sizeof(enteredPin));

    lv_obj_t *scr = makeScreen("BANK OF DAD", C_ACCENT);

    lv_obj_t *sub = lv_label_create(scr);
    lv_label_set_text(sub, "Choose Account");
    lv_obj_set_style_text_color(sub, C_TEXT, 0);
    lv_obj_set_style_text_font(sub, F20, 0);
    lv_obj_set_pos(sub, BTN_X, HDR_H + 18);

    int y = HDR_H + 60;
    for (int i = 0; i < ACCOUNT_COUNT; i++) {
        y = addBtn(scr, y, accounts[i].name, C_PANEL,
                   on_account_btn, (void*)(intptr_t)i);
    }

    addBtn(scr, DISP_H - BTN_H - 20, "DAD ADMIN", C_BAD,
           [](lv_event_t*){ lv_async_call(_go_pin_admin, NULL); }, NULL);

    loadScreen(scr);
}

/* ================================================================
   KEYPAD HELPER  — reused by PIN and AMOUNT screens
   parent: the screen object
   yTop:   y coordinate of first key row
   cb:     called with user_data = key string ("0"-"9", "*", "#")
   ================================================================ */
static void buildKeypad(lv_obj_t *parent, int yTop, lv_event_cb_t cb) {
    static const char *keys[12] = {
        "1","2","3","4","5","6","7","8","9","*","0","#"
    };
    for (int i = 0; i < 12; i++) {
        int col = i % 3;
        int row = i / 3;
        int kx  = BTN_X + col * (KEY_W + KEY_GAP);
        int ky  = yTop  + row * (KEY_H + KEY_GAP);
        makeBtn(parent, kx, ky, KEY_W, KEY_H, keys[i], C_PANEL,
                cb, (void*)keys[i]);
    }
    lv_obj_t *hint = lv_label_create(parent);
    lv_label_set_text(hint, "* Back / Clear     # Enter");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(hint, F14, 0);
    lv_obj_set_pos(hint, BTN_X, yTop + 4 * (KEY_H + KEY_GAP) + 4);
}

/* ================================================================
   PIN SCREEN
   ================================================================ */
static lv_obj_t *s_pinLabel = nullptr;

static void refreshPinLabel() {
    if (!s_pinLabel) return;
    char buf[16] = "PIN: ";
    int  n       = strlen(enteredPin);
    for (int i = 0; i < 4; i++) buf[5 + i] = (i < n) ? '*' : '_';
    buf[9] = '\0';
    lv_label_set_text(s_pinLabel, buf);
}

static void on_pin_key(lv_event_t *e) {
    const char *key = (const char *)lv_event_get_user_data(e);
    if (!key) return;

    if (key[0] == '*') {
        lv_async_call(_go_home, NULL);
        return;
    }
    if (key[0] == '#') {
        if (strlen(enteredPin) != 4) {
            draw_message("PIN INCOMPLETE", "Enter 4 digits", "", true, MSG_RET_PIN);
            return;
        }

        bool ok = false;
        if (currentScreen == SCR_ADMIN_PIN) {
            ok = (strcmp(enteredPin, "9999") == 0);
        } else {
            ok = (selectedAccount >= 0 &&
                  strcmp(enteredPin, accounts[selectedAccount].pin) == 0);
        }
        if (ok) {
            if (currentScreen == SCR_ADMIN_PIN)
                lv_async_call(_go_admin_menu, NULL);
            else
                lv_async_call(_go_account, NULL);
        } else {
            MsgReturn ret = (currentScreen == SCR_ADMIN_PIN)
                            ? MSG_RET_ADMIN_PIN : MSG_RET_PIN;
            draw_message("WRONG PIN", "Try again", "", true, ret);
        }
        return;
    }
    if (strlen(enteredPin) < 4) {
        int n = strlen(enteredPin);
        enteredPin[n]   = key[0];
        enteredPin[n+1] = '\0';
        refreshPinLabel();
    }
}

static void draw_pin(bool isAdmin) {
    adminMode = isAdmin;
    memset(enteredPin, 0, sizeof(enteredPin));
    currentScreen = isAdmin ? SCR_ADMIN_PIN : SCR_PIN;

    lv_obj_t *scr = makeScreen(isAdmin ? "ADMIN PIN" : "ENTER PIN",
                                isAdmin ? C_BAD : C_ACCENT);

    s_pinLabel = lv_label_create(scr);
    lv_label_set_text(s_pinLabel, "PIN: _ _ _ _");
    lv_obj_set_style_text_color(s_pinLabel, C_TEXT, 0);
    lv_obj_set_style_text_font(s_pinLabel, F28, 0);
    lv_obj_align(s_pinLabel, LV_ALIGN_TOP_LEFT, BTN_X, HDR_H + 20);

    buildKeypad(scr, HDR_H + 80, on_pin_key);
    loadScreen(scr);
}

/* ================================================================
   ACCOUNT SCREEN
   ================================================================ */
static void draw_account() {
    currentScreen = SCR_ACCOUNT;

    lv_obj_t *scr = makeScreen(accounts[selectedAccount].name, C_ACCENT);

    /* Balance */
    lv_obj_t *balLbl = lv_label_create(scr);
    lv_label_set_text(balLbl, "Balance");
    lv_obj_set_style_text_color(balLbl, C_TEXT, 0);
    lv_obj_set_style_text_font(balLbl, F20, 0);
    lv_obj_set_pos(balLbl, BTN_X, HDR_H + 14);

    lv_obj_t *balVal = lv_label_create(scr);
    lv_label_set_text(balVal, moneyText(accounts[selectedAccount].balance).c_str());
    lv_obj_set_style_text_color(balVal, C_GOOD, 0);
    lv_obj_set_style_text_font(balVal, F36, 0);
    lv_obj_set_pos(balVal, BTN_X, HDR_H + 44);

    /* XP / Level */
    int level = (int)(accounts[selectedAccount].xp / 100) + 1;
    char xpBuf[48];
    snprintf(xpBuf, sizeof(xpBuf), "Level %d Saver   XP %ld",
             level, accounts[selectedAccount].xp);
    lv_obj_t *xpLbl = lv_label_create(scr);
    lv_label_set_text(xpLbl, xpBuf);
    lv_obj_set_style_text_color(xpLbl, C_CYAN, 0);
    lv_obj_set_style_text_font(xpLbl, F16, 0);
    lv_obj_set_pos(xpLbl, BTN_X, HDR_H + 106);

    /* Navigation buttons — 2×2 grid then leaderboard */
    int bw = (BTN_W - BTN_GAP) / 2;
    int y0 = HDR_H + 140;
    makeBtn(scr, BTN_X,            y0,      bw, BTN_H, "HISTORY",
            C_PANEL, [](lv_event_t*){ lv_async_call(_go_history, NULL); }, NULL);
    makeBtn(scr, BTN_X + bw + BTN_GAP, y0, bw, BTN_H, "GOALS",
            C_PANEL, [](lv_event_t*){ lv_async_call(_go_goals, NULL); }, NULL);

    int y1 = y0 + BTN_H + BTN_GAP;
    makeBtn(scr, BTN_X,            y1,      bw, BTN_H, "CHORES",
            C_PANEL, [](lv_event_t*){ lv_async_call(_go_chores, NULL); }, NULL);
    makeBtn(scr, BTN_X + bw + BTN_GAP, y1, bw, BTN_H, "BADGES",
            C_PANEL, [](lv_event_t*){ lv_async_call(_go_achievements, NULL); }, NULL);

    int y2 = y1 + BTN_H + BTN_GAP;
    addBtn(scr, y2, "LEADERBOARD", C_BLUE,
           [](lv_event_t*){ lv_async_call(_go_leaderboard, NULL); }, NULL);

    addBtn(scr, DISP_H - BTN_H - 16, "LOGOUT", C_BAD,
           [](lv_event_t*){ lv_async_call(_go_home, NULL); }, NULL);

    loadScreen(scr);
}

/* ================================================================
   HISTORY SCREEN
   ================================================================ */
static void draw_history() {
    currentScreen = SCR_HISTORY;
    lv_obj_t *scr = makeScreen("HISTORY", C_ACCENT);

    char sub[56];
    if (storageReady())
        snprintf(sub, sizeof(sub), "%s — SD transaction log",
                 accounts[selectedAccount].name);
    else
        snprintf(sub, sizeof(sub), "%s — recent activity",
                 accounts[selectedAccount].name);
    lv_obj_t *subLbl = lv_label_create(scr);
    lv_label_set_text(subLbl, sub);
    lv_obj_set_style_text_color(subLbl, lv_color_hex(0xAAAAAA), 0);
    lv_obj_set_style_text_font(subLbl, F16, 0);
    lv_obj_set_pos(subLbl, BTN_X, HDR_H + 12);

    int y = HDR_H + 50;
    bool any = false;

    if (storageReady()) {
        String sdLines[20];
        int n = storageReadHistory(accounts[selectedAccount].name, sdLines, 20);
        for (int i = 0; i < n; i++) {
            lv_obj_t *row = lv_label_create(scr);
            lv_label_set_text(row, sdLines[i].c_str());
            lv_obj_set_style_text_color(row, C_TEXT, 0);
            lv_obj_set_style_text_font(row, F20, 0);
            lv_obj_set_pos(row, BTN_X, y);
            y += 34;
            any = true;
        }
    } else {
        for (int i = 0; i < HISTORY_COUNT; i++) {
            int idx = (historyIndex[selectedAccount] + i) % HISTORY_COUNT;
            if (historyLog[selectedAccount][idx].length() > 0) {
                lv_obj_t *row = lv_label_create(scr);
                lv_label_set_text(row, historyLog[selectedAccount][idx].c_str());
                lv_obj_set_style_text_color(row, C_TEXT, 0);
                lv_obj_set_style_text_font(row, F20, 0);
                lv_obj_set_pos(row, BTN_X, y);
                y += 42;
                any = true;
            }
        }
    }

    if (!any) {
        lv_obj_t *none = lv_label_create(scr);
        lv_label_set_text(none, "No history yet");
        lv_obj_set_style_text_color(none, lv_color_hex(0x888888), 0);
        lv_obj_set_style_text_font(none, F24, 0);
        lv_obj_align(none, LV_ALIGN_CENTER, 0, 0);
    }

    addBtn(scr, DISP_H - BTN_H - 16, "BACK", C_BAD,
           [](lv_event_t*){ lv_async_call(_go_account, NULL); }, NULL);
    loadScreen(scr);
}

/* ================================================================
   GOALS SCREEN
   ================================================================ */
static void draw_goals() {
    currentScreen = SCR_GOALS;
    lv_obj_t *scr = makeScreen("SAVINGS GOAL", C_ACCENT);

    long bal     = accounts[selectedAccount].balance;
    long target  = accounts[selectedAccount].goalTarget;
    int  percent = (int)((bal * 100) / target);
    if (percent > 100) percent = 100;

    lv_obj_t *name = lv_label_create(scr);
    lv_label_set_text(name, accounts[selectedAccount].name);
    lv_obj_set_style_text_color(name, C_TEXT, 0);
    lv_obj_set_style_text_font(name, F28, 0);
    lv_obj_set_pos(name, BTN_X, HDR_H + 16);

    auto addInfo = [&](int y, const char *lbl, lv_color_t col, const char *val) {
        lv_obj_t *l = lv_label_create(scr);
        char buf[64];
        snprintf(buf, sizeof(buf), "%s%s", lbl, val);
        lv_label_set_text(l, buf);
        lv_obj_set_style_text_color(l, col, 0);
        lv_obj_set_style_text_font(l, F20, 0);
        lv_obj_set_pos(l, BTN_X, y);
    };
    addInfo(HDR_H + 80,  "Target:  ", C_TEXT, moneyText(target).c_str());
    addInfo(HDR_H + 116, "Saved:   ", C_GOOD, moneyText(bal).c_str());

    /* Progress bar */
    lv_obj_t *bar = lv_bar_create(scr);
    lv_obj_set_size(bar, BTN_W, 36);
    lv_obj_set_pos(bar, BTN_X, HDR_H + 168);
    lv_obj_set_style_bg_color(bar, C_PANEL, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, C_GOOD,  LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar, 6, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 6, LV_PART_INDICATOR);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, percent, LV_ANIM_OFF);

    char pct[8];
    snprintf(pct, sizeof(pct), "%d%%", percent);
    lv_obj_t *pctLbl = lv_label_create(scr);
    lv_label_set_text(pctLbl, pct);
    lv_obj_set_style_text_color(pctLbl, C_WARN, 0);
    lv_obj_set_style_text_font(pctLbl, F28, 0);
    lv_obj_align(pctLbl, LV_ALIGN_TOP_MID, 0, HDR_H + 220);

    addBtn(scr, DISP_H - BTN_H - 16, "BACK", C_BAD,
           [](lv_event_t*){ lv_async_call(_go_account, NULL); }, NULL);
    loadScreen(scr);
}

/* ================================================================
   CHORES SCREEN
   ================================================================ */
static void on_chore_btn(lv_event_t *e) {
    int i = (int)(intptr_t)lv_event_get_user_data(e);
    if (chores[i].pendingChild == -1) {
        chores[i].pendingChild = selectedAccount;
        addHistory(selectedAccount, "Requested " + String(chores[i].title));
        static char titleBuf[32];
        strncpy(titleBuf, chores[i].title, 31);
        lv_async_call([](void *d) {
            (void)d;
            draw_message("JOB REQUESTED", titleBuf, "Waiting Dad", false, MSG_RET_CHORES);
        }, NULL);
    } else {
        lv_async_call([](void*) {
            draw_message("NOT AVAILABLE", "Already pending", "", true, MSG_RET_CHORES);
        }, NULL);
    }
}

static void draw_chores() {
    currentScreen = SCR_CHORES;
    lv_obj_t *scr = makeScreen("CHORES", C_ACCENT);

    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "Tap a job to request it");
    lv_obj_set_style_text_color(hint, lv_color_hex(0xAAAAAA), 0);
    lv_obj_set_style_text_font(hint, F16, 0);
    lv_obj_set_pos(hint, BTN_X, HDR_H + 12);

    int y = HDR_H + 52;
    for (int i = 0; i < CHORE_COUNT; i++) {
        lv_color_t col = C_PANEL;
        String lbl = String(chores[i].title) + "  " + moneyText(chores[i].reward);
        if (chores[i].pendingChild == selectedAccount) {
            lbl = "PENDING";
            col = C_WARN;
        } else if (chores[i].pendingChild >= 0) {
            lbl = "TAKEN";
            col = C_BAD;
        }
        makeBtn(scr, BTN_X, y, BTN_W, BTN_H, lbl.c_str(), col,
                on_chore_btn, (void*)(intptr_t)i);
        y += BTN_H + BTN_GAP;
    }

    addBtn(scr, DISP_H - BTN_H - 16, "BACK", C_BAD,
           [](lv_event_t*){ lv_async_call(_go_account, NULL); }, NULL);
    loadScreen(scr);
}

/* ================================================================
   ACHIEVEMENTS / BADGES SCREEN
   ================================================================ */
static void draw_achievements() {
    currentScreen = SCR_ACHIEVEMENTS;
    updateAchievements(selectedAccount);
    lv_obj_t *scr = makeScreen("BADGES", C_ACCENT);

    struct Badge { const char *label; bool earned; };
    Badge badges[4] = {
        {"First Deposit", accounts[selectedAccount].firstDeposit},
        {"Save GBP 10",   accounts[selectedAccount].firstTen},
        {"Save GBP 25",   accounts[selectedAccount].firstTwentyFive},
        {"Level 2 Saver", accounts[selectedAccount].xp >= 100}
    };

    int y = HDR_H + 40;
    for (int i = 0; i < 4; i++) {
        lv_obj_t *row = lv_obj_create(scr);
        lv_obj_set_size(row, BTN_W, 80);
        lv_obj_set_pos(row, BTN_X, y);
        lv_obj_set_style_bg_color(row, badges[i].earned ? C_GOOD : C_PANEL, 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(row, 10, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *chk = lv_label_create(row);
        lv_label_set_text(chk, badges[i].earned ? "[X]" : "[ ]");
        lv_obj_set_style_text_font(chk, F28, 0);
        lv_obj_set_style_text_color(chk, C_TEXT, 0);
        lv_obj_align(chk, LV_ALIGN_LEFT_MID, 10, 0);

        lv_obj_t *lbl = lv_label_create(row);
        lv_label_set_text(lbl, badges[i].label);
        lv_obj_set_style_text_font(lbl, F24, 0);
        lv_obj_set_style_text_color(lbl, C_TEXT, 0);
        lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 80, 0);

        y += 80 + BTN_GAP;
    }

    addBtn(scr, DISP_H - BTN_H - 16, "BACK", C_BAD,
           [](lv_event_t*){ lv_async_call(_go_account, NULL); }, NULL);
    loadScreen(scr);
}

/* ================================================================
   LEADERBOARD SCREEN
   ================================================================ */
static void draw_leaderboard() {
    currentScreen = SCR_LEADERBOARD;
    lv_obj_t *scr = makeScreen("TOP SAVERS", C_ACCENT);

    int order[ACCOUNT_COUNT] = {0, 1, 2, 3};
    for (int i = 0; i < ACCOUNT_COUNT - 1; i++)
        for (int j = i + 1; j < ACCOUNT_COUNT; j++)
            if (accounts[order[j]].balance > accounts[order[i]].balance)
                { int t = order[i]; order[i] = order[j]; order[j] = t; }

    int y = HDR_H + 30;
    for (int i = 0; i < ACCOUNT_COUNT; i++) {
        int a = order[i];
        lv_obj_t *row = lv_obj_create(scr);
        lv_obj_set_size(row, BTN_W, 85);
        lv_obj_set_pos(row, BTN_X, y);
        lv_obj_set_style_bg_color(row, i == 0 ? C_WARN : C_PANEL, 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(row, 10, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);

        char pos[4];
        snprintf(pos, sizeof(pos), "%d.", i + 1);
        lv_obj_t *posLbl = lv_label_create(row);
        lv_label_set_text(posLbl, pos);
        lv_obj_set_style_text_font(posLbl, F28, 0);
        lv_obj_set_style_text_color(posLbl, C_TEXT, 0);
        lv_obj_align(posLbl, LV_ALIGN_LEFT_MID, 10, 0);

        lv_obj_t *nameLbl = lv_label_create(row);
        lv_label_set_text(nameLbl, accounts[a].name);
        lv_obj_set_style_text_font(nameLbl, F28, 0);
        lv_obj_set_style_text_color(nameLbl, C_TEXT, 0);
        lv_obj_align(nameLbl, LV_ALIGN_LEFT_MID, 60, 0);

        lv_obj_t *balLbl = lv_label_create(row);
        lv_label_set_text(balLbl, moneyText(accounts[a].balance).c_str());
        lv_obj_set_style_text_font(balLbl, F20, 0);
        lv_obj_set_style_text_color(balLbl, C_GOOD, 0);
        lv_obj_align(balLbl, LV_ALIGN_RIGHT_MID, -10, 0);

        y += 85 + BTN_GAP;
    }

    addBtn(scr, DISP_H - BTN_H - 16, "BACK", C_BAD,
           [](lv_event_t*){ lv_async_call(_go_account, NULL); }, NULL);
    loadScreen(scr);
}

/* ================================================================
   ADMIN MENU
   ================================================================ */
static void draw_admin_menu() {
    currentScreen = SCR_ADMIN_MENU;
    adminMode     = true;

    lv_obj_t *scr = makeScreen("DAD ADMIN", C_BAD);

    /* WiFi hint */
    char hint[80];
    snprintf(hint, sizeof(hint), "Web: %s  ->  192.168.4.1", WIFI_AP_SSID);
    lv_obj_t *wifiLbl = lv_label_create(scr);
    lv_label_set_text(wifiLbl, hint);
    lv_obj_set_style_text_color(wifiLbl, lv_color_hex(0xAAAAAA), 0);
    lv_obj_set_style_text_font(wifiLbl, F14, 0);
    lv_obj_set_pos(wifiLbl, BTN_X, HDR_H + 6);

    char sdHint[48];
    snprintf(sdHint, sizeof(sdHint), "%s%s",
             storageStatusText(),
             storageReady() ? " — history + backup on card" : "");
    lv_obj_t *sdLbl = lv_label_create(scr);
    lv_label_set_text(sdLbl, sdHint);
    lv_obj_set_style_text_color(sdLbl, storageReady() ? C_GOOD : lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(sdLbl, F14, 0);
    lv_obj_set_pos(sdLbl, BTN_X, HDR_H + 26);

    int y = HDR_H + 52;
    y = addBtn(scr, y, "EDIT MONEY",  C_GOOD, [](lv_event_t*){ lv_async_call(_go_admin_select, NULL); }, NULL);
    y = addBtn(scr, y, "APPROVALS",   C_WARN, [](lv_event_t*){ lv_async_call(_go_approvals,    NULL); }, NULL);
    y = addBtn(scr, y, "DAD TAX",     C_BAD,  [](lv_event_t*){ lv_async_call(_go_admin_select, NULL); }, (void*)1);

    addBtn(scr, DISP_H - BTN_H - 16, "HOME", C_PANEL,
           [](lv_event_t*){ lv_async_call(_go_home, NULL); }, NULL);
    loadScreen(scr);
}

/* ================================================================
   ADMIN SELECT  (choose which child)
   ================================================================ */
/* user_data encodes whether we're going to action (0) or tax (1) */
static void on_admin_select_btn(lv_event_t *e) {
    selectedAccount = (int)(intptr_t)lv_event_get_user_data(e);
    /* check if we arrived here via DAD TAX path stored in a static */
    extern bool g_taxPath;
    if (g_taxPath) lv_async_call(_go_dad_tax,     NULL);
    else           lv_async_call(_go_admin_action, NULL);
}

bool g_taxPath = false;  /* set before calling draw_admin_select */

static void _go_admin_select_tax(void *) { g_taxPath = true;  draw_admin_select(); }
static void _go_admin_select_act(void *) { g_taxPath = false; draw_admin_select(); }

static void draw_admin_select() {
    currentScreen = SCR_ADMIN_SELECT;
    lv_obj_t *scr = makeScreen("SELECT CHILD", C_BAD);

    int y = HDR_H + 40;
    for (int i = 0; i < ACCOUNT_COUNT; i++) {
        makeBtn(scr, BTN_X, y, BTN_W, BTN_H, accounts[i].name,
                C_PANEL, on_admin_select_btn, (void*)(intptr_t)i);
        y += BTN_H + BTN_GAP;
    }

    addBtn(scr, DISP_H - BTN_H - 16, "BACK", C_BAD,
           [](lv_event_t*){ lv_async_call(_go_admin_menu, NULL); }, NULL);
    loadScreen(scr);
}

/* ================================================================
   ADMIN ACTION  (deposit / withdraw / dad tax for selected child)
   ================================================================ */
static void draw_admin_action() {
    currentScreen = SCR_ADMIN_ACTION;
    g_taxPath     = false;

    lv_obj_t *scr = makeScreen(accounts[selectedAccount].name, C_BAD);

    lv_obj_t *bal = lv_label_create(scr);
    lv_label_set_text(bal, moneyText(accounts[selectedAccount].balance).c_str());
    lv_obj_set_style_text_color(bal, C_GOOD, 0);
    lv_obj_set_style_text_font(bal, F28, 0);
    lv_obj_set_pos(bal, BTN_X, HDR_H + 16);

    int y = HDR_H + 80;
    y = addBtn(scr, y, "DEPOSIT",  C_GOOD, [](lv_event_t*){ lv_async_call(_go_deposit,  NULL); }, NULL);
    y = addBtn(scr, y, "WITHDRAW", C_BAD,  [](lv_event_t*){ lv_async_call(_go_withdraw, NULL); }, NULL);
    y = addBtn(scr, y, "DAD TAX",  C_WARN, [](lv_event_t*){ lv_async_call(_go_dad_tax,  NULL); }, NULL);

    addBtn(scr, DISP_H - BTN_H - 16, "BACK", C_PANEL,
           [](lv_event_t*){ lv_async_call(_go_admin_select_act, NULL); }, NULL);
    loadScreen(scr);
}

/* ================================================================
   APPROVALS SCREEN
   ================================================================ */
static void on_approve_btn(lv_event_t *e) {
    int i = (int)(intptr_t)lv_event_get_user_data(e);
    int child  = chores[i].pendingChild;
    long reward = chores[i].reward;

    accounts[child].balance  += reward;
    accounts[child].xp       += 25;
    accounts[child].firstDeposit = true;
    addHistory(child, "+" + moneyText(reward) + " " + String(chores[i].title));
    chores[i].pendingChild = -1;
    updateAchievements(child);
    saveAccount(child);

    static char nameBuf[16];
    strncpy(nameBuf, accounts[child].name, 15);
    static String rewardStr;
    rewardStr = "+" + moneyText(reward);

    lv_async_call([](void*) {
        draw_message("APPROVED", nameBuf, rewardStr.c_str(), false, MSG_RET_ADMIN_MENU);
    }, NULL);
}

static void draw_approvals() {
    currentScreen = SCR_APPROVALS;
    lv_obj_t *scr = makeScreen("APPROVALS", C_BAD);

    bool found = false;
    int  y     = HDR_H + 40;

    for (int i = 0; i < CHORE_COUNT; i++) {
        if (chores[i].pendingChild >= 0) {
            found = true;
            String lbl = String(accounts[chores[i].pendingChild].name) +
                         "  " + moneyText(chores[i].reward);
            makeBtn(scr, BTN_X, y, BTN_W, BTN_H, lbl.c_str(), C_GOOD,
                    on_approve_btn, (void*)(intptr_t)i);
            y += BTN_H + BTN_GAP;
        }
    }
    if (!found) {
        lv_obj_t *none = lv_label_create(scr);
        lv_label_set_text(none, "No pending approvals");
        lv_obj_set_style_text_color(none, lv_color_hex(0x888888), 0);
        lv_obj_set_style_text_font(none, F24, 0);
        lv_obj_align(none, LV_ALIGN_CENTER, 0, 0);
    }

    addBtn(scr, DISP_H - BTN_H - 16, "BACK", C_BAD,
           [](lv_event_t*){ lv_async_call(_go_admin_menu, NULL); }, NULL);
    loadScreen(scr);
}

/* ================================================================
   DAD TAX SCREEN
   ================================================================ */
static void draw_dad_tax() {
    currentScreen = SCR_DAD_TAX;
    long tax = (accounts[selectedAccount].balance * DAD_TAX_PERCENT) / 100;

    lv_obj_t *scr = makeScreen("DAD TAX", C_BAD);

    lv_obj_t *name = lv_label_create(scr);
    lv_label_set_text(name, accounts[selectedAccount].name);
    lv_obj_set_style_text_color(name, C_TEXT, 0);
    lv_obj_set_style_text_font(name, F28, 0);
    lv_obj_set_pos(name, BTN_X, HDR_H + 16);

    auto addRow = [&](int y, const char *lbl, lv_color_t col, const char *val) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s%s", lbl, val);
        lv_obj_t *l = lv_label_create(scr);
        lv_label_set_text(l, buf);
        lv_obj_set_style_text_color(l, col, 0);
        lv_obj_set_style_text_font(l, F20, 0);
        lv_obj_set_pos(l, BTN_X, y);
    };
    addRow(HDR_H + 80,  "Balance:  ", C_TEXT, moneyText(accounts[selectedAccount].balance).c_str());
    addRow(HDR_H + 120, "Tax (10%): -", C_BAD,  moneyText(tax).c_str());
    addRow(HDR_H + 160, "After:    ", C_GOOD, moneyText(accounts[selectedAccount].balance - tax).c_str());

    if (tax == 0) {
        lv_obj_t *zero = lv_label_create(scr);
        lv_label_set_text(zero, "Nothing to tax!");
        lv_obj_set_style_text_color(zero, C_WARN, 0);
        lv_obj_set_style_text_font(zero, F24, 0);
        lv_obj_set_pos(zero, BTN_X, HDR_H + 220);
        addBtn(scr, DISP_H - BTN_H - 16, "BACK", C_PANEL,
               [](lv_event_t*){ lv_async_call(_go_admin_action, NULL); }, NULL);
    } else {
        int y = HDR_H + 240;
        addBtn(scr, y, "APPLY TAX", C_BAD,
            [](lv_event_t*) {
                long t = (accounts[selectedAccount].balance * DAD_TAX_PERCENT) / 100;
                accounts[selectedAccount].balance -= t;
                addHistory(selectedAccount, "-" + moneyText(t) + " Dad Tax");
                updateAchievements(selectedAccount);
                saveAccount(selectedAccount);
                static String msg;
                msg = "-" + moneyText(t);
                lv_async_call([](void*) {
                    draw_message("TAX APPLIED", msg.c_str(),
                                 moneyText(accounts[selectedAccount].balance).c_str(),
                                 false, MSG_RET_ADMIN_MENU);
                }, NULL);
            }, NULL);
        y += BTN_H + BTN_GAP;
        addBtn(scr, y, "CANCEL", C_PANEL,
               [](lv_event_t*){ lv_async_call(_go_admin_action, NULL); }, NULL);
    }

    loadScreen(scr);
}

/* ================================================================
   AMOUNT SCREEN  (deposit / withdraw keypad)
   ================================================================ */
static lv_obj_t *s_amountLabel = nullptr;

static void refreshAmountLabel() {
    if (!s_amountLabel) return;
    long p = atol(amountInput);
    String s = "Amount  " + moneyText(p);
    lv_label_set_text(s_amountLabel, s.c_str());
}

static void on_amount_key(lv_event_t *e) {
    const char *key = (const char *)lv_event_get_user_data(e);
    if (!key) return;

    if (key[0] == '*') {
        lv_async_call(_go_admin_action, NULL);
        return;
    }
    if (key[0] == '#') {
        long pennies = atol(amountInput);
        if (pennies <= 0) {
            lv_async_call([](void*){ draw_message("NO AMOUNT","Enter value","",true, MSG_RET_ADMIN_ACTION); }, NULL);
            return;
        }
        if (!depositMode && accounts[selectedAccount].balance < pennies) {
            lv_async_call([](void*){ draw_message("NO FUNDS","Not enough","",true, MSG_RET_ADMIN_ACTION); }, NULL);
            return;
        }
        if (depositMode) {
            accounts[selectedAccount].balance += pennies;
            accounts[selectedAccount].xp      += 10;
            accounts[selectedAccount].firstDeposit = true;
            addHistory(selectedAccount, "+" + moneyText(pennies) + " Admin");
        } else {
            accounts[selectedAccount].balance -= pennies;
            addHistory(selectedAccount, "-" + moneyText(pennies) + " Admin");
        }
        updateAchievements(selectedAccount);
        saveAccount(selectedAccount);

        static bool  wasDeposit;
        static String l1, l2;
        wasDeposit = depositMode;
        l1 = (depositMode ? "+" : "-") + moneyText(pennies);
        l2 = "Bal " + moneyText(accounts[selectedAccount].balance);
        lv_async_call([](void*) {
            draw_message(wasDeposit ? "DEPOSIT OK" : "WITHDRAW OK",
                         l1.c_str(), l2.c_str(), false, MSG_RET_ADMIN_MENU);
        }, NULL);
        return;
    }

    int n = strlen(amountInput);
    if (n < 6) {
        amountInput[n]   = key[0];
        amountInput[n+1] = '\0';
        refreshAmountLabel();
    }
}

static void draw_amount(bool isDeposit) {
    currentScreen = SCR_AMOUNT;
    depositMode   = isDeposit;
    memset(amountInput, 0, sizeof(amountInput));

    lv_obj_t *scr = makeScreen(isDeposit ? "DEPOSIT" : "WITHDRAW",
                                isDeposit ? C_GOOD : C_BAD);

    s_amountLabel = lv_label_create(scr);
    lv_label_set_text(s_amountLabel, "Amount  GBP 0.00");
    lv_obj_set_style_text_color(s_amountLabel, C_TEXT, 0);
    lv_obj_set_style_text_font(s_amountLabel, F28, 0);
    lv_obj_set_pos(s_amountLabel, BTN_X, HDR_H + 18);

    lv_obj_t *hint2 = lv_label_create(scr);
    lv_label_set_text(hint2, "Enter pennies  (e.g. 250 = GBP 2.50)");
    lv_obj_set_style_text_color(hint2, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(hint2, F14, 0);
    lv_obj_set_pos(hint2, BTN_X, HDR_H + 60);

    buildKeypad(scr, HDR_H + 90, on_amount_key);
    loadScreen(scr);
}

/* ================================================================
   MESSAGE SCREEN
   ================================================================ */
static MsgReturn s_msgReturn = MSG_RET_HOME;

static void on_message_ok(lv_event_t *e) {
    (void)e;
    switch (s_msgReturn) {
        case MSG_RET_PIN:          lv_async_call(_go_pin_child,   NULL); break;
        case MSG_RET_ADMIN_PIN:    lv_async_call(_go_pin_admin,   NULL); break;
        case MSG_RET_ACCOUNT:      lv_async_call(_go_account,     NULL); break;
        case MSG_RET_CHORES:       lv_async_call(_go_chores,      NULL); break;
        case MSG_RET_ADMIN_MENU:   lv_async_call(_go_admin_menu,  NULL); break;
        case MSG_RET_ADMIN_ACTION: lv_async_call(_go_admin_action,NULL); break;
        default:                   lv_async_call(_go_home,        NULL); break;
    }
}

static void draw_message(const char *title, const char *line1,
                         const char *line2, bool isError, MsgReturn ret) {
    currentScreen = SCR_MESSAGE;
    s_msgReturn   = ret;
    lv_color_t col = isError ? C_BAD : C_GOOD;
    lv_obj_t *scr = makeScreen(isError ? "ERROR" : "COMPLETE", col);

    lv_obj_t *t = lv_label_create(scr);
    lv_label_set_text(t, title);
    lv_obj_set_style_text_color(t, col, 0);
    lv_obj_set_style_text_font(t, F28, 0);
    lv_obj_set_pos(t, BTN_X, HDR_H + 40);

    lv_obj_t *l1 = lv_label_create(scr);
    lv_label_set_text(l1, line1);
    lv_obj_set_style_text_color(l1, C_TEXT, 0);
    lv_obj_set_style_text_font(l1, F24, 0);
    lv_obj_set_pos(l1, BTN_X, HDR_H + 110);

    if (line2 && line2[0]) {
        lv_obj_t *l2 = lv_label_create(scr);
        lv_label_set_text(l2, line2);
        lv_obj_set_style_text_color(l2, C_TEXT, 0);
        lv_obj_set_style_text_font(l2, F24, 0);
        lv_obj_set_pos(l2, BTN_X, HDR_H + 162);
    }

    addBtn(scr, DISP_H - BTN_H - 16, "OK", col, on_message_ok, NULL);

    loadScreen(scr);
}

/* ================================================================
   appSetup / appLoop  (called from .ino)
   ================================================================ */
void appSetup() {
    /* NVS: detect first boot and seed history */
    bool fresh = !prefs.begin("bank", true);
    prefs.end();
    loadAccounts();
    loadChores();

    storageInit();

    if (fresh && storageReady()) {
        StorageAccountSnap snap[ACCOUNT_COUNT];
        for (int i = 0; i < ACCOUNT_COUNT; i++) {
            snap[i] = {accounts[i].name, accounts[i].balance, accounts[i].xp,
                       accounts[i].firstDeposit, accounts[i].firstTen,
                       accounts[i].firstTwentyFive};
        }
        if (storageRestoreAccounts(snap, ACCOUNT_COUNT)) {
            for (int i = 0; i < ACCOUNT_COUNT; i++) {
                accounts[i].balance         = snap[i].balance;
                accounts[i].xp              = snap[i].xp;
                accounts[i].firstDeposit    = snap[i].firstDeposit;
                accounts[i].firstTen        = snap[i].firstTen;
                accounts[i].firstTwentyFive = snap[i].firstTwentyFive;
                saveAccount(i);
            }
            Serial.println("Restored account data from SD backup");
        }
    }

    if (fresh) {
        for (int i = 0; i < ACCOUNT_COUNT; i++)
            addHistory(i, "Account opened");
    }

    /* WiFi AP + web chore editor */
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
    Serial.printf("AP: %s  IP: %s\n", WIFI_AP_SSID,
                  WiFi.softAPIP().toString().c_str());
    setupWebServer();
    webServer.begin();
    Serial.println("Chore editor: http://192.168.4.1");

    Serial.println("Admin PIN: 9999");
    Serial.println("Child PINs: Nixon 1111, Oakley 2222, Hadley 3333, Maddison 4444");

    draw_home();
}

void appLoop() {
    lv_timer_handler();      /* drive LVGL rendering & animations */
    webServer.handleClient(); /* serve web chore editor */
}

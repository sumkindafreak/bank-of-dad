#pragma once

#define LV_CONF_INCLUDE_SIMPLE
#include <lvgl.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>

/* ================================================================
   Bank of Dad — App header
   All data structures, constants, and forward declarations.
   ================================================================ */

/* ---- WiFi AP ---- */
#define WIFI_AP_SSID        "BankOfDad"
#define WIFI_AP_PASS        "pocket123"

/* ---- Data limits ---- */
#define ACCOUNT_COUNT       4
#define HISTORY_COUNT       6
#define CHORE_COUNT         4
#define DAD_TAX_PERCENT     10

/* ---- Account ---- */
struct Account {
    const char *name;
    const char *pin;
    long        balance;     /* stored in pennies */
    long        goalTarget;
    long        xp;
    bool        firstDeposit;
    bool        firstTen;
    bool        firstTwentyFive;
};

/* ---- Chore ---- */
struct Chore {
    char title[32];   /* mutable — web editor can update */
    long reward;      /* pennies */
    int  pendingChild;/* -1 = available, ≥0 = index of requesting child */
};

/* ---- Screens ---- */
enum Screen {
    SCR_HOME,
    SCR_PIN,
    SCR_ACCOUNT,
    SCR_HISTORY,
    SCR_GOALS,
    SCR_CHORES,
    SCR_ACHIEVEMENTS,
    SCR_LEADERBOARD,
    SCR_ADMIN_PIN,
    SCR_ADMIN_MENU,
    SCR_ADMIN_SELECT,
    SCR_ADMIN_ACTION,
    SCR_AMOUNT,
    SCR_APPROVALS,
    SCR_DAD_TAX,
    SCR_MESSAGE
};

/* Where the MESSAGE screen OK button navigates */
enum MsgReturn {
    MSG_RET_HOME,
    MSG_RET_PIN,
    MSG_RET_ADMIN_PIN,
    MSG_RET_ACCOUNT,
    MSG_RET_CHORES,
    MSG_RET_ADMIN_MENU,
    MSG_RET_ADMIN_ACTION
};

/* ---- Shared globals (defined in app.cpp) ---- */
extern Account   accounts[ACCOUNT_COUNT];
extern Chore     chores[CHORE_COUNT];
extern String    historyLog[ACCOUNT_COUNT][HISTORY_COUNT];
extern int       historyIndex[ACCOUNT_COUNT];
extern Screen    currentScreen;
extern int       selectedAccount;
extern bool      adminMode;
extern bool      depositMode;
extern char      enteredPin[5];
extern char      amountInput[7];
extern Preferences prefs;
extern WebServer   webServer;

/* ---- Entry points (called from .ino) ---- */
void appSetup();
void appLoop();

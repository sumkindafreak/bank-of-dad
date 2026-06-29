#include "model.h"
#include "storage.h"
#include <string.h>

BankState    g_bank;
Preferences  g_prefs;

static const char *kAchNames[ACH_COUNT] = {
    "First Chore", "10 Chores", "100 Chores",
    "Save GBP10", "Save GBP50", "Save GBP100",
    "7 Day Streak", "30 Day Streak",
    "Kitchen King", "Laundry Legend", "Garden Hero",
    "Super Saver", "Family Helper", "Shop Spender",
    "Goal Reached", "Daily Mission"
};

static const char *kCatNames[CHORE_CAT_COUNT] = {
    "Bedroom", "Kitchen", "Bathroom", "Laundry", "Garden", "Outdoor", "General"
};

static void seedDefaultChores() {
    static const struct { const char *t; long p; uint16_t xp; ChoreCategory c; } defs[] = {
        {"Make bed",           50,  8,  CHORE_CAT_BEDROOM},
        {"Laundry basket",     75, 10,  CHORE_CAT_BEDROOM},
        {"Tidy bedroom",      100, 12,  CHORE_CAT_BEDROOM},
        {"Empty bin",          50,  8,  CHORE_CAT_GENERAL},
        {"Feed pets",         100, 12,  CHORE_CAT_GENERAL},
        {"Water plants",       75, 10,  CHORE_CAT_GARDEN},
        {"Set table",          50,  8,  CHORE_CAT_KITCHEN},
        {"Clear table",        50,  8,  CHORE_CAT_KITCHEN},
        {"Kitchen wipe",      100, 12,  CHORE_CAT_KITCHEN},
        {"Vacuum room",       150, 15,  CHORE_CAT_BEDROOM},
        {"Sweep floor",       100, 12,  CHORE_CAT_GENERAL},
        {"Dust downstairs",   200, 18,  CHORE_CAT_GENERAL},
        {"Unload dishwasher",  75, 10,  CHORE_CAT_KITCHEN},
        {"Load dishwasher",    75, 10,  CHORE_CAT_KITCHEN},
        {"Fold washing",      150, 15,  CHORE_CAT_LAUNDRY},
        {"Hang washing",      150, 15,  CHORE_CAT_LAUNDRY},
        {"Bring washing in",  100, 12,  CHORE_CAT_LAUNDRY},
        {"Bathroom sink",     100, 12,  CHORE_CAT_BATHROOM},
        {"Mop kitchen",       200, 18,  CHORE_CAT_KITCHEN},
        {"Hoover downstairs", 250, 20,  CHORE_CAT_GENERAL},
        {"Clean toilet",      150, 15,  CHORE_CAT_BATHROOM},
        {"Help cook dinner",  200, 18,  CHORE_CAT_KITCHEN},
        {"Wash car",          350, 25,  CHORE_CAT_OUTDOOR},
        {"Mow lawn",          400, 28,  CHORE_CAT_GARDEN},
        {"Clean windows",     300, 22,  CHORE_CAT_OUTDOOR},
        {"Deep clean bathroom", 450, 30, CHORE_CAT_BATHROOM},
        {"Deep clean kitchen",  450, 30, CHORE_CAT_KITCHEN},
    };
    const int n = (int)(sizeof(defs) / sizeof(defs[0]));
    g_bank.choreCount = (uint8_t)n;
    for (int i = 0; i < n; i++) {
        strncpy(g_bank.chores[i].title, defs[i].t, 31);
        g_bank.chores[i].title[31] = '\0';
        g_bank.chores[i].rewardPence = defs[i].p;
        if (g_bank.chores[i].rewardPence > BANK_MAX_REWARD_PENCE)
            g_bank.chores[i].rewardPence = BANK_MAX_REWARD_PENCE;
        g_bank.chores[i].xpReward = defs[i].xp;
        g_bank.chores[i].category = defs[i].c;
        g_bank.chores[i].completionCount = 0;
        g_bank.chores[i].enabled = true;
        g_bank.chores[i].pendingChild = -1;
    }
    for (int i = n; i < BANK_CHORE_SLOTS; i++) {
        g_bank.chores[i].title[0] = '\0';
        g_bank.chores[i].rewardPence = 0;
        g_bank.chores[i].xpReward = 10;
        g_bank.chores[i].category = CHORE_CAT_GENERAL;
        g_bank.chores[i].completionCount = 0;
        g_bank.chores[i].enabled = false;
        g_bank.chores[i].pendingChild = -1;
    }
}

static void seedDefaultShop() {
    static const struct { const char *t; long p; } defs[] = {
        {"Chocolate", 150}, {"Movie night", 300}, {"Late bedtime", 200},
        {"McDonald's", 500}, {"Cinema", 800}, {"Small toy", 600},
        {"Video game time", 400}, {"Day trip", 1200},
        {"Theme park", 2500}, {"Extra screen time", 250},
        {"Ice cream", 100}, {"Choose dinner", 350}
    };
    const int n = (int)(sizeof(defs) / sizeof(defs[0]));
    g_bank.shopCount = (uint8_t)n;
    for (int i = 0; i < n; i++) {
        strncpy(g_bank.shop[i].title, defs[i].t, 23);
        g_bank.shop[i].title[23] = '\0';
        g_bank.shop[i].pricePence = defs[i].p;
        g_bank.shop[i].enabled = true;
    }
}

static void seedDefaultAccounts() {
    static const struct { const char *n; const char *p; const char *a; long bal; long sav; long goal; long xp; } defs[] = {
        {"Nixon",    "1111", ":)", 1250, 200, 5000,  40},
        {"Oakley",   "2222", ":D",  800, 100, 3000,  20},
        {"Hadley",   "3333", ";)", 1575, 300, 4500,  65},
        {"Maddison", "4444", "B)", 2200, 500, 6000, 120}
    };
    g_bank.accountCount = BANK_ACCOUNT_COUNT;
    for (int i = 0; i < BANK_ACCOUNT_COUNT; i++) {
        strncpy(g_bank.accounts[i].name, defs[i].n, 15);
        strncpy(g_bank.accounts[i].pin, defs[i].p, 4);
        strncpy(g_bank.accounts[i].avatar, defs[i].a, 3);
        strncpy(g_bank.accounts[i].goalName, "Savings goal", 23);
        g_bank.accounts[i].balance = defs[i].bal;
        g_bank.accounts[i].savingsBalance = defs[i].sav;
        g_bank.accounts[i].goalTarget = defs[i].goal;
        g_bank.accounts[i].xp = defs[i].xp;
        g_bank.accounts[i].active = true;
        g_bank.accounts[i].frozen = false;
        g_bank.accounts[i].firstDeposit = false;
    }
    for (int i = BANK_ACCOUNT_COUNT; i < BANK_MAX_ACCOUNTS; i++)
        g_bank.accounts[i].active = false;
}

const char *modelCategoryName(ChoreCategory cat) {
    if ((uint8_t)cat < CHORE_CAT_COUNT) return kCatNames[cat];
    return "General";
}

const char *modelAchievementName(AchievementId id) {
    if ((uint8_t)id < ACH_COUNT) return kAchNames[id];
    return "Achievement";
}

String modelMoneyText(long pennies) {
    long pounds = pennies / 100;
    int pence = (int)labs(pennies % 100);
    char buf[20];
    snprintf(buf, sizeof(buf), "GBP %ld.%02d", pounds, pence);
    return String(buf);
}

int modelLevelFromXp(long xp) {
    return (int)(xp / 100) + 1;
}

uint32_t modelTodayStamp() {
    return (uint32_t)(millis() / 86400000UL);
}

void modelInitDefaults() {
    memset(&g_bank, 0, sizeof(g_bank));
    seedDefaultAccounts();
    seedDefaultChores();
    seedDefaultShop();
    g_bank.config.storageVersion = BANK_STORAGE_VERSION;
    g_bank.config.dadTaxPercent = 10;
    g_bank.config.dadTaxEnabled = true;
    g_bank.config.interestRatePercent = 5;
    g_bank.config.interestEnabled = true;
    g_bank.config.weeklyChallengeType = 1;
    g_bank.config.dayStamp = modelTodayStamp();
    g_bank.currentScreen = SCR_HOME;
    g_bank.selectedAccount = -1;
}

static void storageBackupAll() {
    StorageAccountSnap snap[BANK_MAX_ACCOUNTS];
    int n = 0;
    for (int i = 0; i < BANK_MAX_ACCOUNTS; i++) {
        if (!g_bank.accounts[i].active) continue;
        snap[n++] = {g_bank.accounts[i].name, g_bank.accounts[i].balance, g_bank.accounts[i].xp,
                     g_bank.accounts[i].firstDeposit, modelHasAchievement(i, ACH_SAVED_10),
                     modelHasAchievement(i, ACH_SAVED_100)};
    }
    if (n > 0) storageBackupAccounts(snap, n);
}

static void migrateFromV1() {
    g_prefs.begin("bank", true);
    for (int i = 0; i < 4; i++) {
        String k = String(i);
        if (g_prefs.isKey(("bal" + k).c_str())) {
            g_bank.accounts[i].balance = g_prefs.getLong(("bal" + k).c_str(), g_bank.accounts[i].balance);
            g_bank.accounts[i].xp = g_prefs.getLong(("xp" + k).c_str(), g_bank.accounts[i].xp);
            g_bank.accounts[i].firstDeposit = g_prefs.getBool(("fd" + k).c_str(), false);
            if (g_prefs.getBool(("f10" + k).c_str(), false)) modelUnlockAchievement(i, ACH_SAVED_10);
            if (g_prefs.getBool(("f25" + k).c_str(), false)) modelUnlockAchievement(i, ACH_SAVED_50);
        }
        String title = g_prefs.getString(("ct" + k).c_str(), "");
        if (title.length() > 0 && i < (int)g_bank.choreCount) {
            title.toCharArray(g_bank.chores[i].title, 32);
            g_bank.chores[i].rewardPence = g_prefs.getLong(("cr" + k).c_str(), g_bank.chores[i].rewardPence);
            g_bank.chores[i].enabled = true;
        }
    }
    g_prefs.end();
    g_bank.config.storageVersion = BANK_STORAGE_VERSION;
    modelSaveAll();
}

bool modelLoadAll() {
    modelInitDefaults();
    g_prefs.begin("bank", true);
    uint16_t ver = (uint16_t)g_prefs.getUInt("ver", 0);
    g_prefs.end();
    if (ver == 0) {
        migrateFromV1();
        return true;
    }
    if (ver != BANK_STORAGE_VERSION) {
        g_bank.config.storageVersion = BANK_STORAGE_VERSION;
    }

    g_prefs.begin("bank", true);
    g_bank.accountCount = (uint8_t)g_prefs.getUChar("acnt", BANK_ACCOUNT_COUNT);
    for (int i = 0; i < BANK_MAX_ACCOUNTS; i++) {
        String k = String(i);
        String name = g_prefs.getString(("nm" + k).c_str(), g_bank.accounts[i].name);
        name.toCharArray(g_bank.accounts[i].name, 16);
        String av = g_prefs.getString(("av" + k).c_str(), g_bank.accounts[i].avatar);
        av.toCharArray(g_bank.accounts[i].avatar, 4);
        String pin = g_prefs.getString(("pn" + k).c_str(), g_bank.accounts[i].pin);
        pin.toCharArray(g_bank.accounts[i].pin, 5);
        g_bank.accounts[i].balance = g_prefs.getLong(("bal" + k).c_str(), g_bank.accounts[i].balance);
        g_bank.accounts[i].savingsBalance = g_prefs.getLong(("sav" + k).c_str(), g_bank.accounts[i].savingsBalance);
        g_bank.accounts[i].goalTarget = g_prefs.getLong(("gt" + k).c_str(), g_bank.accounts[i].goalTarget);
        String gn = g_prefs.getString(("gn" + k).c_str(), g_bank.accounts[i].goalName);
        gn.toCharArray(g_bank.accounts[i].goalName, 24);
        g_bank.accounts[i].xp = g_prefs.getLong(("xp" + k).c_str(), g_bank.accounts[i].xp);
        g_bank.accounts[i].choresCompleted = (uint16_t)g_prefs.getUShort(("cc" + k).c_str(), 0);
        g_bank.accounts[i].currentStreak = (uint16_t)g_prefs.getUShort(("cs" + k).c_str(), 0);
        g_bank.accounts[i].longestStreak = (uint16_t)g_prefs.getUShort(("ls" + k).c_str(), 0);
        g_bank.accounts[i].weeklyEarned = g_prefs.getULong(("we" + k).c_str(), 0);
        g_bank.accounts[i].monthlyEarned = g_prefs.getULong(("me" + k).c_str(), 0);
        g_bank.accounts[i].lifetimeEarned = g_prefs.getULong(("le" + k).c_str(), 0);
        g_bank.accounts[i].moneySpent = g_prefs.getULong(("ms" + k).c_str(), 0);
        g_bank.accounts[i].achievementMask = g_prefs.getULong(("am" + k).c_str(), 0);
        g_bank.accounts[i].frozen = g_prefs.getBool(("fr" + k).c_str(), false);
        g_bank.accounts[i].active = g_prefs.getBool(("on" + k).c_str(), i < BANK_ACCOUNT_COUNT);
        g_bank.accounts[i].firstDeposit = g_prefs.getBool(("fd" + k).c_str(), false);
    }

    g_bank.choreCount = (uint8_t)g_prefs.getUChar("chcnt", g_bank.choreCount);
    for (int i = 0; i < BANK_CHORE_SLOTS; i++) {
        String k = String(i);
        String title = g_prefs.getString(("ct" + k).c_str(), g_bank.chores[i].title);
        title.toCharArray(g_bank.chores[i].title, 32);
        g_bank.chores[i].rewardPence = g_prefs.getLong(("cr" + k).c_str(), g_bank.chores[i].rewardPence);
        g_bank.chores[i].xpReward = (uint16_t)g_prefs.getUShort(("cx" + k).c_str(), g_bank.chores[i].xpReward);
        g_bank.chores[i].category = (ChoreCategory)g_prefs.getUChar(("cca" + k).c_str(), g_bank.chores[i].category);
        g_bank.chores[i].completionCount = (uint16_t)g_prefs.getUShort(("ccc" + k).c_str(), 0);
        g_bank.chores[i].enabled = g_prefs.getBool(("cen" + k).c_str(), g_bank.chores[i].enabled);
        g_bank.chores[i].pendingChild = (int8_t)g_prefs.getChar(("cpd" + k).c_str(), -1);
    }

    g_bank.shopCount = (uint8_t)g_prefs.getUChar("shcnt", g_bank.shopCount);
    for (int i = 0; i < BANK_SHOP_SLOTS; i++) {
        String k = String(i);
        String title = g_prefs.getString(("st" + k).c_str(), g_bank.shop[i].title);
        title.toCharArray(g_bank.shop[i].title, 24);
        g_bank.shop[i].pricePence = g_prefs.getLong(("sp" + k).c_str(), g_bank.shop[i].pricePence);
        g_bank.shop[i].enabled = g_prefs.getBool(("sen" + k).c_str(), g_bank.shop[i].enabled);
    }

    g_bank.config.dadTaxPercent = (uint8_t)g_prefs.getUChar("tax", 10);
    g_bank.config.dadTaxEnabled = g_prefs.getBool("taxOn", true);
    g_bank.config.interestRatePercent = (uint8_t)g_prefs.getUChar("int", 5);
    g_bank.config.interestEnabled = g_prefs.getBool("intOn", true);
    g_bank.config.totalDadTaxCollected = g_prefs.getULong("taxTot", 0);
    g_bank.config.totalInterestPaid = g_prefs.getULong("intTot", 0);
    g_bank.config.weeklyChallengeProgress = (uint8_t)g_prefs.getUChar("wcp", 0);
    g_bank.config.weeklyChallengeClaimed = g_prefs.getBool("wcc", false);
    g_bank.config.dailyMissionChore = (uint8_t)g_prefs.getUChar("dmc", 0);
    g_bank.config.dailyMissionDone = g_prefs.getBool("dmd", false);
    g_bank.config.dayStamp = g_prefs.getULong("day", modelTodayStamp());
    g_prefs.end();
    return false;
}

void modelSaveConfig() {
    g_prefs.begin("bank", false);
    g_prefs.putUInt("ver", BANK_STORAGE_VERSION);
    g_prefs.putUChar("tax", g_bank.config.dadTaxPercent);
    g_prefs.putBool("taxOn", g_bank.config.dadTaxEnabled);
    g_prefs.putUChar("int", g_bank.config.interestRatePercent);
    g_prefs.putBool("intOn", g_bank.config.interestEnabled);
    g_prefs.putULong("taxTot", g_bank.config.totalDadTaxCollected);
    g_prefs.putULong("intTot", g_bank.config.totalInterestPaid);
    g_prefs.putUChar("wcp", g_bank.config.weeklyChallengeProgress);
    g_prefs.putBool("wcc", g_bank.config.weeklyChallengeClaimed);
    g_prefs.putUChar("dmc", g_bank.config.dailyMissionChore);
    g_prefs.putBool("dmd", g_bank.config.dailyMissionDone);
    g_prefs.putULong("day", g_bank.config.dayStamp);
    g_prefs.end();
}

void modelSaveAccount(int index) {
    if (index < 0 || index >= BANK_MAX_ACCOUNTS) return;
    g_prefs.begin("bank", false);
    String k = String(index);
    g_prefs.putString(("nm" + k).c_str(), g_bank.accounts[index].name);
    g_prefs.putString(("av" + k).c_str(), g_bank.accounts[index].avatar);
    g_prefs.putString(("pn" + k).c_str(), g_bank.accounts[index].pin);
    g_prefs.putLong(("bal" + k).c_str(), g_bank.accounts[index].balance);
    g_prefs.putLong(("sav" + k).c_str(), g_bank.accounts[index].savingsBalance);
    g_prefs.putLong(("gt" + k).c_str(), g_bank.accounts[index].goalTarget);
    g_prefs.putString(("gn" + k).c_str(), g_bank.accounts[index].goalName);
    g_prefs.putLong(("xp" + k).c_str(), g_bank.accounts[index].xp);
    g_prefs.putUShort(("cc" + k).c_str(), g_bank.accounts[index].choresCompleted);
    g_prefs.putUShort(("cs" + k).c_str(), g_bank.accounts[index].currentStreak);
    g_prefs.putUShort(("ls" + k).c_str(), g_bank.accounts[index].longestStreak);
    g_prefs.putULong(("we" + k).c_str(), g_bank.accounts[index].weeklyEarned);
    g_prefs.putULong(("me" + k).c_str(), g_bank.accounts[index].monthlyEarned);
    g_prefs.putULong(("le" + k).c_str(), g_bank.accounts[index].lifetimeEarned);
    g_prefs.putULong(("ms" + k).c_str(), g_bank.accounts[index].moneySpent);
    g_prefs.putULong(("am" + k).c_str(), g_bank.accounts[index].achievementMask);
    g_prefs.putBool(("fr" + k).c_str(), g_bank.accounts[index].frozen);
    g_prefs.putBool(("on" + k).c_str(), g_bank.accounts[index].active);
    g_prefs.putBool(("fd" + k).c_str(), g_bank.accounts[index].firstDeposit);
    g_prefs.putUChar("acnt", g_bank.accountCount);
    g_prefs.end();
    storageBackupAll();
}

void modelSaveChores() {
    g_prefs.begin("bank", false);
    g_prefs.putUChar("chcnt", g_bank.choreCount);
    for (int i = 0; i < BANK_CHORE_SLOTS; i++) {
        String k = String(i);
        g_prefs.putString(("ct" + k).c_str(), g_bank.chores[i].title);
        g_prefs.putLong(("cr" + k).c_str(), g_bank.chores[i].rewardPence);
        g_prefs.putUShort(("cx" + k).c_str(), g_bank.chores[i].xpReward);
        g_prefs.putUChar(("cca" + k).c_str(), (uint8_t)g_bank.chores[i].category);
        g_prefs.putUShort(("ccc" + k).c_str(), g_bank.chores[i].completionCount);
        g_prefs.putBool(("cen" + k).c_str(), g_bank.chores[i].enabled);
        g_prefs.putChar(("cpd" + k).c_str(), g_bank.chores[i].pendingChild);
    }
    g_prefs.end();
}

void modelSaveShop() {
    g_prefs.begin("bank", false);
    g_prefs.putUChar("shcnt", g_bank.shopCount);
    for (int i = 0; i < BANK_SHOP_SLOTS; i++) {
        String k = String(i);
        g_prefs.putString(("st" + k).c_str(), g_bank.shop[i].title);
        g_prefs.putLong(("sp" + k).c_str(), g_bank.shop[i].pricePence);
        g_prefs.putBool(("sen" + k).c_str(), g_bank.shop[i].enabled);
    }
    g_prefs.end();
}

void modelSaveAll() {
    modelSaveConfig();
    for (int i = 0; i < BANK_MAX_ACCOUNTS; i++) {
        if (g_bank.accounts[i].active) modelSaveAccount(i);
    }
    modelSaveChores();
    modelSaveShop();
}

bool modelValidateChildPin(int index, const char *pin) {
    if (index < 0 || index >= BANK_MAX_ACCOUNTS || !pin) return false;
    if (!g_bank.accounts[index].active || g_bank.accounts[index].frozen) return false;
    return strcmp(g_bank.accounts[index].pin, pin) == 0;
}

bool modelValidateAdminPin(const char *pin) {
    return pin && strcmp(pin, BANK_ADMIN_PIN) == 0;
}

void modelAddHistory(int account, const char *entry) {
    if (account < 0 || account >= BANK_MAX_ACCOUNTS || !entry) return;
    g_bank.historyLog[account][g_bank.historyIndex[account]] = entry;
    if (++g_bank.historyIndex[account] >= BANK_HISTORY_RAM)
        g_bank.historyIndex[account] = 0;
    storageAppendHistory(g_bank.accounts[account].name, entry);
}

void modelPushNotification(const char *text) {
    if (!text) return;
    uint8_t idx = g_bank.notificationCount % BANK_NOTIFICATION_MAX;
    strncpy(g_bank.notifications[idx].text, text, 63);
    g_bank.notifications[idx].text[63] = '\0';
    g_bank.notifications[idx].unread = true;
    if (g_bank.notificationCount < BANK_NOTIFICATION_MAX) g_bank.notificationCount++;
}

void modelMarkNotificationsRead() {
    for (uint8_t i = 0; i < g_bank.notificationCount; i++)
        g_bank.notifications[i].unread = false;
}

void modelRecordLogin(int account) {
    if (account < 0 || account >= BANK_MAX_ACCOUNTS) return;
    uint32_t today = modelTodayStamp();
    if (g_bank.accounts[account].lastLoginDay + 1 == today)
        g_bank.accounts[account].currentStreak++;
    else if (g_bank.accounts[account].lastLoginDay != today)
        g_bank.accounts[account].currentStreak = 1;
    if (g_bank.accounts[account].currentStreak > g_bank.accounts[account].longestStreak)
        g_bank.accounts[account].longestStreak = g_bank.accounts[account].currentStreak;
    g_bank.accounts[account].lastLoginDay = today;
    modelSaveAccount(account);
}

bool modelHasAchievement(int account, AchievementId id) {
    if (account < 0 || account >= BANK_MAX_ACCOUNTS || (uint8_t)id >= ACH_COUNT) return false;
    return (g_bank.accounts[account].achievementMask & (1UL << id)) != 0;
}

void modelUnlockAchievement(int account, AchievementId id) {
    if (account < 0 || account >= BANK_MAX_ACCOUNTS || (uint8_t)id >= ACH_COUNT) return;
    if (modelHasAchievement(account, id)) return;
    g_bank.accounts[account].achievementMask |= (1UL << id);
    char buf[72];
    snprintf(buf, sizeof(buf), "Achievement: %s", modelAchievementName(id));
    modelAddHistory(account, buf);
    modelPushNotification(buf);
}

void modelCheckAchievements(int account) {
    if (account < 0 || account >= BANK_MAX_ACCOUNTS) return;
    ChildAccount &a = g_bank.accounts[account];
    if (a.choresCompleted >= 1) modelUnlockAchievement(account, ACH_FIRST_CHORE);
    if (a.choresCompleted >= 10) modelUnlockAchievement(account, ACH_CHORES_10);
    if (a.choresCompleted >= 100) modelUnlockAchievement(account, ACH_CHORES_100);
    if (a.savingsBalance + a.balance >= 1000) modelUnlockAchievement(account, ACH_SAVED_10);
    if (a.savingsBalance + a.balance >= 5000) modelUnlockAchievement(account, ACH_SAVED_50);
    if (a.savingsBalance + a.balance >= 10000) modelUnlockAchievement(account, ACH_SAVED_100);
    if (a.currentStreak >= 7) modelUnlockAchievement(account, ACH_STREAK_7);
    if (a.currentStreak >= 30) modelUnlockAchievement(account, ACH_STREAK_30);
    if (a.goalTarget > 0 && a.savingsBalance >= a.goalTarget)
        modelUnlockAchievement(account, ACH_GOAL_REACHED);
}

static void creditAccount(int account, long pence, uint16_t xp, const char *reason) {
    ChildAccount &a = g_bank.accounts[account];
    a.balance += pence;
    a.xp += xp;
    a.weeklyEarned += (uint32_t)pence;
    a.monthlyEarned += (uint32_t)pence;
    a.lifetimeEarned += (uint32_t)pence;
    a.firstDeposit = true;
    char buf[64];
    snprintf(buf, sizeof(buf), "+%s %s", modelMoneyText(pence).c_str(), reason);
    modelAddHistory(account, buf);
    modelCheckAchievements(account);
    modelSaveAccount(account);
}

bool modelRequestChore(int account, int choreIndex) {
    if (account < 0 || choreIndex < 0 || choreIndex >= BANK_CHORE_SLOTS) return false;
    ChoreDef &c = g_bank.chores[choreIndex];
    if (!c.enabled || c.title[0] == '\0' || c.pendingChild >= 0) return false;
    c.pendingChild = (int8_t)account;
    modelSaveChores();
    char buf[48];
    snprintf(buf, sizeof(buf), "Requested %s", c.title);
    modelAddHistory(account, buf);
    return true;
}

void modelApproveChore(int choreIndex) {
    if (choreIndex < 0 || choreIndex >= BANK_CHORE_SLOTS) return;
    ChoreDef &c = g_bank.chores[choreIndex];
    int child = c.pendingChild;
    if (child < 0) return;
    creditAccount(child, c.rewardPence, c.xpReward, c.title);
    g_bank.accounts[child].choresCompleted++;
    c.completionCount++;
    c.pendingChild = -1;

    if (g_bank.config.dailyMissionChore == (uint8_t)choreIndex && !g_bank.config.dailyMissionDone) {
        g_bank.config.dailyMissionDone = true;
        creditAccount(child, 50, 15, "Daily mission bonus");
        modelUnlockAchievement(child, ACH_DAILY_MISSION);
    }
    if (g_bank.config.weeklyChallengeProgress < 10)
        g_bank.config.weeklyChallengeProgress++;

    char note[64];
    snprintf(note, sizeof(note), "Dad approved %s", c.title);
    modelPushNotification(note);
    modelSaveChores();
    modelSaveConfig();
}

void modelRejectChore(int choreIndex) {
    if (choreIndex < 0 || choreIndex >= BANK_CHORE_SLOTS) return;
    g_bank.chores[choreIndex].pendingChild = -1;
    modelSaveChores();
}

bool modelDeposit(int account, long pence) {
    if (account < 0 || pence <= 0) return false;
    creditAccount(account, pence, 10, "Admin deposit");
    return true;
}

bool modelWithdraw(int account, long pence) {
    if (account < 0 || pence <= 0 || g_bank.accounts[account].balance < pence) return false;
    g_bank.accounts[account].balance -= pence;
    char buf[48];
    snprintf(buf, sizeof(buf), "-%s Admin withdraw", modelMoneyText(pence).c_str());
    modelAddHistory(account, buf);
    modelSaveAccount(account);
    return true;
}

bool modelApplyDadTax(int account, long *taxOut) {
    if (account < 0 || !g_bank.config.dadTaxEnabled) return false;
    long tax = (g_bank.accounts[account].balance * g_bank.config.dadTaxPercent) / 100;
    if (tax <= 0) return false;
    g_bank.accounts[account].balance -= tax;
    g_bank.config.totalDadTaxCollected += (uint32_t)tax;
    char buf[48];
    snprintf(buf, sizeof(buf), "-%s Dad Tax", modelMoneyText(tax).c_str());
    modelAddHistory(account, buf);
    modelSaveAccount(account);
    modelSaveConfig();
    if (taxOut) *taxOut = tax;
    return true;
}

bool modelAwardBonus(int account, long pence) {
    if (account < 0 || pence <= 0) return false;
    creditAccount(account, pence, 20, "Dad bonus");
    modelPushNotification("Dad awarded you a bonus!");
    return true;
}

bool modelApplyFine(int account, long pence) {
    if (account < 0 || pence <= 0 || g_bank.accounts[account].balance < pence) return false;
    g_bank.accounts[account].balance -= pence;
    char buf[48];
    snprintf(buf, sizeof(buf), "-%s Dad fine", modelMoneyText(pence).c_str());
    modelAddHistory(account, buf);
    modelSaveAccount(account);
    return true;
}

bool modelTransferToSavings(int account, long pence) {
    if (account < 0 || pence <= 0 || g_bank.accounts[account].balance < pence) return false;
    g_bank.accounts[account].balance -= pence;
    g_bank.accounts[account].savingsBalance += pence;
    char buf[48];
    snprintf(buf, sizeof(buf), "Saved %s", modelMoneyText(pence).c_str());
    modelAddHistory(account, buf);
    modelCheckAchievements(account);
    modelSaveAccount(account);
    return true;
}

bool modelPurchaseShopItem(int account, int shopIndex) {
    if (account < 0 || shopIndex < 0 || shopIndex >= BANK_SHOP_SLOTS) return false;
    ShopItem &item = g_bank.shop[shopIndex];
    if (!item.enabled || item.pricePence <= 0) return false;
    ChildAccount &a = g_bank.accounts[account];
    if (a.balance < item.pricePence) return false;
    a.balance -= item.pricePence;
    a.moneySpent += (uint32_t)item.pricePence;
    char buf[64];
    snprintf(buf, sizeof(buf), "Bought %s -%s", item.title, modelMoneyText(item.pricePence).c_str());
    modelAddHistory(account, buf);
    modelPushNotification("Reward purchased!");
    modelUnlockAchievement(account, ACH_SHOP_SPENDER);
    modelSaveAccount(account);
    return true;
}

void modelRunScheduledInterest() {
    if (!g_bank.config.interestEnabled) return;
    for (int i = 0; i < BANK_MAX_ACCOUNTS; i++) {
        if (!g_bank.accounts[i].active) continue;
        long savings = g_bank.accounts[i].savingsBalance;
        if (savings <= 0) continue;
        long interest = (savings * g_bank.config.interestRatePercent) / 100 / 52;
        if (interest <= 0) continue;
        g_bank.accounts[i].savingsBalance += interest;
        g_bank.config.totalInterestPaid += (uint32_t)interest;
        char buf[48];
        snprintf(buf, sizeof(buf), "+%s Interest", modelMoneyText(interest).c_str());
        modelAddHistory(i, buf);
        modelPushNotification("Interest paid on savings");
        modelSaveAccount(i);
    }
    modelSaveConfig();
}

void modelTick(uint32_t nowMs) {
    (void)nowMs;
    uint32_t today = modelTodayStamp();
    if (today != g_bank.config.dayStamp) {
        g_bank.config.dayStamp = today;
        g_bank.config.dailyMissionDone = false;
        g_bank.config.dailyMissionChore = (uint8_t)(today % g_bank.choreCount);
        modelRunScheduledInterest();
        modelSaveConfig();
    }
}

int modelFindChoreByTitle(const char *title) {
    if (!title) return -1;
    for (int i = 0; i < BANK_CHORE_SLOTS; i++) {
        if (g_bank.chores[i].enabled && strcmp(g_bank.chores[i].title, title) == 0)
            return i;
    }
    return -1;
}

void modelRunScheduledTax() {
    (void)g_bank;
}

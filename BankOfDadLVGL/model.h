#pragma once

#include <Arduino.h>
#include <Preferences.h>

#define BANK_STORAGE_VERSION  220
#define BANK_MAX_ACCOUNTS     6
#define BANK_ACCOUNT_COUNT    4
#define BANK_CHORE_SLOTS      30
#define BANK_SHOP_SLOTS       12
#define BANK_ACHIEVEMENT_COUNT 16
#define BANK_NOTIFICATION_MAX 24
#define BANK_HISTORY_RAM      8
#define BANK_MAX_REWARD_PENCE 450
#define BANK_ADMIN_PIN        "9999"
#define BANK_WIFI_AP_SSID     "BankOfDad"
#define BANK_WIFI_AP_PASS     "pocket123"

enum ChoreCategory : uint8_t {
    CHORE_CAT_BEDROOM = 0,
    CHORE_CAT_KITCHEN,
    CHORE_CAT_BATHROOM,
    CHORE_CAT_LAUNDRY,
    CHORE_CAT_GARDEN,
    CHORE_CAT_OUTDOOR,
    CHORE_CAT_GENERAL,
    CHORE_CAT_COUNT
};

enum AchievementId : uint8_t {
    ACH_FIRST_CHORE = 0,
    ACH_CHORES_10,
    ACH_CHORES_100,
    ACH_SAVED_10,
    ACH_SAVED_50,
    ACH_SAVED_100,
    ACH_STREAK_7,
    ACH_STREAK_30,
    ACH_KITCHEN_KING,
    ACH_LAUNDRY_LEGEND,
    ACH_GARDEN_HERO,
    ACH_SUPER_SAVER,
    ACH_FAMILY_HELPER,
    ACH_SHOP_SPENDER,
    ACH_GOAL_REACHED,
    ACH_DAILY_MISSION,
    ACH_COUNT
};

enum ScreenId : uint8_t {
    SCR_HOME,
    SCR_PIN,
    SCR_ACCOUNT,
    SCR_HISTORY,
    SCR_GOALS,
    SCR_CHORES,
    SCR_ACHIEVEMENTS,
    SCR_LEADERBOARD,
    SCR_SHOP,
    SCR_STATS,
    SCR_NOTIFICATIONS,
    SCR_ADMIN_PIN,
    SCR_ADMIN_MENU,
    SCR_ADMIN_SELECT,
    SCR_ADMIN_ACTION,
    SCR_AMOUNT,
    SCR_APPROVALS,
    SCR_DAD_TAX,
    SCR_ADMIN_CHORES,
    SCR_ADMIN_SHOP,
    SCR_ADMIN_DIAG,
    SCR_MESSAGE
};

enum MsgReturn : uint8_t {
    MSG_RET_HOME,
    MSG_RET_PIN,
    MSG_RET_ADMIN_PIN,
    MSG_RET_ACCOUNT,
    MSG_RET_CHORES,
    MSG_RET_ADMIN_MENU,
    MSG_RET_ADMIN_ACTION,
    MSG_RET_SHOP,
    MSG_RET_STATS
};

struct ChoreDef {
    char           title[32];
    long           rewardPence;
    uint16_t       xpReward;
    ChoreCategory  category;
    uint16_t       completionCount;
    bool           enabled;
    int8_t         pendingChild;
};

struct ChildAccount {
    char     name[16];
    char     pin[5];
    char     avatar[4];
    long     balance;
    long     savingsBalance;
    long     goalTarget;
    char     goalName[24];
    long     xp;
    uint16_t choresCompleted;
    uint16_t currentStreak;
    uint16_t longestStreak;
    uint32_t weeklyEarned;
    uint32_t monthlyEarned;
    uint32_t lifetimeEarned;
    uint32_t moneySpent;
    uint32_t lastLoginDay;
    uint32_t achievementMask;
    bool     frozen;
    bool     active;
    bool     firstDeposit;
};

struct ShopItem {
    char title[24];
    long pricePence;
    bool enabled;
};

struct Notification {
    char text[64];
    bool unread;
};

struct SystemConfig {
    uint16_t storageVersion;
    uint8_t  dadTaxPercent;
    bool     dadTaxEnabled;
    uint8_t  interestRatePercent;
    bool     interestEnabled;
    uint32_t totalDadTaxCollected;
    uint32_t totalInterestPaid;
    uint8_t  weeklyChallengeType;
    uint8_t  weeklyChallengeProgress;
    bool     weeklyChallengeClaimed;
    uint8_t  dailyMissionChore;
    bool     dailyMissionDone;
    uint32_t dayStamp;
};

struct BankState {
    ChildAccount accounts[BANK_MAX_ACCOUNTS];
    uint8_t      accountCount;
    ChoreDef     chores[BANK_CHORE_SLOTS];
    uint8_t      choreCount;
    ShopItem     shop[BANK_SHOP_SLOTS];
    uint8_t      shopCount;
    String       historyLog[BANK_MAX_ACCOUNTS][BANK_HISTORY_RAM];
    int          historyIndex[BANK_MAX_ACCOUNTS];
    Notification notifications[BANK_NOTIFICATION_MAX];
    uint8_t      notificationCount;
    SystemConfig config;
    ScreenId     currentScreen;
    int          selectedAccount;
    bool         adminMode;
    bool         depositMode;
    char         enteredPin[5];
    char         amountInput[7];
    MsgReturn    msgReturn;
    bool         g_taxPath;
};

extern BankState g_bank;
extern Preferences g_prefs;

const char *modelCategoryName(ChoreCategory cat);
const char *modelAchievementName(AchievementId id);
String      modelMoneyText(long pennies);
int         modelLevelFromXp(long xp);
uint32_t    modelTodayStamp();

void modelInitDefaults();
bool modelLoadAll();
void modelSaveAll();
void modelSaveAccount(int index);
void modelSaveChores();
void modelSaveShop();
void modelSaveConfig();

bool modelValidateChildPin(int index, const char *pin);
bool modelValidateAdminPin(const char *pin);

void modelAddHistory(int account, const char *entry);
void modelPushNotification(const char *text);
void modelMarkNotificationsRead();

void modelRecordLogin(int account);
void modelCheckAchievements(int account);
bool modelHasAchievement(int account, AchievementId id);
void modelUnlockAchievement(int account, AchievementId id);

bool modelRequestChore(int account, int choreIndex);
void modelApproveChore(int choreIndex);
void modelRejectChore(int choreIndex);

bool modelDeposit(int account, long pence);
bool modelWithdraw(int account, long pence);
bool modelApplyDadTax(int account, long *taxOut);
bool modelAwardBonus(int account, long pence);
bool modelApplyFine(int account, long pence);
bool modelTransferToSavings(int account, long pence);
bool modelPurchaseShopItem(int account, int shopIndex);

void modelTick(uint32_t nowMs);
void modelRunScheduledInterest();
void modelRunScheduledTax();

int  modelFindChoreByTitle(const char *title);

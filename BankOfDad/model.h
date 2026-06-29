#pragma once

#include "config.h"
#include <Arduino.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>

struct Account {
  char name[24];
  char pin[8];
  long balance;
  long goalTarget;
  long xp;
  bool frozen;
  bool firstDeposit;
  bool firstTen;
  bool firstTwentyFive;
};

struct Chore {
  char title[32];
  long reward;
  int pendingChild;
};

struct Notification {
  char tag[16];
  char message[48];
  uint32_t epoch;
};

extern Account accounts[ACCOUNT_COUNT];
extern Chore chores[CHORE_COUNT];
extern String historyLog[ACCOUNT_COUNT][HISTORY_COUNT];
extern int historyIndex[ACCOUNT_COUNT];
extern Notification notifications[NOTIFY_COUNT];
extern int notifyCount;
extern uint32_t accountAchievements[ACCOUNT_COUNT];
extern uint16_t accountChoreStreak[ACCOUNT_COUNT];
extern uint16_t accountChoresDone[ACCOUNT_COUNT];
extern WebServer webServer;

void modelInit();
void modelWebServe();
void modelWifiTick();
void modelTimeTick();
void modelFormatClock(char *buf, size_t len);
bool modelTimeIsSynced();
void modelFormatWebHint(char *buf, size_t len);
void modelWifiStatusText(char *buf, size_t len);
void modelSaveAccount(int i);
void modelLoadAccounts();
void modelSaveChores();
void modelLoadChores();
void modelAddHistory(int account, const String &entry);
void modelUpdateAchievements(int account);
String modelMoneyText(long pennies);
long modelFamilyBalance();
int modelAccountCount();
bool modelDeposit(int account, long pennies);
bool modelWithdraw(int account, long pennies);
long modelDadTaxAmount(int account);
bool modelApplyDadTax(int account);
bool modelRequestChore(int chore, int child);
bool modelApproveChore(int chore);
void modelLeaderboardOrder(int *orderOut);
void modelPushNotification(const char *tag, const char *message);
void modelFormatNotifyFeed(char *buf, size_t len);
int modelPendingApprovalCount();
int modelTopSaverIndex();
void modelFormatFamilyStats(char *buf, size_t len);
void modelFormatAchievements(int account, char *buf, size_t len);

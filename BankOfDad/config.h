#pragma once

#include <Arduino.h>

#define WIFI_AP_SSID "BankOfDad"
#define WIFI_AP_PASS "pocket123"

#define ACCOUNT_COUNT   4
#define HISTORY_COUNT   8
#define CHORE_COUNT     27
#define GOAL_SLOTS      3
#define VAULT_COUNT     2
#define MISSION_COUNT   6
#define REWARD_COUNT    10
#define AUDIT_COUNT     16
#define AVATAR_COUNT    8
#define DAD_TAX_PERCENT 10

#define NOTIFY_COUNT    12
#define PLATFORM_VERSION "2.2.0"

/** Achievement bit flags stored per account in NVS as ach{n}. */
#define ACH_FIRST_DEPOSIT  (1UL << 0)
#define ACH_SAVE_10        (1UL << 1)
#define ACH_SAVE_25        (1UL << 2)
#define ACH_SAVE_50        (1UL << 3)
#define ACH_SAVE_100       (1UL << 4)
#define ACH_SAVE_500       (1UL << 5)
#define ACH_STREAK_7       (1UL << 6)
#define ACH_STREAK_30      (1UL << 7)
#define ACH_CHORES_100     (1UL << 8)
#define ACH_SUPER_SAVER    (1UL << 9)
#define ACH_GOAL_CRUSHER   (1UL << 10)
#define ACH_FAMILY_HELPER  (1UL << 11)

#define SAVE_10_PENCE   1000L
#define SAVE_25_PENCE   2500L
#define SAVE_50_PENCE   5000L
#define SAVE_100_PENCE  10000L
#define SAVE_500_PENCE  50000L

#define TOUCH_DEBOUNCE_MS 120

/** UK time: GMT + BST. Change offsets for your region. */
#define NTP_GMT_OFFSET_SEC      0
#define NTP_DAYLIGHT_OFFSET_SEC 3600
#define NTP_SERVER_1            "pool.ntp.org"
#define NTP_SERVER_2            "time.google.com"

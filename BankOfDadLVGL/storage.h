#pragma once

#include <Arduino.h>

/* JC8048W550C microSD (SPI) — change pins if your board differs */
#define SD_SCK  12
#define SD_MISO 13
#define SD_MOSI 11
#define SD_CS   10

bool storageInit();
bool storageReady();
const char *storageStatusText();

void storageAppendHistory(const char *accountName, const char *entry);
int  storageReadHistory(const char *accountName, String *out, int maxLines);

struct StorageAccountSnap {
    const char *name;
    long        balance;
    long        xp;
    bool        firstDeposit;
    bool        firstTen;
    bool        firstTwentyFive;
};

void storageBackupAccounts(const StorageAccountSnap *accts, int count);
bool storageRestoreAccounts(StorageAccountSnap *accts, int count);

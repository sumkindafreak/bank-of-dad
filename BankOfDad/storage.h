#pragma once

#include <Arduino.h>

/* CYD microSD — shares the TFT SPI bus, CS on GPIO 5 */
#define SD_CS 5

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

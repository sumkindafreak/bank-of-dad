#include "storage.h"
#include <SD.h>
#include <SPI.h>

static bool s_ready = false;

static String historyPath(const char *name) {
    return String("/bank/") + name + ".log";
}

bool storageInit() {
    if (!SD.begin(SD_CS, SPI)) {
        Serial.println("SD: not mounted (optional — insert a FAT32 card)");
        s_ready = false;
        return false;
    }
    if (!SD.exists("/bank")) SD.mkdir("/bank");
    s_ready = true;
    Serial.println("SD: ready — extended history + backups enabled");
    return true;
}

bool storageReady() { return s_ready; }

const char *storageStatusText() {
    return s_ready ? "SD OK" : "No SD";
}

void storageAppendHistory(const char *accountName, const char *entry) {
    if (!s_ready || !accountName || !entry) return;
    File f = SD.open(historyPath(accountName), FILE_APPEND);
    if (!f) return;
    f.println(entry);
    f.close();
}

int storageReadHistory(const char *accountName, String *out, int maxLines) {
    if (!s_ready || !accountName || !out || maxLines <= 0) return 0;

    File f = SD.open(historyPath(accountName), FILE_READ);
    if (!f) return 0;

    /* Count lines, then read the last maxLines into a small ring */
    String ring[24];
    int cap = maxLines < 24 ? maxLines : 24;
    int count = 0;
    int total = 0;

    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;
        ring[count % cap] = line;
        count++;
        total++;
    }
    f.close();

    if (total == 0) return 0;

    int n = total < cap ? total : cap;
    int start = total < cap ? 0 : count % cap;
    for (int i = 0; i < n; i++) {
        out[i] = ring[(start + i) % cap];
    }
    return n;
}

void storageBackupAccounts(const StorageAccountSnap *accts, int count) {
    if (!s_ready || !accts) return;
    File f = SD.open("/bank/backup.csv", FILE_WRITE);
    if (!f) return;
    f.println("name,balance,xp,firstDeposit,firstTen,firstTwentyFive");
    for (int i = 0; i < count; i++) {
        f.printf("%s,%ld,%ld,%d,%d,%d\n",
                 accts[i].name, accts[i].balance, accts[i].xp,
                 accts[i].firstDeposit ? 1 : 0,
                 accts[i].firstTen ? 1 : 0,
                 accts[i].firstTwentyFive ? 1 : 0);
    }
    f.close();
}

bool storageRestoreAccounts(StorageAccountSnap *accts, int count) {
    if (!s_ready || !accts) return false;
    if (!SD.exists("/bank/backup.csv")) return false;

    File f = SD.open("/bank/backup.csv", FILE_READ);
    if (!f) return false;
    f.readStringUntil('\n'); /* skip header */

    bool any = false;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        int c1 = line.indexOf(',');
        int c2 = line.indexOf(',', c1 + 1);
        int c3 = line.indexOf(',', c2 + 1);
        int c4 = line.indexOf(',', c3 + 1);
        int c5 = line.indexOf(',', c4 + 1);
        if (c5 < 0) continue;

        String name = line.substring(0, c1);
        for (int i = 0; i < count; i++) {
            if (name.equals(accts[i].name)) {
                accts[i].balance         = line.substring(c1 + 1, c2).toInt();
                accts[i].xp              = line.substring(c2 + 1, c3).toInt();
                accts[i].firstDeposit    = line.substring(c3 + 1, c4).toInt() != 0;
                accts[i].firstTen        = line.substring(c4 + 1, c5).toInt() != 0;
                accts[i].firstTwentyFive = line.substring(c5 + 1).toInt() != 0;
                any = true;
                break;
            }
        }
    }
    f.close();
    return any;
}

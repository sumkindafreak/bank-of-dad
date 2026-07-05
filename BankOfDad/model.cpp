#include "model.h"
#include "config.h"
#include "platform.h"

#include <time.h>

static Preferences prefs;
static bool s_webStarted = false;

Account accounts[ACCOUNT_COUNT] = {
    {"Nixon", "1111", 1250, 5000, 40, false, false, false, false},
    {"Oakley", "2222", 800, 3000, 20, false, false, false, false},
    {"Hadley", "3333", 1575, 4500, 65, false, false, false, false},
    {"Maddison", "4444", 2200, 6000, 120, false, false, false, false},
};

Chore chores[CHORE_COUNT] = {
    {"Make Bed", 20, -1},
    {"Laundry Basket", 20, -1},
    {"Tidy Bedroom", 50, -1},
    {"Empty Bin", 50, -1},
    {"Feed Pets", 50, -1},
    {"Water Plants", 50, -1},
    {"Set Table", 50, -1},
    {"Clear Table", 50, -1},
    {"Wipe Sides", 75, -1},
    {"Vacuum Room", 100, -1},
    {"Sweep Floors", 100, -1},
    {"Dust Downstairs", 100, -1},
    {"Unload Dishwasher", 100, -1},
    {"Load Dishwasher", 125, -1},
    {"Fold Washing", 125, -1},
    {"Hang Washing", 125, -1},
    {"Bring Washing In", 125, -1},
    {"Clean Sink Mirror", 150, -1},
    {"Mop Kitchen", 150, -1},
    {"Hoover Downstairs", 200, -1},
    {"Clean Toilet", 200, -1},
    {"Help Dinner", 200, -1},
    {"Wash Car", 250, -1},
    {"Mow Lawn", 300, -1},
    {"Clean Windows", 350, -1},
    {"Deep Bathroom", 400, -1},
    {"Deep Kitchen", 450, -1},
};

String historyLog[ACCOUNT_COUNT][HISTORY_COUNT];
int historyIndex[ACCOUNT_COUNT] = {0};
Notification notifications[NOTIFY_COUNT];
int notifyCount = 0;
uint32_t accountAchievements[ACCOUNT_COUNT] = {0};
uint16_t accountChoreStreak[ACCOUNT_COUNT] = {0};
uint16_t accountChoresDone[ACCOUNT_COUNT] = {0};
WebServer webServer(80);

static const char *DEFAULT_CHORE_TITLES[CHORE_COUNT] = {
    "Make Bed", "Laundry Basket", "Tidy Bedroom", "Empty Bin", "Feed Pets",
    "Water Plants", "Set Table", "Clear Table", "Wipe Sides", "Vacuum Room",
    "Sweep Floors", "Dust Downstairs", "Unload Dishwasher", "Load Dishwasher",
    "Fold Washing", "Hang Washing", "Bring Washing In", "Clean Sink Mirror",
    "Mop Kitchen", "Hoover Downstairs", "Clean Toilet", "Help Dinner",
    "Wash Car", "Mow Lawn", "Clean Windows", "Deep Bathroom", "Deep Kitchen",
};

static const long DEFAULT_CHORE_REWARDS[CHORE_COUNT] = {
    20, 20, 50, 50, 50, 50, 50, 50, 75, 100, 100, 100, 100, 125, 125, 125,
    125, 150, 150, 200, 200, 200, 250, 300, 350, 400, 450,
};

static String buildPageHeader() {
  return String(
      "<!DOCTYPE html><html><head>"
      "<meta name='viewport' content='width=device-width,initial-scale=1'>"
      "<title>Bank of Dad</title><style>"
      "body{font-family:sans-serif;max-width:520px;margin:0 auto;padding:16px;"
      "background:#111;color:#eee}"
      "h1{color:#e74c3c;margin:0 0 2px}"
      "p.sub{color:#888;margin:0 0 16px;font-size:.9em}"
      ".panel{background:#222;padding:12px;margin:8px 0;border-radius:8px}"
      "label{display:block;margin-bottom:4px;font-size:.85em;color:#aaa}"
      "input[type=text],input[type=password],input[type=number]{"
      "width:100%;box-sizing:border-box;padding:8px;background:#333;color:#fff;"
      "border:1px solid #555;border-radius:4px;margin-bottom:8px}"
      "button,.btn{display:inline-block;padding:10px 14px;background:#e74c3c;"
      "color:#fff;border:none;border-radius:8px;font-size:.95em;cursor:pointer;"
      "text-decoration:none;margin:4px 4px 4px 0}"
      "button:hover,.btn:hover{background:#c0392b}"
      ".btn.secondary{background:#444}.btn.secondary:hover{background:#555}"
      ".ok{padding:10px;background:#27ae60;border-radius:6px;margin-bottom:12px}"
      ".warn{font-size:.85em;color:#e67e22}"
      ".nav{margin-bottom:12px}"
      ".chore{background:#222;padding:12px;margin:8px 0;border-radius:8px}"
      "</style></head><body>"
      "<h1>Bank of Dad</h1>"
      "<p class='sub'>v") +
         PLATFORM_VERSION + " — Dad admin panel</p>"
                          "<div class='nav'>"
                          "<a class='btn' href='/'>Chores</a>"
                          "<a class='btn secondary' href='/wifi'>WiFi &amp; OTA</a>"
                          "</div>";
}

static String buildStatusPanel() {
  char status[96];
  modelWifiStatusText(status, sizeof(status));

  String html = "<div class='panel'><strong>Network</strong><br>";
  html += status;
  html += "<br><span class='warn'>OTA password: ";
  html += OTA_PASSWORD;
  html += " (Arduino IDE → Upload Using OTA when on home WiFi)</span></div>";
  return html;
}

static String buildChorePage(bool saved) {
  String html = buildPageHeader();
  html += buildStatusPanel();

  if (saved) html += "<div class='ok'>&#10003; Chore changes saved</div>";

  html += "<form method='POST' action='/update'>";

  for (int i = 0; i < CHORE_COUNT; i++) {
    char rewardBuf[12];
    dtostrf(chores[i].reward / 100.0f, 1, 2, rewardBuf);

    String pending;
    if (chores[i].pendingChild >= 0) {
      pending = "<p class='warn'>&#9888; Pending approval for " +
                String(accounts[chores[i].pendingChild].name) + "</p>";
    }

    html += "<div class='chore'>";
    html += "<label>Chore " + String(i + 1) + " — Name</label>";
    html += "<input type='text' name='t" + String(i) + "' value=\"";
    html += String(chores[i].title);
    html += "\" maxlength='31' required>";
    html += "<label>Reward (GBP)</label>";
    html += "<input type='number' name='r" + String(i) + "' step='0.01' min='0' max='4.50' value='";
    html += String(rewardBuf);
    html += "'>";
    html += pending;
    html += "</div>";
  }

  html += "<button type='submit'>Save Chores</button></form></body></html>";
  return html;
}

static String buildWifiPage(bool saved, bool cleared) {
  char ssid[33] = {0};
  modelWifiGetCredentials(ssid, sizeof(ssid), nullptr, 0);

  String html = buildPageHeader();
  html += buildStatusPanel();

  if (saved) html += "<div class='ok'>&#10003; Home WiFi saved — reconnecting</div>";
  if (cleared) html += "<div class='ok'>&#10003; Home WiFi forgotten</div>";

  html += "<div class='panel'>";
  html += "<strong>Connect to your home network</strong>";
  html += "<p class='warn'>Once connected, open this page from your phone using the home IP shown above. "
          "The BankOfDad hotspot stays available for setup.</p>";
  html += "<form method='POST' action='/wifi/save'>";
  html += "<label>Home WiFi name (SSID)</label>";
  html += "<input type='text' name='ssid' maxlength='32' value=\"";
  html += String(ssid);
  html += "\" required>";
  html += "<label>WiFi password</label>";
  html += "<input type='password' name='pass' maxlength='64' placeholder='Leave blank to keep saved password'>";
  html += "<button type='submit'>Save &amp; Connect</button>";
  html += "</form>";
  html += "<form method='POST' action='/wifi/forget' style='margin-top:12px'>";
  html += "<button type='submit' class='btn secondary'>Forget Home WiFi</button>";
  html += "</form>";
  html += "</div>";

  html += "<div class='panel'>";
  html += "<strong>Over-the-air updates</strong>";
  html += "<p>When connected to home WiFi, upload firmware from Arduino IDE using "
          "<em>Sketch → Upload Using OTA</em>.</p>";
  html += "<p>Hostname: <code>";
  html += OTA_HOSTNAME;
  html += ".local</code> &nbsp; Port: <code>";
  html += OTA_PORT;
  html += "</code></p>";
  html += "<p>Password: <code>";
  html += OTA_PASSWORD;
  html += "</code></p>";
  html += "</div></body></html>";

  return html;
}

static void setupWebRoutes() {
  webServer.on("/", HTTP_GET, []() {
    webServer.send(200, "text/html", buildChorePage(webServer.hasArg("saved")));
  });

  webServer.on("/update", HTTP_POST, []() {
    for (int i = 0; i < CHORE_COUNT; i++) {
      String tk = "t" + String(i);
      String rk = "r" + String(i);

      if (webServer.hasArg(tk)) {
        String val = webServer.arg(tk);
        val.trim();
        if (val.length() > 0 && val.length() < 32) {
          strncpy(chores[i].title, val.c_str(), 31);
          chores[i].title[31] = '\0';
        }
      }

      if (webServer.hasArg(rk)) {
        float pounds = webServer.arg(rk).toFloat();
        if (pounds >= 0.0f && pounds <= 4.50f)
          chores[i].reward = (long)(pounds * 100.0f + 0.5f);
      }
    }

    modelSaveChores();
    webServer.sendHeader("Location", "/?saved=1");
    webServer.send(302, "text/plain", "");
  });

  webServer.on("/wifi", HTTP_GET, []() {
    webServer.send(200, "text/html",
                   buildWifiPage(webServer.hasArg("saved"), webServer.hasArg("cleared")));
  });

  webServer.on("/wifi/save", HTTP_POST, []() {
    if (!webServer.hasArg("ssid")) {
      webServer.send(400, "text/plain", "Missing SSID");
      return;
    }

    String ssid = webServer.arg("ssid");
    ssid.trim();
    if (ssid.length() == 0 || ssid.length() > 32) {
      webServer.send(400, "text/plain", "Invalid SSID");
      return;
    }

    char existingPass[65] = {0};
    modelWifiGetCredentials(nullptr, 0, existingPass, sizeof(existingPass));

    String pass = webServer.hasArg("pass") ? webServer.arg("pass") : String("");
    pass.trim();
    if (pass.length() == 0) pass = String(existingPass);

    if (pass.length() > 64) {
      webServer.send(400, "text/plain", "Password too long");
      return;
    }

    modelWifiSetCredentials(ssid.c_str(), pass.c_str());
    modelWifiRequestReconnect();

    webServer.sendHeader("Location", "/wifi?saved=1");
    webServer.send(302, "text/plain", "");
  });

  webServer.on("/wifi/forget", HTTP_POST, []() {
    modelWifiClearCredentials();
    modelWifiRequestReconnect();
    webServer.sendHeader("Location", "/wifi?cleared=1");
    webServer.send(302, "text/plain", "");
  });

  webServer.onNotFound([]() {
    webServer.sendHeader("Location", "/");
    webServer.send(302, "text/plain", "");
  });
}

String modelMoneyText(long pennies) {
  long pounds = pennies / 100;
  int pence = abs((int)(pennies % 100));
  char buf[16];
  snprintf(buf, sizeof(buf), "£%ld.%02d", pounds, pence);
  return String(buf);
}

void modelSaveAccount(int i) {
  if (i < 0 || i >= ACCOUNT_COUNT) return;
  prefs.begin("bank", false);
  String k = String(i);
  prefs.putLong(("bal" + k).c_str(), accounts[i].balance);
  prefs.putLong(("xp" + k).c_str(), accounts[i].xp);
  prefs.putBool(("fd" + k).c_str(), accounts[i].firstDeposit);
  prefs.putBool(("f10" + k).c_str(), accounts[i].firstTen);
  prefs.putBool(("f25" + k).c_str(), accounts[i].firstTwentyFive);
  prefs.putBool(("frz" + k).c_str(), accounts[i].frozen);
  prefs.putLong(("gt" + k).c_str(), accounts[i].goalTarget);
  prefs.end();
}

void modelLoadAccounts() {
  prefs.begin("bank", true);
  for (int i = 0; i < ACCOUNT_COUNT; i++) {
    String k = String(i);
    accounts[i].balance = prefs.getLong(("bal" + k).c_str(), accounts[i].balance);
    accounts[i].xp = prefs.getLong(("xp" + k).c_str(), accounts[i].xp);
    accounts[i].firstDeposit = prefs.getBool(("fd" + k).c_str(), accounts[i].firstDeposit);
    accounts[i].firstTen = prefs.getBool(("f10" + k).c_str(), accounts[i].firstTen);
    accounts[i].firstTwentyFive = prefs.getBool(("f25" + k).c_str(), accounts[i].firstTwentyFive);
    accounts[i].frozen = prefs.getBool(("frz" + k).c_str(), accounts[i].frozen);
    accounts[i].goalTarget = prefs.getLong(("gt" + k).c_str(), accounts[i].goalTarget);
  }
  prefs.end();
}

void modelSaveChores() {
  prefs.begin("bank", false);
  for (int i = 0; i < CHORE_COUNT; i++) {
    String k = String(i);
    prefs.putString(("ct" + k).c_str(), chores[i].title);
    prefs.putLong(("cr" + k).c_str(), chores[i].reward);
    prefs.putInt(("cp" + k).c_str(), chores[i].pendingChild);
  }
  prefs.end();
}

void modelLoadChores() {
  prefs.begin("bank", true);
  for (int i = 0; i < CHORE_COUNT; i++) {
    String k = String(i);
    String title = prefs.getString(("ct" + k).c_str(), DEFAULT_CHORE_TITLES[i]);
    title.toCharArray(chores[i].title, sizeof(chores[i].title));
    chores[i].reward = prefs.getLong(("cr" + k).c_str(), DEFAULT_CHORE_REWARDS[i]);
    chores[i].pendingChild = prefs.getInt(("cp" + k).c_str(), -1);
  }
  prefs.end();
}

void modelAddHistory(int account, const String &entry) {
  if (account < 0 || account >= ACCOUNT_COUNT) return;
  historyLog[account][historyIndex[account]] = entry;
  historyIndex[account] = (historyIndex[account] + 1) % HISTORY_COUNT;
}

void modelUpdateAchievements(int account) {
  if (account < 0 || account >= ACCOUNT_COUNT) return;
  if (accounts[account].balance >= SAVE_10_PENCE) accounts[account].firstTen = true;
  if (accounts[account].balance >= SAVE_25_PENCE) accounts[account].firstTwentyFive = true;
}

long modelFamilyBalance() {
  long total = 0;
  for (int i = 0; i < ACCOUNT_COUNT; i++) total += accounts[i].balance;
  return total;
}

int modelAccountCount() { return ACCOUNT_COUNT; }

bool modelDeposit(int account, long pennies) {
  if (account < 0 || account >= ACCOUNT_COUNT || pennies <= 0) return false;
  accounts[account].balance += pennies;
  if (!accounts[account].firstDeposit) accounts[account].firstDeposit = true;
  modelUpdateAchievements(account);
  modelSaveAccount(account);
  return true;
}

bool modelWithdraw(int account, long pennies) {
  if (account < 0 || account >= ACCOUNT_COUNT || pennies <= 0) return false;
  if (accounts[account].balance < pennies) return false;
  accounts[account].balance -= pennies;
  modelSaveAccount(account);
  return true;
}

long modelDadTaxAmount(int account) {
  if (account < 0 || account >= ACCOUNT_COUNT) return 0;
  return (accounts[account].balance * DAD_TAX_PERCENT) / 100;
}

bool modelApplyDadTax(int account) {
  long tax = modelDadTaxAmount(account);
  if (tax <= 0) return false;
  accounts[account].balance -= tax;
  modelSaveAccount(account);
  return true;
}

bool modelRequestChore(int chore, int child) {
  if (chore < 0 || chore >= CHORE_COUNT || child < 0 || child >= ACCOUNT_COUNT) return false;
  if (chores[chore].pendingChild >= 0) return false;
  chores[chore].pendingChild = child;
  modelSaveChores();
  modelPushNotification("CHORE", chores[chore].title);
  return true;
}

bool modelApproveChore(int chore) {
  if (chore < 0 || chore >= CHORE_COUNT) return false;
  int child = chores[chore].pendingChild;
  if (child < 0 || child >= ACCOUNT_COUNT) return false;

  long reward = chores[chore].reward;
  accounts[child].balance += reward;
  accounts[child].xp += max(5L, reward / 5);
  accountChoresDone[child]++;
  chores[chore].pendingChild = -1;

  modelUpdateAchievements(child);
  modelSaveAccount(child);
  modelSaveChores();
  modelAddHistory(child, "Approved: " + String(chores[chore].title) + " +" + modelMoneyText(reward));
  modelPushNotification("PAID", chores[chore].title);
  return true;
}

void modelLeaderboardOrder(int *orderOut) {
  for (int i = 0; i < ACCOUNT_COUNT; i++) orderOut[i] = i;
  for (int i = 0; i < ACCOUNT_COUNT - 1; i++) {
    for (int j = i + 1; j < ACCOUNT_COUNT; j++) {
      if (accounts[orderOut[j]].balance > accounts[orderOut[i]].balance) {
        int tmp = orderOut[i];
        orderOut[i] = orderOut[j];
        orderOut[j] = tmp;
      }
    }
  }
}

void modelPushNotification(const char *tag, const char *message) {
  if (!tag || !message) return;
  int idx = notifyCount < NOTIFY_COUNT ? notifyCount : NOTIFY_COUNT - 1;
  if (notifyCount >= NOTIFY_COUNT) {
    for (int i = 0; i < NOTIFY_COUNT - 1; i++) notifications[i] = notifications[i + 1];
    idx = NOTIFY_COUNT - 1;
  } else {
    notifyCount++;
  }
  strlcpy(notifications[idx].tag, tag, sizeof(notifications[idx].tag));
  strlcpy(notifications[idx].message, message, sizeof(notifications[idx].message));
  notifications[idx].epoch = time(nullptr);
}

void modelFormatNotifyFeed(char *buf, size_t len) {
  if (!buf || len == 0) return;
  buf[0] = '\0';
  for (int i = max(0, notifyCount - 4); i < notifyCount; i++) {
    char line[80];
    snprintf(line, sizeof(line), "[%s] %s\n", notifications[i].tag, notifications[i].message);
    strlcat(buf, line, len);
  }
}

int modelPendingApprovalCount() {
  int n = 0;
  for (int i = 0; i < CHORE_COUNT; i++)
    if (chores[i].pendingChild >= 0) n++;
  return n;
}

int modelTopSaverIndex() {
  int best = 0;
  for (int i = 1; i < ACCOUNT_COUNT; i++)
    if (accounts[i].balance > accounts[best].balance) best = i;
  return best;
}

void modelFormatFamilyStats(char *buf, size_t len) {
  snprintf(buf, len, "Family £%.2f | Pending %d",
           modelFamilyBalance() / 100.0f, modelPendingApprovalCount());
}

void modelFormatAchievements(int account, char *buf, size_t len) {
  if (!buf || len == 0 || account < 0 || account >= ACCOUNT_COUNT) return;
  snprintf(buf, len, "XP %ld | Done %u", accounts[account].xp, accountChoresDone[account]);
}

bool modelWifiHasCredentials() {
  prefs.begin("wifi", true);
  String ssid = prefs.getString("ssid", "");
  prefs.end();
  return ssid.length() > 0;
}

void modelWifiGetCredentials(char *ssid, size_t ssidLen, char *pass, size_t passLen) {
  prefs.begin("wifi", true);
  String savedSsid = prefs.getString("ssid", "");
  String savedPass = prefs.getString("pass", "");
  prefs.end();

  if (ssid && ssidLen > 0) strlcpy(ssid, savedSsid.c_str(), ssidLen);
  if (pass && passLen > 0) strlcpy(pass, savedPass.c_str(), passLen);
}

bool modelWifiSetCredentials(const char *ssid, const char *pass) {
  if (!ssid || ssid[0] == '\0') return false;
  prefs.begin("wifi", false);
  prefs.putString("ssid", ssid);
  prefs.putString("pass", pass ? pass : "");
  prefs.end();
  return true;
}

void modelWifiClearCredentials() {
  prefs.begin("wifi", false);
  prefs.remove("ssid");
  prefs.remove("pass");
  prefs.end();
}

void modelWifiRequestReconnect() {
  platformWifiReconnect();
}

void modelWifiStatusText(char *buf, size_t len) {
  if (!buf || len == 0) return;

  if (platformStaConnected()) {
    snprintf(buf, len, "Home WiFi: %s  (http://%s)",
             modelWifiHasCredentials() ? "connected" : "connected",
             platformStaIpText());
  } else if (modelWifiHasCredentials()) {
    snprintf(buf, len, "Home WiFi: reconnecting…  AP: %s (http://%s)",
             WIFI_AP_SSID, platformApIpText());
  } else {
    snprintf(buf, len, "Setup hotspot: %s  (http://%s/wifi)",
             WIFI_AP_SSID, platformApIpText());
  }
}

void modelFormatWebHint(char *buf, size_t len) {
  if (!buf || len == 0) return;
  if (platformStaConnected()) {
    snprintf(buf, len, "Phone: http://%s", platformStaIpText());
  } else {
    snprintf(buf, len, "Setup: http://%s/wifi", platformApIpText());
  }
}

void modelFormatClock(char *buf, size_t len) {
  if (!buf || len == 0) return;
  if (!modelTimeIsSynced()) {
    strlcpy(buf, "--:--", len);
    return;
  }
  time_t now = time(nullptr);
  struct tm tmInfo;
  localtime_r(&now, &tmInfo);
  strftime(buf, len, "%H:%M", &tmInfo);
}

bool modelTimeIsSynced() {
  return time(nullptr) > 1700000000;
}

void modelTimeTick() {
  static unsigned long last = 0;
  if (millis() - last < 3600000UL) return;
  last = millis();
  if (platformStaConnected()) configTime(NTP_GMT_OFFSET_SEC, NTP_DAYLIGHT_OFFSET_SEC,
                                         NTP_SERVER_1, NTP_SERVER_2);
}

void modelWifiTick() {
  modelTimeTick();
}

void modelInit() {
  modelLoadAccounts();
  modelLoadChores();

  setupWebRoutes();
  webServer.begin();
  s_webStarted = true;

  Serial.println("[Web] Chore editor: http://192.168.4.1/");
  Serial.println("[Web] WiFi setup:   http://192.168.4.1/wifi");
}

void modelWebServe() {
  if (s_webStarted) webServer.handleClient();
}

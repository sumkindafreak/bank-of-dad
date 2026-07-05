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

Reward rewards[REWARD_COUNT] = {
    {"Chocolate Bar", 200},
    {"Pick Tonight's Film", 500},
    {"Stay Up 30 Minutes", 750},
    {"McDonald's Treat", 1500},
    {"Cinema Trip", 3000},
    {"Video Game Fund", 5000},
    {"Ice Cream Trip", 300},
    {"Choose Takeaway", 2000},
    {"Book Voucher", 800},
    {"Theme Park Fund", 10000},
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

static const char *DEFAULT_REWARD_TITLES[REWARD_COUNT] = {
    "Chocolate Bar", "Pick Tonight's Film", "Stay Up 30 Minutes", "McDonald's Treat",
    "Cinema Trip", "Video Game Fund", "Ice Cream Trip", "Choose Takeaway",
    "Book Voucher", "Theme Park Fund",
};

static const long DEFAULT_REWARD_COSTS[REWARD_COUNT] = {
    200, 500, 750, 1500, 3000, 5000, 300, 2000, 800, 10000,
};

static String htmlEscape(const char *text) {
  String out;
  if (!text) return out;
  for (const char *p = text; *p; p++) {
    if (*p == '&') out += "&amp;";
    else if (*p == '"') out += "&quot;";
    else if (*p == '<') out += "&lt;";
    else if (*p == '>') out += "&gt;";
    else out += *p;
  }
  return out;
}

static String poundsInput(long pennies) {
  char buf[12];
  dtostrf(pennies / 100.0f, 1, 2, buf);
  return String(buf);
}

static long parsePounds(const String &raw, long maxPence = 999999L) {
  float pounds = raw.toFloat();
  if (pounds < 0.0f) return -1;
  long pennies = (long)(pounds * 100.0f + 0.5f);
  if (pennies > maxPence) return -1;
  return pennies;
}

static String buildPageHeader(const char *active) {
  auto navLink = [&](const char *href, const char *label) {
    bool isActive = active && strcmp(active, href) == 0;
    String cls = isActive ? "btn active" : "btn secondary";
    return String("<a class='") + cls + "' href='" + href + "'>" + label + "</a>";
  };

  return String(
      "<!DOCTYPE html><html><head>"
      "<meta name='viewport' content='width=device-width,initial-scale=1'>"
      "<title>Bank of Dad</title><style>"
      "body{font-family:sans-serif;max-width:560px;margin:0 auto;padding:16px;"
      "background:#111;color:#eee}"
      "h1{color:#e74c3c;margin:0 0 2px}"
      "h2{color:#fff;font-size:1.1em;margin:0 0 10px}"
      "p.sub{color:#888;margin:0 0 16px;font-size:.9em}"
      ".panel{background:#222;padding:12px;margin:8px 0;border-radius:8px}"
      ".item{background:#1a1a1a;padding:12px;margin:8px 0;border-radius:8px;border:1px solid #333}"
      "label{display:block;margin-bottom:4px;font-size:.85em;color:#aaa}"
      "input[type=text],input[type=password],input[type=number]{"
      "width:100%;box-sizing:border-box;padding:8px;background:#333;color:#fff;"
      "border:1px solid #555;border-radius:4px;margin-bottom:8px}"
      "button,.btn{display:inline-block;padding:10px 14px;background:#e74c3c;"
      "color:#fff;border:none;border-radius:8px;font-size:.95em;cursor:pointer;"
      "text-decoration:none;margin:4px 4px 4px 0}"
      "button:hover,.btn:hover{background:#c0392b}"
      ".btn.secondary{background:#444}.btn.secondary:hover{background:#555}"
      ".btn.active{background:#27ae60}.btn.good{background:#27ae60}"
      ".btn.good:hover{background:#219653}"
      ".btn.warn{background:#555}.btn.danger{background:#922b21}"
      ".ok{padding:10px;background:#27ae60;border-radius:6px;margin-bottom:12px}"
      ".warn{font-size:.85em;color:#e67e22}"
      ".nav{margin-bottom:12px;display:flex;flex-wrap:wrap;gap:4px}"
      ".scroll{max-height:70vh;overflow-y:auto;padding-right:4px}"
      ".row{display:grid;grid-template-columns:1fr 1fr;gap:8px}"
      ".actions{display:flex;flex-wrap:wrap;gap:6px;margin-top:8px}"
      "</style></head><body>"
      "<h1>Bank of Dad</h1>"
      "<p class='sub'>v") +
         PLATFORM_VERSION + " — Dad admin panel</p>"
                          "<div class='nav'>" +
                          navLink("/", "Chores") +
                          navLink("/rewards", "Shop") +
                          navLink("/accounts", "Accounts") +
                          navLink("/approvals", "Approvals") +
                          navLink("/wifi", "WiFi") +
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
  String html = buildPageHeader("/");
  html += buildStatusPanel();

  if (saved) html += "<div class='ok'>&#10003; Chore changes saved</div>";

  html += "<form method='POST' action='/update'><div class='scroll'>";
  html += "<p class='warn'>Edit chore names and rewards (max £4.50 each). Changes apply on the device immediately.</p>";

  for (int i = 0; i < CHORE_COUNT; i++) {
    String pending;
    if (chores[i].pendingChild >= 0) {
      pending = "<p class='warn'>&#9888; Pending approval for " +
                htmlEscape(accounts[chores[i].pendingChild].name) + "</p>";
    }

    html += "<div class='item'>";
    html += "<label>Chore " + String(i + 1) + " — Name</label>";
    html += "<input type='text' name='t" + String(i) + "' value=\"";
    html += htmlEscape(chores[i].title);
    html += "\" maxlength='31' required>";
    html += "<label>Reward (GBP)</label>";
    html += "<input type='number' name='r" + String(i) + "' step='0.01' min='0' max='4.50' value='";
    html += poundsInput(chores[i].reward);
    html += "'>";
    html += pending;
    html += "</div>";
  }

  html += "</div><button type='submit'>Save All Chores</button></form></body></html>";
  return html;
}

static String buildRewardsPage(bool saved) {
  String html = buildPageHeader("/rewards");
  html += buildStatusPanel();
  if (saved) html += "<div class='ok'>&#10003; Shop rewards saved</div>";

  html += "<form method='POST' action='/rewards/update'><div class='scroll'>";
  html += "<p class='warn'>Edit prize shop items and prices. Children buy these from their balance on the device.</p>";

  for (int i = 0; i < REWARD_COUNT; i++) {
    html += "<div class='item'>";
    html += "<label>Reward " + String(i + 1) + " — Name</label>";
    html += "<input type='text' name='n" + String(i) + "' value=\"";
    html += htmlEscape(rewards[i].title);
    html += "\" maxlength='31' required>";
    html += "<label>Price (GBP)</label>";
    html += "<input type='number' name='c" + String(i) + "' step='0.01' min='0' max='999.99' value='";
    html += poundsInput(rewards[i].cost);
    html += "'>";
    html += "</div>";
  }

  html += "</div><button type='submit'>Save All Rewards</button></form></body></html>";
  return html;
}

static String buildAccountsPage(bool saved) {
  String html = buildPageHeader("/accounts");
  html += buildStatusPanel();
  if (saved) html += "<div class='ok'>&#10003; Account changes saved</div>";

  html += "<form method='POST' action='/accounts/update'><div class='scroll'>";
  html += "<p class='warn'>Edit child names, PINs, balances and savings goals.</p>";

  for (int i = 0; i < ACCOUNT_COUNT; i++) {
    html += "<div class='item'>";
    html += "<h2>" + String(accounts[i].name) + "</h2>";
    html += "<label>Name</label>";
    html += "<input type='text' name='n" + String(i) + "' value=\"";
    html += htmlEscape(accounts[i].name);
    html += "\" maxlength='23' required>";
    html += "<label>PIN</label>";
    html += "<input type='text' name='p" + String(i) + "' value=\"";
    html += htmlEscape(accounts[i].pin);
    html += "\" maxlength='7' pattern='[0-9]*' required>";
    html += "<div class='row'>";
    html += "<div><label>Balance (GBP)</label>";
    html += "<input type='number' name='b" + String(i) + "' step='0.01' min='0' value='";
    html += poundsInput(accounts[i].balance);
    html += "'></div>";
    html += "<div><label>Savings Goal (GBP)</label>";
    html += "<input type='number' name='g" + String(i) + "' step='0.01' min='0' value='";
    html += poundsInput(accounts[i].goalTarget);
    html += "'></div></div>";
    html += "<label>XP</label>";
    html += "<input type='number' name='x" + String(i) + "' step='1' min='0' value='";
    html += String(accounts[i].xp);
    html += "'>";
    html += "</div>";
  }

  html += "</div><button type='submit'>Save All Accounts</button></form></body></html>";
  return html;
}

static String buildApprovalsPage(bool acted) {
  String html = buildPageHeader("/approvals");
  html += buildStatusPanel();
  if (acted) html += "<div class='ok'>&#10003; Approval queue updated</div>";

  html += "<div class='scroll'>";
  int pending = 0;

  for (int i = 0; i < CHORE_COUNT; i++) {
    if (chores[i].pendingChild < 0) continue;
    pending++;

    html += "<div class='item'>";
    html += "<strong>" + htmlEscape(accounts[chores[i].pendingChild].name) + "</strong><br>";
    html += htmlEscape(chores[i].title) + "<br>";
    html += "<span class='warn'>" + modelMoneyText(chores[i].reward) + "</span>";
    html += "<div class='actions'>";
    html += "<form method='POST' action='/approvals/approve' style='display:inline'>";
    html += "<input type='hidden' name='c' value='" + String(i) + "'>";
    html += "<button type='submit' class='btn good'>Approve</button></form>";
    html += "<form method='POST' action='/approvals/reject' style='display:inline'>";
    html += "<input type='hidden' name='c' value='" + String(i) + "'>";
    html += "<button type='submit' class='btn danger'>Try Again</button></form>";
    html += "</div></div>";
  }

  if (pending == 0) {
    html += "<div class='panel'>No chores waiting for approval.</div>";
  }

  html += "</div></body></html>";
  return html;
}

static String buildWifiPage(bool saved, bool cleared) {
  char ssid[33] = {0};
  modelWifiGetCredentials(ssid, sizeof(ssid), nullptr, 0);

  String html = buildPageHeader("/wifi");
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
        long pennies = parsePounds(webServer.arg(rk), 450L);
        if (pennies >= 0) chores[i].reward = pennies;
      }
    }

    modelSaveChores();
    webServer.sendHeader("Location", "/?saved=1");
    webServer.send(302, "text/plain", "");
  });

  webServer.on("/rewards", HTTP_GET, []() {
    webServer.send(200, "text/html", buildRewardsPage(webServer.hasArg("saved")));
  });

  webServer.on("/rewards/update", HTTP_POST, []() {
    for (int i = 0; i < REWARD_COUNT; i++) {
      String nk = "n" + String(i);
      String ck = "c" + String(i);

      if (webServer.hasArg(nk)) {
        String val = webServer.arg(nk);
        val.trim();
        if (val.length() > 0 && val.length() < 32) {
          strncpy(rewards[i].title, val.c_str(), 31);
          rewards[i].title[31] = '\0';
        }
      }

      if (webServer.hasArg(ck)) {
        long pennies = parsePounds(webServer.arg(ck), 99999L);
        if (pennies >= 0) rewards[i].cost = pennies;
      }
    }

    modelSaveRewards();
    webServer.sendHeader("Location", "/rewards?saved=1");
    webServer.send(302, "text/plain", "");
  });

  webServer.on("/accounts", HTTP_GET, []() {
    webServer.send(200, "text/html", buildAccountsPage(webServer.hasArg("saved")));
  });

  webServer.on("/accounts/update", HTTP_POST, []() {
    for (int i = 0; i < ACCOUNT_COUNT; i++) {
      String nk = "n" + String(i);
      String pk = "p" + String(i);
      String bk = "b" + String(i);
      String gk = "g" + String(i);
      String xk = "x" + String(i);

      if (webServer.hasArg(nk)) {
        String val = webServer.arg(nk);
        val.trim();
        if (val.length() > 0 && val.length() < 24) {
          strncpy(accounts[i].name, val.c_str(), 23);
          accounts[i].name[23] = '\0';
        }
      }

      if (webServer.hasArg(pk)) {
        String val = webServer.arg(pk);
        val.trim();
        if (val.length() > 0 && val.length() < 8) {
          strncpy(accounts[i].pin, val.c_str(), 7);
          accounts[i].pin[7] = '\0';
        }
      }

      if (webServer.hasArg(bk)) {
        long pennies = parsePounds(webServer.arg(bk));
        if (pennies >= 0) accounts[i].balance = pennies;
      }

      if (webServer.hasArg(gk)) {
        long pennies = parsePounds(webServer.arg(gk));
        if (pennies >= 0) accounts[i].goalTarget = pennies;
      }

      if (webServer.hasArg(xk)) {
        long xp = webServer.arg(xk).toInt();
        if (xp >= 0) accounts[i].xp = xp;
      }

      modelUpdateAchievements(i);
      modelSaveAccount(i);
    }

    webServer.sendHeader("Location", "/accounts?saved=1");
    webServer.send(302, "text/plain", "");
  });

  webServer.on("/approvals", HTTP_GET, []() {
    webServer.send(200, "text/html", buildApprovalsPage(webServer.hasArg("acted")));
  });

  webServer.on("/approvals/approve", HTTP_POST, []() {
    if (webServer.hasArg("c")) {
      modelApproveChore(webServer.arg("c").toInt());
    }
    webServer.sendHeader("Location", "/approvals?acted=1");
    webServer.send(302, "text/plain", "");
  });

  webServer.on("/approvals/reject", HTTP_POST, []() {
    if (webServer.hasArg("c")) {
      modelRejectChore(webServer.arg("c").toInt());
    }
    webServer.sendHeader("Location", "/approvals?acted=1");
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
  prefs.putString(("nm" + k).c_str(), accounts[i].name);
  prefs.putString(("pn" + k).c_str(), accounts[i].pin);
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
    String name = prefs.getString(("nm" + k).c_str(), accounts[i].name);
    String pin = prefs.getString(("pn" + k).c_str(), accounts[i].pin);
    name.toCharArray(accounts[i].name, sizeof(accounts[i].name));
    pin.toCharArray(accounts[i].pin, sizeof(accounts[i].pin));
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

void modelSaveRewards() {
  prefs.begin("bank", false);
  for (int i = 0; i < REWARD_COUNT; i++) {
    String k = String(i);
    prefs.putString(("rt" + k).c_str(), rewards[i].title);
    prefs.putLong(("rc" + k).c_str(), rewards[i].cost);
  }
  prefs.end();
}

void modelLoadRewards() {
  prefs.begin("bank", true);
  for (int i = 0; i < REWARD_COUNT; i++) {
    String k = String(i);
    String title = prefs.getString(("rt" + k).c_str(), DEFAULT_REWARD_TITLES[i]);
    title.toCharArray(rewards[i].title, sizeof(rewards[i].title));
    rewards[i].cost = prefs.getLong(("rc" + k).c_str(), DEFAULT_REWARD_COSTS[i]);
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

bool modelRejectChore(int chore) {
  if (chore < 0 || chore >= CHORE_COUNT) return false;
  if (chores[chore].pendingChild < 0) return false;
  chores[chore].pendingChild = -1;
  modelSaveChores();
  modelPushNotification("REJECT", chores[chore].title);
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
  modelLoadRewards();

  setupWebRoutes();
  webServer.begin();
  s_webStarted = true;

  Serial.println("[Web] Admin panel: http://192.168.4.1/");
  Serial.println("[Web] Chores, shop, accounts, approvals, WiFi all editable from phone");
}

void modelWebServe() {
  if (s_webStarted) webServer.handleClient();
}

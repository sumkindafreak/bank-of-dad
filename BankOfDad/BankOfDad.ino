#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "XPT2046_Touchscreen.h"
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>

// ==========================
// DISPLAY PINS - YOUR WORKING SETUP
// ==========================
#define TFT_CS   15
#define TFT_DC   2
#define TFT_MOSI 13
#define TFT_MISO 12
#define TFT_SCK  14

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// ==========================
// TOUCH PINS - CYD STYLE
// ==========================
#define TOUCH_CS   33
#define TOUCH_CLK  25
#define TOUCH_MOSI 32
#define TOUCH_MISO 39
#define TOUCH_IRQ  36

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// ==========================
// RGB LED PINS
// CYD RGB LEDs are active LOW
// ==========================
#define LED_R 4
#define LED_G 16
#define LED_B 17

// ==========================
// SCREEN SIZE
// ==========================
#define SCREEN_W 240
#define SCREEN_H 320

// ==========================
// WEB ADMIN
// ESP32 creates its own hotspot. Connect phone/laptop to the SSID
// below then open http://192.168.4.1 to edit chores.
// Change WIFI_AP_PASS to any 8+ character string you prefer.
// ==========================
#define WIFI_AP_SSID "BankOfDad"
#define WIFI_AP_PASS "pocket123"

// ==========================
// TOUCH CALIBRATION
// Adjust MIN/MAX if touches feel offset on your panel.
// Print raw p.x / p.y values from getTouch() to find your edges.
// ==========================
#define TOUCH_CAL_MIN_X   200
#define TOUCH_CAL_MAX_X  3800
#define TOUCH_CAL_MIN_Y   200
#define TOUCH_CAL_MAX_Y  3800
#define TOUCH_DEBOUNCE_MS 180   // minimum ms between accepted touches

// ==========================
// SIMPLE COLOURS
// ==========================
#define COL_BG       ILI9341_BLACK
#define COL_PANEL    ILI9341_DARKGREY
#define COL_TEXT     ILI9341_WHITE
#define COL_GOOD     ILI9341_GREEN
#define COL_BAD      ILI9341_RED
#define COL_WARN     ILI9341_YELLOW
#define COL_BLUE     ILI9341_BLUE
#define COL_CYAN     ILI9341_CYAN

// ==========================
// DATA LIMITS
// ==========================
#define ACCOUNT_COUNT   4
#define HISTORY_COUNT   6
#define CHORE_COUNT     4
#define DAD_TAX_PERCENT 10  // % deducted from balance when Dad Tax is applied

// ==========================
// ACCOUNT DATA
// Money is stored in pennies
// ==========================
struct Account {
  const char* name;
  const char* pin;
  long balance;
  long goalTarget;
  long xp;
  bool firstDeposit;
  bool firstTen;
  bool firstTwentyFive;
};

Account accounts[ACCOUNT_COUNT] = {
  {"Nixon",    "1111", 1250, 5000,  40, false, true,  false},
  {"Oakley",   "2222", 800,  3000,  20, false, false, false},
  {"Hadley",   "3333", 1575, 4500,  65, false, true,  false},
  {"Maddison", "4444", 2200, 6000, 120, false, true,  false}
};

// ==========================
// TRANSACTION HISTORY
// ==========================
String historyLog[ACCOUNT_COUNT][HISTORY_COUNT];
int historyIndex[ACCOUNT_COUNT] = {0, 0, 0, 0};

// ==========================
// CHORES
// pendingChild = -1 means available
// pendingChild >= 0 means waiting for Dad approval
// ==========================
struct Chore {
  char title[32];   // mutable so the web editor can update it
  long reward;
  int  pendingChild;
};

Chore chores[CHORE_COUNT] = {
  {"Feed Pet", 100, -1},
  {"Tidy Room", 200, -1},
  {"Vacuum", 300, -1},
  {"Garden Help", 500, -1}
};

// ==========================
// BUTTON STRUCT
// ==========================
struct Button {
  int x;
  int y;
  int w;
  int h;
  String label;
};

// ==========================
// APP SCREENS
// ==========================
enum Screen {
  HOME,
  PIN,
  ACCOUNT,
  HISTORY,
  GOALS,
  CHORES,
  ACHIEVEMENTS,
  LEADERBOARD,
  ADMIN_PIN,
  ADMIN_MENU,
  ADMIN_SELECT,
  ADMIN_ACTION,
  AMOUNT,
  APPROVALS,
  DAD_TAX,
  MESSAGE
};

Screen currentScreen = HOME;

int selectedAccount = -1;
String enteredPin = "";
String amountInput = "";
bool adminMode = false;
bool depositMode = true;

Preferences      prefs;
unsigned long    lastTouchMs = 0;
WebServer        webServer(80);

// ==========================
// RGB CONTROL
// ==========================
void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_R, r ? LOW : HIGH);
  digitalWrite(LED_G, g ? LOW : HIGH);
  digitalWrite(LED_B, b ? LOW : HIGH);
}

// ==========================
// MONEY TEXT
// ==========================
String moneyText(long pennies) {
  long pounds = pennies / 100;
  int pence = abs(pennies % 100);

  String text = "GBP ";
  text += pounds;
  text += ".";
  if (pence < 10) text += "0";
  text += pence;

  return text;
}

// ==========================
// HISTORY ADD
// ==========================
void addHistory(int account, String entry) {
  historyLog[account][historyIndex[account]] = entry;
  historyIndex[account]++;
  if (historyIndex[account] >= HISTORY_COUNT) historyIndex[account] = 0;

  Serial.print("History added for ");
  Serial.print(accounts[account].name);
  Serial.print(": ");
  Serial.println(entry);
}

// ==========================
// NVS PERSISTENCE
// Keys are kept short (<= 15 chars) as required by Preferences.
// Each account uses its index as a suffix: "bal0", "xp1", etc.
// ==========================
void saveAccount(int i) {
  prefs.begin("bank", false);
  String k = String(i);
  prefs.putLong(("bal" + k).c_str(), accounts[i].balance);
  prefs.putLong(("xp"  + k).c_str(), accounts[i].xp);
  prefs.putBool(("fd"  + k).c_str(), accounts[i].firstDeposit);
  prefs.putBool(("f10" + k).c_str(), accounts[i].firstTen);
  prefs.putBool(("f25" + k).c_str(), accounts[i].firstTwentyFive);
  prefs.end();
}

void loadAccounts() {
  prefs.begin("bank", true);
  for (int i = 0; i < ACCOUNT_COUNT; i++) {
    String k = String(i);
    accounts[i].balance         = prefs.getLong(("bal" + k).c_str(), accounts[i].balance);
    accounts[i].xp              = prefs.getLong(("xp"  + k).c_str(), accounts[i].xp);
    accounts[i].firstDeposit    = prefs.getBool(("fd"  + k).c_str(), accounts[i].firstDeposit);
    accounts[i].firstTen        = prefs.getBool(("f10" + k).c_str(), accounts[i].firstTen);
    accounts[i].firstTwentyFive = prefs.getBool(("f25" + k).c_str(), accounts[i].firstTwentyFive);
  }
  prefs.end();
}

void saveChores() {
  prefs.begin("bank", false);
  for (int i = 0; i < CHORE_COUNT; i++) {
    String k = String(i);
    prefs.putString(("ct" + k).c_str(), chores[i].title);
    prefs.putLong  (("cr" + k).c_str(), chores[i].reward);
  }
  prefs.end();
}

void loadChores() {
  prefs.begin("bank", true);
  for (int i = 0; i < CHORE_COUNT; i++) {
    String k     = String(i);
    String title = prefs.getString(("ct" + k).c_str(), chores[i].title);
    title.toCharArray(chores[i].title, 32);
    chores[i].reward = prefs.getLong(("cr" + k).c_str(), chores[i].reward);
  }
  prefs.end();
}

// ==========================
// ACHIEVEMENT CHECK
// ==========================
void updateAchievements(int account) {
  if (accounts[account].balance >= 1000) accounts[account].firstTen = true;
  if (accounts[account].balance >= 2500) accounts[account].firstTwentyFive = true;
}

// ==========================
// BUTTON HELPERS
// ==========================
bool inside(Button b, int x, int y) {
  return x >= b.x && x <= b.x + b.w && y >= b.y && y <= b.y + b.h;
}

void drawButton(Button b, uint16_t colour) {
  tft.fillRoundRect(b.x, b.y, b.w, b.h, 7, colour);
  tft.drawRoundRect(b.x, b.y, b.w, b.h, 7, COL_TEXT);

  tft.setTextColor(COL_TEXT);
  tft.setTextSize(2);

  int16_t x1, y1;
  uint16_t tw, th;
  tft.getTextBounds(b.label, 0, 0, &x1, &y1, &tw, &th);

  tft.setCursor(b.x + (b.w - tw) / 2, b.y + (b.h - th) / 2);
  tft.print(b.label);
}

// ==========================
// HEADER
// ==========================
void drawHeader(const char* title) {
  tft.fillScreen(COL_BG);
  tft.fillRect(0, 0, SCREEN_W, 38, COL_PANEL);

  tft.setTextColor(COL_WARN);
  tft.setTextSize(2);
  tft.setCursor(10, 11);
  tft.print(title);
}

// ==========================
// HOME
// ==========================
void drawHome() {
  currentScreen = HOME;
  selectedAccount = -1;
  adminMode = false;
  enteredPin = "";
  setRGB(false, false, true);

  drawHeader("BANK OF DAD");

  tft.setTextColor(COL_TEXT);
  tft.setTextSize(2);
  tft.setCursor(26, 52);
  tft.print("Choose Account");

  for (int i = 0; i < ACCOUNT_COUNT; i++) {
    Button b = {20, 80 + i * 42, 200, 34, accounts[i].name};
    drawButton(b, COL_PANEL);
  }

  Button admin = {20, 270, 200, 36, "DAD ADMIN"};
  drawButton(admin, COL_BAD);
}

// ==========================
// KEYPAD
// ==========================
void drawKeypad() {
  String keys[12] = {
    "1", "2", "3",
    "4", "5", "6",
    "7", "8", "9",
    "*", "0", "#"
  };

  int startX = 30;
  int startY = 105;
  int bw = 50;
  int bh = 38;
  int gap = 10;

  for (int i = 0; i < 12; i++) {
    int col = i % 3;
    int row = i / 3;

    Button b = {
      startX + col * (bw + gap),
      startY + row * (bh + gap),
      bw,
      bh,
      keys[i]
    };

    drawButton(b, COL_PANEL);
  }

  tft.setTextColor(ILI9341_LIGHTGREY);
  tft.setTextSize(1);
  tft.setCursor(20, 300);
  tft.print("* Back/Clear     # Enter");
}

void updatePinDisplay() {
  tft.fillRect(35, 58, 175, 30, COL_BG);

  tft.setTextColor(COL_TEXT);
  tft.setTextSize(2);
  tft.setCursor(45, 65);
  tft.print("PIN: ");

  for (int i = 0; i < 4; i++) {
    if (i < (int)enteredPin.length()) tft.print("*");
    else tft.print("_");
  }
}

void drawPin(bool admin) {
  adminMode = admin;
  enteredPin = "";
  currentScreen = admin ? ADMIN_PIN : PIN;

  drawHeader(admin ? "ADMIN PIN" : "ENTER PIN");
  updatePinDisplay();
  drawKeypad();
}

// ==========================
// ACCOUNT SCREEN
// ==========================
void drawAccount() {
  currentScreen = ACCOUNT;
  setRGB(false, true, false);

  drawHeader(accounts[selectedAccount].name);

  tft.setTextColor(COL_TEXT);
  tft.setTextSize(2);
  tft.setCursor(30, 55);
  tft.print("Balance");

  tft.setTextColor(COL_GOOD);
  tft.setTextSize(3);
  tft.setCursor(25, 84);
  tft.print(moneyText(accounts[selectedAccount].balance));

  int level = accounts[selectedAccount].xp / 100 + 1;
  tft.setTextColor(COL_CYAN);
  tft.setTextSize(1);
  tft.setCursor(25, 124);
  tft.print("Level ");
  tft.print(level);
  tft.print(" Saver   XP ");
  tft.print(accounts[selectedAccount].xp);

  drawButton({15, 150, 100, 34, "HISTORY"}, COL_PANEL);
  drawButton({125, 150, 100, 34, "GOALS"}, COL_PANEL);
  drawButton({15, 195, 100, 34, "CHORES"}, COL_PANEL);
  drawButton({125, 195, 100, 34, "BADGES"}, COL_PANEL);
  drawButton({15, 240, 210, 34, "LEADERBOARD"}, COL_BLUE);
  drawButton({15, 280, 210, 30, "LOGOUT"}, COL_BAD);
}

// ==========================
// HISTORY SCREEN
// ==========================
void drawHistory() {
  currentScreen = HISTORY;
  drawHeader("HISTORY");

  tft.setTextColor(COL_TEXT);
  tft.setTextSize(1);
  tft.setCursor(15, 50);
  tft.print(accounts[selectedAccount].name);
  tft.print(" recent activity");

  int y = 75;
  for (int i = 0; i < HISTORY_COUNT; i++) {
    int idx = (historyIndex[selectedAccount] + i) % HISTORY_COUNT;
    if (historyLog[selectedAccount][idx].length() > 0) {
      tft.setCursor(12, y);
      tft.print(historyLog[selectedAccount][idx]);
      y += 28;
    }
  }

  if (y == 75) {
    tft.setTextSize(2);
    tft.setCursor(30, 130);
    tft.print("No history yet");
  }

  drawButton({20, 270, 200, 36, "BACK"}, COL_BAD);
}

// ==========================
// GOALS SCREEN
// ==========================
void drawGoals() {
  currentScreen = GOALS;
  drawHeader("SAVINGS GOAL");

  long balance = accounts[selectedAccount].balance;
  long target = accounts[selectedAccount].goalTarget;
  int percent = (balance * 100) / target;
  if (percent > 100) percent = 100;

  tft.setTextColor(COL_TEXT);
  tft.setTextSize(2);
  tft.setCursor(25, 60);
  tft.print(accounts[selectedAccount].name);

  tft.setTextSize(1);
  tft.setCursor(25, 100);
  tft.print("Target: ");
  tft.print(moneyText(target));

  tft.setCursor(25, 120);
  tft.print("Saved:  ");
  tft.print(moneyText(balance));

  tft.drawRect(20, 155, 200, 28, COL_TEXT);
  tft.fillRect(22, 157, (196 * percent) / 100, 24, COL_GOOD);

  tft.setTextColor(COL_WARN);
  tft.setTextSize(2);
  tft.setCursor(80, 200);
  tft.print(percent);
  tft.print("%");

  drawButton({20, 270, 200, 36, "BACK"}, COL_BAD);
}

// ==========================
// CHORES SCREEN
// ==========================
void drawChores() {
  currentScreen = CHORES;
  drawHeader("CHORES");

  tft.setTextColor(COL_TEXT);
  tft.setTextSize(1);
  tft.setCursor(20, 50);
  tft.print("Tap a job to request it");

  for (int i = 0; i < CHORE_COUNT; i++) {
    String label = String(chores[i].title) + " " + moneyText(chores[i].reward);

    uint16_t colour = COL_PANEL;
    if (chores[i].pendingChild == selectedAccount) {
      label = "PENDING";
      colour = COL_WARN;
    } else if (chores[i].pendingChild >= 0) {
      label = "TAKEN";
      colour = COL_BAD;
    }

    drawButton({15, 78 + i * 42, 210, 34, label}, colour);
  }

  drawButton({20, 270, 200, 36, "BACK"}, COL_BAD);
}

// ==========================
// ACHIEVEMENTS
// ==========================
void drawAchievements() {
  currentScreen = ACHIEVEMENTS;
  updateAchievements(selectedAccount);

  drawHeader("BADGES");

  tft.setTextColor(COL_TEXT);
  tft.setTextSize(2);

  tft.setCursor(25, 70);
  tft.print(accounts[selectedAccount].firstDeposit ? "[X] " : "[ ] ");
  tft.print("First Deposit");

  tft.setCursor(25, 115);
  tft.print(accounts[selectedAccount].firstTen ? "[X] " : "[ ] ");
  tft.print("Save GBP10");

  tft.setCursor(25, 160);
  tft.print(accounts[selectedAccount].firstTwentyFive ? "[X] " : "[ ] ");
  tft.print("Save GBP25");

  tft.setCursor(25, 205);
  tft.print(accounts[selectedAccount].xp >= 100 ? "[X] " : "[ ] ");
  tft.print("Level 2");

  drawButton({20, 270, 200, 36, "BACK"}, COL_BAD);
}

// ==========================
// LEADERBOARD
// ==========================
void drawLeaderboard() {
  currentScreen = LEADERBOARD;
  drawHeader("TOP SAVERS");

  int order[ACCOUNT_COUNT] = {0, 1, 2, 3};

  for (int i = 0; i < ACCOUNT_COUNT - 1; i++) {
    for (int j = i + 1; j < ACCOUNT_COUNT; j++) {
      if (accounts[order[j]].balance > accounts[order[i]].balance) {
        int temp = order[i];
        order[i] = order[j];
        order[j] = temp;
      }
    }
  }

  tft.setTextColor(COL_TEXT);
  tft.setTextSize(2);

  for (int i = 0; i < ACCOUNT_COUNT; i++) {
    int a = order[i];
    tft.setCursor(20, 70 + i * 42);
    tft.print(i + 1);
    tft.print(". ");
    tft.print(accounts[a].name);

    tft.setTextSize(1);
    tft.setCursor(135, 76 + i * 42);
    tft.print(moneyText(accounts[a].balance));
    tft.setTextSize(2);
  }

  drawButton({20, 270, 200, 36, "BACK"}, COL_BAD);
}

// ==========================
// WEB ADMIN — Chore Editor
// Served at http://192.168.4.1 on the "BankOfDad" AP.
// Rewards are submitted in GBP (e.g. 1.00) and stored in pennies.
// ==========================
String buildChorePage(bool saved) {
  String html =
    "<!DOCTYPE html><html><head>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>Bank of Dad</title><style>"
    "body{font-family:sans-serif;max-width:480px;margin:0 auto;padding:16px;"
      "background:#111;color:#eee}"
    "h1{color:#e74c3c;margin:0 0 2px}"
    "p.sub{color:#888;margin:0 0 16px;font-size:.9em}"
    ".chore{background:#222;padding:12px;margin:8px 0;border-radius:8px}"
    "label{display:block;margin-bottom:4px;font-size:.85em;color:#aaa}"
    "input[type=text],input[type=number]{"
      "width:100%;box-sizing:border-box;padding:8px;"
      "background:#333;color:#fff;border:1px solid #555;"
      "border-radius:4px;margin-bottom:8px}"
    "button{width:100%;padding:12px;background:#e74c3c;color:#fff;"
      "border:none;border-radius:8px;font-size:1em;cursor:pointer;margin-top:8px}"
    "button:hover{background:#c0392b}"
    ".ok{padding:10px;background:#27ae60;border-radius:6px;"
      "margin-bottom:12px;text-align:center}"
    ".warn{font-size:.8em;color:#e67e22;margin-top:4px}"
    "</style></head><body>"
    "<h1>Bank of Dad</h1>"
    "<p class='sub'>Chore Editor &mdash; changes take effect immediately</p>";

  if (saved) html += "<div class='ok'>&#10003; Changes saved!</div>";

  html += "<form method='POST' action='/update'>";

  for (int i = 0; i < CHORE_COUNT; i++) {
    char rewardBuf[12];
    dtostrf(chores[i].reward / 100.0f, 1, 2, rewardBuf);

    String pending = "";
    if (chores[i].pendingChild >= 0)
      pending = "<p class='warn'>&#9888; Pending approval for "
                + String(accounts[chores[i].pendingChild].name) + "</p>";

    html += "<div class='chore'>";
    html += "<label>Chore " + String(i + 1) + " &mdash; Name</label>";
    html += "<input type='text' name='t" + String(i) + "' value=\"";
    html += String(chores[i].title);
    html += "\" maxlength='31' required>";
    html += "<label>Reward (GBP)</label>";
    html += "<input type='number' name='r" + String(i) + "' step='0.01' min='0' value='";
    html += String(rewardBuf);
    html += "'>";
    html += pending;
    html += "</div>";
  }

  html += "<button type='submit'>Save Chores</button></form>"
          "</body></html>";

  return html;
}

void setupWebServer() {
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
        if (pounds >= 0.0f)
          chores[i].reward = (long)(pounds * 100.0f + 0.5f);
      }
    }

    saveChores();

    webServer.sendHeader("Location", "/?saved=1");
    webServer.send(302, "text/plain", "");
  });

  webServer.onNotFound([]() {
    webServer.sendHeader("Location", "/");
    webServer.send(302, "text/plain", "");
  });
}

// ==========================
// ADMIN SCREENS
// ==========================
void drawAdminMenu() {
  currentScreen = ADMIN_MENU;
  setRGB(true, true, false);

  drawHeader("DAD ADMIN");

  // Show the AP hotspot name and URL so Dad knows how to open the web editor
  tft.setTextColor(ILI9341_LIGHTGREY);
  tft.setTextSize(1);
  tft.setCursor(15, 47);
  tft.print("Web: ");
  tft.print(WIFI_AP_SSID);
  tft.print(" > 192.168.4.1");

  drawButton({20, 70, 200, 42, "EDIT MONEY"}, COL_GOOD);
  drawButton({20, 130, 200, 42, "APPROVALS"}, COL_WARN);
  drawButton({20, 190, 200, 42, "DAD TAX"}, COL_BAD);
  drawButton({20, 260, 200, 36, "HOME"}, COL_PANEL);
}

void drawAdminSelect() {
  currentScreen = ADMIN_SELECT;
  drawHeader("SELECT CHILD");

  for (int i = 0; i < ACCOUNT_COUNT; i++) {
    drawButton({20, 70 + i * 42, 200, 34, accounts[i].name}, COL_PANEL);
  }

  drawButton({20, 270, 200, 36, "BACK"}, COL_BAD);
}

void drawAdminAction() {
  currentScreen = ADMIN_ACTION;
  drawHeader(accounts[selectedAccount].name);

  tft.setTextColor(COL_GOOD);
  tft.setTextSize(2);
  tft.setCursor(30, 58);
  tft.print(moneyText(accounts[selectedAccount].balance));

  drawButton({20, 100, 200, 42, "DEPOSIT"}, COL_GOOD);
  drawButton({20, 155, 200, 42, "WITHDRAW"}, COL_BAD);
  drawButton({20, 215, 200, 42, "DAD TAX"}, COL_WARN);
  drawButton({20, 270, 200, 36, "BACK"}, COL_PANEL);
}

void drawApprovals() {
  currentScreen = APPROVALS;
  drawHeader("APPROVALS");

  bool found = false;
  int y = 65;

  for (int i = 0; i < CHORE_COUNT; i++) {
    if (chores[i].pendingChild >= 0) {
      found = true;
      String label = String(accounts[chores[i].pendingChild].name) + " " + moneyText(chores[i].reward);
      drawButton({15, y, 210, 34, label}, COL_GOOD);
      y += 45;
    }
  }

  if (!found) {
    tft.setTextColor(COL_TEXT);
    tft.setTextSize(2);
    tft.setCursor(32, 130);
    tft.print("No approvals");
  }

  drawButton({20, 270, 200, 36, "BACK"}, COL_BAD);
}

// ==========================
// DAD TAX SCREEN
// Shows the calculated tax and asks for confirmation before applying.
// ==========================
void drawDadTax() {
  currentScreen = DAD_TAX;
  drawHeader("DAD TAX");

  long tax = (accounts[selectedAccount].balance * DAD_TAX_PERCENT) / 100;

  tft.setTextColor(COL_TEXT);
  tft.setTextSize(2);
  tft.setCursor(25, 55);
  tft.print(accounts[selectedAccount].name);

  tft.setTextSize(1);
  tft.setTextColor(COL_TEXT);
  tft.setCursor(25, 95);
  tft.print("Balance: ");
  tft.print(moneyText(accounts[selectedAccount].balance));

  tft.setTextColor(COL_BAD);
  tft.setCursor(25, 120);
  tft.print("Tax (");
  tft.print(DAD_TAX_PERCENT);
  tft.print("%):  -");
  tft.print(moneyText(tax));

  tft.setTextColor(COL_GOOD);
  tft.setCursor(25, 145);
  tft.print("After:   ");
  tft.print(moneyText(accounts[selectedAccount].balance - tax));

  if (tax == 0) {
    tft.setTextColor(COL_WARN);
    tft.setTextSize(2);
    tft.setCursor(30, 195);
    tft.print("Nothing to tax!");
    drawButton({20, 270, 200, 36, "BACK"}, COL_PANEL);
  } else {
    drawButton({20, 185, 200, 42, "APPLY TAX"}, COL_BAD);
    drawButton({20, 240, 200, 36, "CANCEL"}, COL_PANEL);
  }
}

// ==========================
// AMOUNT SCREEN
// ==========================
void updateAmountDisplay() {
  tft.fillRect(20, 58, 210, 32, COL_BG);

  long pennies = amountInput.toInt();

  tft.setTextColor(COL_TEXT);
  tft.setTextSize(2);
  tft.setCursor(25, 65);
  tft.print("Amount ");
  tft.print(moneyText(pennies));
}

void drawAmount(bool deposit) {
  currentScreen = AMOUNT;
  depositMode = deposit;
  amountInput = "";

  drawHeader(deposit ? "DEPOSIT" : "WITHDRAW");
  updateAmountDisplay();
  drawKeypad();
}

// ==========================
// MESSAGE SCREEN
// ==========================
void drawMessage(String title, String line1, String line2, bool error) {
  currentScreen = MESSAGE;

  if (error) setRGB(true, false, false);
  else setRGB(false, true, false);

  drawHeader(error ? "ERROR" : "COMPLETE");

  tft.setTextColor(error ? COL_BAD : COL_GOOD);
  tft.setTextSize(2);
  tft.setCursor(25, 70);
  tft.print(title);

  tft.setTextColor(COL_TEXT);
  tft.setCursor(25, 120);
  tft.print(line1);

  tft.setCursor(25, 160);
  tft.print(line2);

  drawButton({20, 250, 200, 42, "OK"}, error ? COL_BAD : COL_GOOD);
}

// ==========================
// TOUCH READ
// Non-blocking: rejects events closer than TOUCH_DEBOUNCE_MS apart
// so the CPU is never stalled by delay().
// ==========================
bool getTouch(int &x, int &y) {
  if (!touch.touched()) return false;

  unsigned long now = millis();
  if (now - lastTouchMs < TOUCH_DEBOUNCE_MS) return false;

  TS_Point p = touch.getPoint();

  x = map(p.x, TOUCH_CAL_MIN_X, TOUCH_CAL_MAX_X, 0, SCREEN_W);
  y = map(p.y, TOUCH_CAL_MIN_Y, TOUCH_CAL_MAX_Y, 0, SCREEN_H);

  x = constrain(x, 0, SCREEN_W);
  y = constrain(y, 0, SCREEN_H);

  Serial.print("Touch X=");
  Serial.print(x);
  Serial.print(" Y=");
  Serial.println(y);

  lastTouchMs = now;
  return true;
}

String keypadKey(int x, int y) {
  String keys[12] = {
    "1", "2", "3",
    "4", "5", "6",
    "7", "8", "9",
    "*", "0", "#"
  };

  int startX = 30;
  int startY = 105;
  int bw = 50;
  int bh = 38;
  int gap = 10;

  for (int i = 0; i < 12; i++) {
    int col = i % 3;
    int row = i / 3;

    Button b = {
      startX + col * (bw + gap),
      startY + row * (bh + gap),
      bw,
      bh,
      keys[i]
    };

    if (inside(b, x, y)) return keys[i];
  }

  return "";
}

// ==========================
// HANDLE PIN
// ==========================
void handlePin(String key) {
  if (key == "*") {
    drawHome();
    return;
  }

  if (key == "#") {
    if (currentScreen == ADMIN_PIN) {
      if (enteredPin == "9999") drawAdminMenu();
      else drawMessage("WRONG PIN", "Try again", "", true);
    } else {
      if (selectedAccount >= 0 && enteredPin == accounts[selectedAccount].pin) {
        drawAccount();
      } else {
        drawMessage("WRONG PIN", "Try again", "", true);
      }
    }
    return;
  }

  if (enteredPin.length() < 4) {
    enteredPin += key;
    updatePinDisplay();
  }
}

// ==========================
// HANDLE AMOUNT
// ==========================
void handleAmount(String key) {
  if (key == "*") {
    drawAdminAction();
    return;
  }

  if (key == "#") {
    long pennies = amountInput.toInt();

    if (pennies <= 0) {
      drawMessage("NO AMOUNT", "Enter value", "", true);
      return;
    }

    if (!depositMode && accounts[selectedAccount].balance < pennies) {
      drawMessage("NO FUNDS", "Not enough", "", true);
      return;
    }

    if (depositMode) {
      accounts[selectedAccount].balance += pennies;
      accounts[selectedAccount].xp += 10;
      accounts[selectedAccount].firstDeposit = true;
      addHistory(selectedAccount, "+" + moneyText(pennies) + " Admin");
      updateAchievements(selectedAccount);
      saveAccount(selectedAccount);
      drawMessage("DEPOSIT OK", "+" + moneyText(pennies), "Bal " + moneyText(accounts[selectedAccount].balance), false);
    } else {
      accounts[selectedAccount].balance -= pennies;
      addHistory(selectedAccount, "-" + moneyText(pennies) + " Admin");
      updateAchievements(selectedAccount);
      saveAccount(selectedAccount);
      drawMessage("WITHDRAW OK", "-" + moneyText(pennies), "Bal " + moneyText(accounts[selectedAccount].balance), false);
    }

    return;
  }

  if (amountInput.length() < 6) {
    amountInput += key;
    updateAmountDisplay();
  }
}

// ==========================
// MAIN TOUCH HANDLER
// ==========================
void handleTouch(int x, int y) {
  if (currentScreen == HOME) {
    for (int i = 0; i < ACCOUNT_COUNT; i++) {
      if (inside({20, 80 + i * 42, 200, 34, ""}, x, y)) {
        selectedAccount = i;
        drawPin(false);
        return;
      }
    }

    if (inside({20, 270, 200, 36, ""}, x, y)) {
      drawPin(true);
      return;
    }
  }

  else if (currentScreen == PIN || currentScreen == ADMIN_PIN) {
    String key = keypadKey(x, y);
    if (key != "") handlePin(key);
  }

  else if (currentScreen == ACCOUNT) {
    if (inside({15, 150, 100, 34, ""}, x, y)) drawHistory();
    else if (inside({125, 150, 100, 34, ""}, x, y)) drawGoals();
    else if (inside({15, 195, 100, 34, ""}, x, y)) drawChores();
    else if (inside({125, 195, 100, 34, ""}, x, y)) drawAchievements();
    else if (inside({15, 240, 210, 34, ""}, x, y)) drawLeaderboard();
    else if (inside({15, 280, 210, 30, ""}, x, y)) drawHome();
  }

  else if (currentScreen == HISTORY || currentScreen == GOALS ||
           currentScreen == ACHIEVEMENTS || currentScreen == LEADERBOARD) {
    if (inside({20, 270, 200, 36, ""}, x, y)) drawAccount();
  }

  else if (currentScreen == CHORES) {
    for (int i = 0; i < CHORE_COUNT; i++) {
      if (inside({15, 78 + i * 42, 210, 34, ""}, x, y)) {
        if (chores[i].pendingChild == -1) {
          chores[i].pendingChild = selectedAccount;
          addHistory(selectedAccount, "Requested " + String(chores[i].title));
          drawMessage("JOB REQUESTED", chores[i].title, "Waiting Dad", false);
        } else {
          drawMessage("NOT AVAILABLE", "Already pending", "", true);
        }
        return;
      }
    }

    if (inside({20, 270, 200, 36, ""}, x, y)) drawAccount();
  }

  else if (currentScreen == ADMIN_MENU) {
    if (inside({20, 70, 200, 42, ""}, x, y))  drawAdminSelect();
    else if (inside({20, 130, 200, 42, ""}, x, y)) drawApprovals();
    else if (inside({20, 190, 200, 42, ""}, x, y)) drawAdminSelect();
    else if (inside({20, 260, 200, 36, ""}, x, y)) drawHome();
  }

  else if (currentScreen == ADMIN_SELECT) {
    for (int i = 0; i < ACCOUNT_COUNT; i++) {
      if (inside({20, 70 + i * 42, 200, 34, ""}, x, y)) {
        selectedAccount = i;
        drawAdminAction();
        return;
      }
    }

    if (inside({20, 270, 200, 36, ""}, x, y)) drawAdminMenu();
  }

  else if (currentScreen == ADMIN_ACTION) {
    if (inside({20, 100, 200, 42, ""}, x, y))      drawAmount(true);
    else if (inside({20, 155, 200, 42, ""}, x, y)) drawAmount(false);
    else if (inside({20, 215, 200, 42, ""}, x, y)) drawDadTax();
    else if (inside({20, 270, 200, 36, ""}, x, y)) drawAdminSelect();
  }

  else if (currentScreen == AMOUNT) {
    String key = keypadKey(x, y);
    if (key != "") handleAmount(key);
  }

  else if (currentScreen == APPROVALS) {
    int yPos = 65;

    for (int i = 0; i < CHORE_COUNT; i++) {
      if (chores[i].pendingChild >= 0) {
        if (inside({15, yPos, 210, 34, ""}, x, y)) {
          int child = chores[i].pendingChild;
          long reward = chores[i].reward;

          accounts[child].balance += reward;
          accounts[child].xp += 25;
          accounts[child].firstDeposit = true;

          addHistory(child, "+" + moneyText(reward) + " " + chores[i].title);

          chores[i].pendingChild = -1;
          updateAchievements(child);
          saveAccount(child);

          drawMessage("APPROVED", accounts[child].name, "+" + moneyText(reward), false);
          return;
        }

        yPos += 45;
      }
    }

    if (inside({20, 270, 200, 36, ""}, x, y)) drawAdminMenu();
  }

  else if (currentScreen == DAD_TAX) {
    long tax = (accounts[selectedAccount].balance * DAD_TAX_PERCENT) / 100;
    if (tax > 0 && inside({20, 185, 200, 42, ""}, x, y)) {
      accounts[selectedAccount].balance -= tax;
      addHistory(selectedAccount, "-" + moneyText(tax) + " Dad Tax");
      updateAchievements(selectedAccount);
      saveAccount(selectedAccount);
      drawMessage("TAX APPLIED", "-" + moneyText(tax), "Bal " + moneyText(accounts[selectedAccount].balance), false);
    } else if (inside({20, 240, 200, 36, ""}, x, y) || inside({20, 270, 200, 36, ""}, x, y)) {
      drawAdminAction();
    }
  }

  else if (currentScreen == MESSAGE) {
    if (inside({20, 250, 200, 42, ""}, x, y)) {
      if (adminMode) drawAdminMenu();
      else if (selectedAccount >= 0) drawAccount();
      else drawHome();
    }
  }
}

// ==========================
// SETUP
// ==========================
void setup() {
  Serial.begin(115200);
  Serial.println("Bank of Dad FULL CYD Wokwi Build Starting...");

  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
  tft.begin();
  tft.setRotation(0);

  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  touch.begin(touchSPI);
  touch.setRotation(0);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  setRGB(false, false, false);

  // Restore balances/XP from NVS. On a brand-new device the Preferences
  // namespace won't exist, so getLong/getBool return the defaults already
  // baked into the accounts[] array.
  bool freshInstall = !prefs.begin("bank", true);
  prefs.end();
  loadAccounts();
  loadChores();

  for (int i = 0; i < ACCOUNT_COUNT; i++) {
    if (freshInstall) addHistory(i, "Account opened");
  }

  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
  Serial.print("AP started: ");
  Serial.print(WIFI_AP_SSID);
  Serial.print("  IP: ");
  Serial.println(WiFi.softAPIP());

  setupWebServer();
  webServer.begin();
  Serial.println("Web chore editor: http://192.168.4.1");

  Serial.println("Ready.");
  Serial.println("Admin PIN: 9999");
  Serial.println("Child PINs: Nixon 1111, Oakley 2222, Hadley 3333, Maddison 4444");

  drawHome();
}

// ==========================
// LOOP
// ==========================
void loop() {
  webServer.handleClient();

  int x, y;
  if (getTouch(x, y)) {
    handleTouch(x, y);
  }
}

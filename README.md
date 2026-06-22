# Bank of Dad

A touch-screen pocket-money manager for kids, running on the **CYD (Cheap Yellow Display)** — an ESP32 dev board with a built-in 2.8" ILI9341 TFT (240×320) and XPT2046 resistive touchscreen.

---

## Features

| Area | Details |
|---|---|
| **Accounts** | 4 child accounts, each PIN-protected |
| **Balance** | Stored in pennies (GBP), displayed as `GBP x.xx` |
| **Persistence** | Balances, XP, and badges survive power cycles (ESP32 NVS via `Preferences`) |
| **History** | Rolling 6-entry transaction log per account |
| **Goals** | Visual progress bar toward a savings target |
| **Chores** | Kids request chores; Dad approves and pays out |
| **Chore Editor** | Edit chore names and rewards from any browser over WiFi (no router needed) |
| **Dad Tax** | Percentage-based deduction (default 10%) with confirmation screen |
| **Badges** | Achievements: first deposit, save £10, save £25, Level 2 |
| **Leaderboard** | Top-savers ranking across all accounts |
| **Admin** | Dad PIN (9999) → deposit / withdraw / Dad Tax / approve chores |
| **RGB LED** | Status colour: blue = home, green = logged in, yellow = admin, red = error |

---

## Hardware

| Component | Value |
|---|---|
| Board | ESP32 (CYD / JC2432W328) |
| Display | ILI9341 240×320 (HSPI) |
| Touch | XPT2046 resistive (VSPI) |
| RGB LED | Active-LOW on GPIO 4 / 16 / 17 |

### Pin mapping

```
TFT  : CS=15  DC=2   MOSI=13  MISO=12  SCK=14
Touch: CS=33  CLK=25 MOSI=32  MISO=39  IRQ=36
LED  : R=4    G=16   B=17
```

---

## Required Libraries

Install via the Arduino Library Manager (or see `libraries.txt`).  
`WiFi` and `WebServer` are part of the ESP32 Arduino core — no extra install needed.

| Library | Notes |
|---|---|
| `Adafruit GFX Library` | ≥ 1.11 |
| `Adafruit ILI9341` | ≥ 1.6 |
| `XPT2046_Touchscreen` | ≥ 1.4 |
| `WiFi` / `WebServer` | Built into ESP32 Arduino core |
| `Preferences` | Built into ESP32 Arduino core |

---

## Default PINs

| Account | PIN |
|---|---|
| Nixon | 1111 |
| Oakley | 2222 |
| Hadley | 3333 |
| Maddison | 4444 |
| **Dad Admin** | **9999** |

---

## Web Chore Editor

The ESP32 creates its own WPA2 WiFi hotspot on every boot — no home router needed.

| Setting | Default | Where to change |
|---|---|---|
| Hotspot name | `BankOfDad` | `#define WIFI_AP_SSID` |
| Hotspot password | `pocket123` | `#define WIFI_AP_PASS` |
| URL | `http://192.168.4.1` | — |

**How to use:**

1. On the device, enter **Dad Admin** — the sub-header shows `Web: BankOfDad > 192.168.4.1`
2. On your phone or laptop, connect to the `BankOfDad` WiFi network
3. Open `http://192.168.4.1` in a browser
4. Edit any chore name and/or reward (entered in GBP, e.g. `2.50`)
5. Tap **Save Chores** — changes apply immediately and persist across reboots

If a chore has a pending child approval, the editor shows a warning next to it.

---

## Touch Calibration

If touches feel offset on your specific CYD panel, adjust the four constants near the top of the sketch:

```cpp
#define TOUCH_CAL_MIN_X   200
#define TOUCH_CAL_MAX_X  3800
#define TOUCH_CAL_MIN_Y   200
#define TOUCH_CAL_MAX_Y  3800
```

To find your panel's true edges, temporarily add `Serial.print(p.x)` / `Serial.print(p.y)` inside `getTouch()` before the `map()` calls and read the raw values at each corner.

---

## Dad Tax

Dad Tax deducts a fixed percentage of a child's current balance.  
The default rate is **10%**, changeable with:

```cpp
#define DAD_TAX_PERCENT 10
```

The confirmation screen shows the current balance, calculated deduction, and projected balance before any money moves.

---

## Sketch structure

```
BankOfDad/
└── BankOfDad.ino   ← single-file sketch
libraries.txt       ← Library Manager names
```

Open `BankOfDad/BankOfDad.ino` in the Arduino IDE, select **ESP32 Dev Module**, and upload.

---

## Technical notes

- All money values are stored in **pennies** throughout to avoid floating-point rounding; `moneyText()` formats them as `GBP x.xx` for display.
- Persistence uses the ESP32 `Preferences` library (NVS flash). Account data keys: `bal`, `xp`, `fd`, `f10`, `f25` + account index suffix. Chore keys: `ct` (title), `cr` (reward) + chore index suffix. All in the `bank` namespace.
- Touch debounce is **non-blocking** — `getTouch()` checks `millis()` rather than calling `delay()`, so `webServer.handleClient()` runs freely in the main loop.
- Chore titles are stored as `char[32]` (mutable) rather than `const char*` so the web editor can update them in place.

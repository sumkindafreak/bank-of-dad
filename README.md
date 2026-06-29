# Bank of Dad

A retro family chore economy system where children earn pocket money, XP, levels, badges, streaks, and prize-shop rewards. Three targets are supported — pick the one that matches how you are running it.

| Target | Folder / entry | Platform | UI |
|---|---|---|---|
| **Web demo** | `index.html` | Any modern browser | HTML/CSS/JS |
| **CYD** | `BankOfDad/BankOfDad.ino` | ESP32 CYD (JC2432W328) | Adafruit GFX |
| **LVGL v2.2** | `BankOfDadLVGL/BankOfDadLVGL.ino` | ESP32-S3 JC8048W550C | LVGL 9 |

The LVGL target is the full commercial product build (UK household economy, shop, achievements, stats, notifications, modular architecture). The web demo is a browser prototype with `localStorage`. The CYD sketch is the original compact ESP32 build.

---

## Web demo

Open `index.html` in a browser — no install step required.

### Current features

- Child account dashboard
- Household chore list with rewards capped at £4.50
- Dad approval queue before money is awarded
- XP and level system
- Achievement badges
- Daily streak rewards
- 2% savings interest button
- Prize shop
- Kindness and no-reminder bonuses
- Local browser storage using `localStorage`
- Retro arcade / 80s computer styling

### Files

- `index.html` — main app layout
- `style.css` — retro dashboard styling
- `app.js` — child accounts, chores, approval queue, XP, badges, shop and storage logic

### Planned (web)

- Parent PIN login
- Child PIN login
- Photo proof uploads
- Custom chore / reward shop editors
- Export / backup data
- Firebase sync
- PWA install mode

---

## Features (ESP32 targets)

### Shared (CYD + LVGL)

| Area | Details |
|---|---|
| **Accounts** | 4 child accounts, each PIN-protected |
| **Balance** | Stored in pennies (GBP), displayed as `GBP x.xx` |
| **Persistence** | Balances, XP, and badges survive power cycles (ESP32 NVS via `Preferences`) |
| **History** | Rolling transaction log per account |
| **Goals** | Visual progress bar toward a savings target |
| **Chores** | Kids request chores; Dad approves and pays out |
| **Chore Editor** | Edit chore rewards from any browser over WiFi (no router needed) |
| **SD Card** | Optional FAT32 card — extended transaction logs + account backup/restore |
| **Dad Tax** | Percentage-based deduction (default 10%) with confirmation screen |
| **Leaderboard** | Top-savers ranking across all accounts |
| **Admin** | Dad PIN (9999) → deposit / withdraw / Dad Tax / approve chores |

### LVGL v2.2 additions (`BankOfDadLVGL/`)

| Area | Details |
|---|---|
| **UK Economy** | 27 household chores (max reward £4.50), categories, XP, completion counts |
| **Reward Shop** | 12 default rewards (chocolate, cinema, theme park, etc.) — kids purchase with balance |
| **Achievements** | 16 unlockable badges with animated popups |
| **Savings** | Separate savings balance, interest (weekly), goal name + progress estimate |
| **Statistics** | Lifetime/weekly/monthly earnings, chores completed, streaks, Dad tax total |
| **Notifications** | In-app alert centre (achievements, approvals, purchases, interest) |
| **Parent tools** | Approve/reject chores, bonus, fine, diagnostics screen |
| **CRT theme** | Green DOS terminal aesthetic with scanline overlay |
| **Architecture** | `model.*` data layer, `ui_lvgl.*` screens, `platform.*` WiFi web editor |

---

## Target 1 — CYD (`BankOfDad/BankOfDad.ino`)

### Hardware

| Component | Value |
|---|---|
| Board | ESP32 (CYD / JC2432W328) |
| Display | ILI9341 240×320 SPI (HSPI) |
| Touch | XPT2046 resistive (VSPI) |
| RGB LED | Active-LOW on GPIO 4 / 16 / 17 |

```
TFT  : CS=15  DC=2   MOSI=13  MISO=12  SCK=14
Touch: CS=33  CLK=25 MOSI=32  MISO=39  IRQ=36
LED  : R=4    G=16   B=17
```

### Arduino IDE settings

Board: **ESP32 Dev Module** → upload and go.

### Required Libraries

Install via the Arduino Library Manager (also listed in `libraries.txt`):

| Library | Notes |
|---|---|
| `Adafruit GFX Library` | ≥ 1.11 |
| `Adafruit ILI9341` | ≥ 1.6 |
| `XPT2046_Touchscreen` | ≥ 1.4 |
| `WiFi` / `WebServer` / `Preferences` / `SD` | Built into ESP32 Arduino core |

### microSD card (optional)

Insert a **FAT32** microSD card before boot. CS = **GPIO 5** (shares the TFT SPI bus).

| Path on card | Contents |
|---|---|
| `/bank/Nixon.log` (one per child) | Append-only transaction log |
| `/bank/backup.csv` | Balance + XP snapshot (written on every save) |

History screen shows up to **12 lines** from SD when mounted. On first boot with blank NVS, `backup.csv` is used to restore balances if present. Admin menu shows `SD OK` or `No SD`.

### Touch Calibration

If touches feel offset, adjust these four constants at the top of `BankOfDad.ino`:

```cpp
#define TOUCH_CAL_MIN_X   200
#define TOUCH_CAL_MAX_X  3800
#define TOUCH_CAL_MIN_Y   200
#define TOUCH_CAL_MAX_Y  3800
```

Print raw `p.x` / `p.y` from `getTouch()` to Serial to find your panel's true edges.

---

## Target 2 — JC8048W550C / ESP32-S3 (`BankOfDadLVGL/`)

### Hardware

| Component | Value |
|---|---|
| Board | ESP32-S3 (JC8048W550C) |
| Display | 5″ 800×480 RGB panel (LVGL rotates 90° → 480×800 portrait UI) |
| Touch | GT911 capacitive, 5-point |
| Backlight | GPIO 2 (PWM) |

```
RGB panel : DATA pins configured inside Arduino_ESP32RGBPanel in BankOfDadLVGL.ino
GT911     : SDA=19  SCL=20  INT=18  RST=38
Backlight : GPIO 2
```

### Arduino IDE settings

| Setting | Value |
|---|---|
| Board | ESP32S3 Dev Module |
| PSRAM | OPI PSRAM |
| Flash Size | 16MB |
| Flash Mode | QIO 80MHz |
| USB CDC On Boot | Enabled |

### Required Libraries

Install via the Arduino Library Manager:

| Library | Notes |
|---|---|
| `Arduino_GFX_Library` | moononournation/Arduino_GFX |
| `TAMC_GT911` | TAMCTech/TAMC_GT911 |
| `lvgl` | lvgl/lvgl ≥ 9.x |
| `WiFi` / `WebServer` / `Preferences` / `SD` | Built into ESP32-S3 Arduino core |

### microSD card (optional)

Insert a **FAT32** microSD card before boot.

| Pin | GPIO |
|---|---|
| CS | 10 |
| SCK | 12 |
| MISO | 13 |
| MOSI | 11 |

**On card:** same `/bank/*.log` and `/bank/backup.csv` layout as the CYD sketch (NVS-compatible).

- **History screen** shows up to 20 lines from SD when a card is present.
- **Admin menu** shows SD status in green when mounted.

Change pins in `BankOfDadLVGL/storage.h` if your board uses different SD wiring.

---

The sketch folder includes a pre-configured `lv_conf.h`. If your LVGL library version adds new required defines, copy the library's `lv_conf_template.h` into `BankOfDadLVGL/`, rename it `lv_conf.h`, change `#if 0` → `#if 1`, then apply the settings from the included `lv_conf.h`.

### File structure (v2.2)

```
BankOfDadLVGL/
├── BankOfDadLVGL.ino   ← hardware init (display, touch, backlight)
├── lv_conf.h           ← LVGL 9 configuration
├── app.h / app.cpp     ← thin shell: wires model + platform + UI
├── model.h / model.cpp ← data model, UK economy, NVS v2 persistence
├── ui_lvgl.h / ui_lvgl.cpp   ← all LVGL screens
├── ui_platform.h / ui_platform.cpp ← CRT theme, buttons, scroll lists, keypad
├── ui_fx.h / ui_fx.cpp       ← CRT overlay, achievement popups
├── platform.h / platform.cpp ← WiFi AP + web chore editor
├── rgb_sync.h / rgb_sync.cpp ← RGB status sync (future)
├── storage.h / storage.cpp   ← optional SD card logs + backup
├── lvgl_port.h/cpp     ← LVGL display driver (800×480 → 480×800 portrait)
└── touch_lvgl.h/cpp    ← GT911 → LVGL indev with 90° CW coordinate transform
```

### Touch Calibration (LVGL target)

If touches are mirrored or offset, edit the two lines marked `CALIBRATE` in `touch_lvgl.cpp`:

```cpp
case ROTATION_RIGHT:
    lx = py;                   // CALIBRATE if needed
    ly = (SCREEN_W - 1) - px;  // CALIBRATE if needed
    break;
```

Print `s_dev->tp[0].x` / `s_dev->tp[0].y` to Serial while tapping each screen corner to verify the mapping.

---

## Default PINs (ESP32)

| Account | PIN |
|---|---|
| Nixon | 1111 |
| Oakley | 2222 |
| Hadley | 3333 |
| Maddison | 4444 |
| **Dad Admin** | **9999** |

---

## Web Chore Editor (ESP32)

Both ESP32 targets create a WPA2 WiFi hotspot on boot — no home router needed.

| Setting | Default | Where to change |
|---|---|---|
| Hotspot name | `BankOfDad` | `BANK_WIFI_AP_SSID` in `model.h` (LVGL) or `BankOfDad.ino` (CYD) |
| Hotspot password | `pocket123` | `BANK_WIFI_AP_PASS` / `WIFI_AP_PASS` |
| URL | `http://192.168.4.1` | — |

**How to use:**

1. On the device, enter **Dad Admin** — the screen shows `Web: BankOfDad → 192.168.4.1`
2. Connect your phone or laptop to the `BankOfDad` WiFi network
3. Open `http://192.168.4.1` in a browser
4. Edit chore rewards (up to £4.50 on LVGL v2.2)
5. Tap **Save** — changes apply immediately and persist across reboots

A warning is shown next to any chore with a pending child approval.

---

## Dad Tax

Dad Tax deducts a fixed percentage of a child's current balance with a confirmation screen.  
Default rate is **10%** on both ESP32 targets. On LVGL v2.2, tax percentage and enable flag are stored in NVS (`g_bank.config.dadTaxPercent`) and shown on the diagnostics screen.

```cpp
// CYD sketch only — change at compile time:
#define DAD_TAX_PERCENT 10
```

---

## Technical Notes

- All money is stored in **pennies** to avoid floating-point rounding. `moneyText()` / `modelMoneyText()` formats as `GBP x.xx`.
- **CYD** NVS keys: `bal`, `xp`, `fd`, `f10`, `f25` + account index; `ct`, `cr` + chore index — in the `bank` namespace.
- **LVGL v2.2** uses storage version `220` with extended keys (`nm`, `av`, `sav`, `am`, etc.). v1 data is migrated automatically on first boot.
- **PIN security:** the message screen OK button routes to an explicit destination per message type. A wrong PIN returns to the PIN screen — it does **not** grant account access.
- **CYD**: touch debounce uses `millis()` — no `delay()` call.
- **LVGL**: `lv_timer_handler()` and `webServer.handleClient()` share the `loop()`. LVGL display buffers (2 × 800×20 × 2 bytes) are allocated with `ps_malloc()` in PSRAM. Large LVGL widget allocations are routed to PSRAM via `heap_caps_malloc_extmem_enable(8192)`.
- Screen transitions use `lv_async_call()` to ensure the current LVGL event completes before the old screen is deleted.

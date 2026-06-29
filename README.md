# Bank of Dad

A touch-screen pocket-money manager for kids. Two hardware targets are supported — pick the one that matches your board.

| Sketch | Board | Display | Touch | UI |
|---|---|---|---|---|
| `BankOfDad/` | ESP32 CYD (JC2432W328) | ILI9341 2.8″ 240×320 SPI | XPT2046 resistive | Adafruit GFX |
| `BankOfDadLVGL/` | ESP32-S3 JC8048W550C | 5″ 800×480 RGB panel | GT911 capacitive | LVGL 9 |

Both sketches share the same features, data format, NVS key scheme, and web chore editor.

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

---

## Target 1 — CYD (BankOfDad)

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
| `WiFi` / `WebServer` / `Preferences` | Built into ESP32 Arduino core |

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

## Target 2 — JC8048W550C / ESP32-S3 (BankOfDadLVGL)

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
| `WiFi` / `WebServer` / `Preferences` | Built into ESP32-S3 Arduino core |

### LVGL Configuration

The sketch folder includes a pre-configured `lv_conf.h`. If your LVGL library version adds new required defines, copy the library's `lv_conf_template.h` into `BankOfDadLVGL/`, rename it `lv_conf.h`, change `#if 0` → `#if 1`, then apply the settings from the included `lv_conf.h`.

### File structure

```
BankOfDadLVGL/
├── BankOfDadLVGL.ino   ← hardware init (display, touch, backlight)
├── lv_conf.h           ← LVGL 9 configuration
├── lvgl_port.h/cpp     ← LVGL display driver (800×480 → 480×800 portrait)
├── touch_lvgl.h/cpp    ← GT911 → LVGL indev with 90° CW coordinate transform
├── app.h               ← data structs, constants, function declarations
└── app.cpp             ← all screens, logic, WiFi AP, NVS persistence
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

Both targets create a WPA2 WiFi hotspot on boot — no home router needed.

| Setting | Default | Where to change |
|---|---|---|
| Hotspot name | `BankOfDad` | `#define WIFI_AP_SSID` in `BankOfDad.ino` or `app.h` |
| Hotspot password | `pocket123` | `#define WIFI_AP_PASS` |
| URL | `http://192.168.4.1` | — |

**How to use:**

1. On the device, enter **Dad Admin** — the screen shows `Web: BankOfDad → 192.168.4.1`
2. Connect your phone or laptop to the `BankOfDad` WiFi network
3. Open `http://192.168.4.1` in a browser
4. Edit any chore name and/or reward (entered in GBP, e.g. `2.50`)
5. Tap **Save Chores** — changes apply immediately and persist across reboots

A warning is shown next to any chore with a pending child approval.

---

## Dad Tax

Dad Tax deducts a fixed percentage of a child's current balance with a confirmation screen.  
Default rate is **10%**, changeable with:

```cpp
#define DAD_TAX_PERCENT 10
```

---

## Technical Notes

- All money is stored in **pennies** to avoid floating-point rounding. `moneyText()` formats as `GBP x.xx`.
- NVS keys: `bal`, `xp`, `fd`, `f10`, `f25` + account index; `ct`, `cr` + chore index — all in the `bank` namespace. Both sketches use the same key scheme so data is compatible if you ever swap hardware.
- **CYD**: touch debounce uses `millis()` — no `delay()` call.
- **LVGL**: `lv_timer_handler()` and `webServer.handleClient()` share the `loop()`. LVGL display buffers (2 × 800×20 × 2 bytes) are allocated with `ps_malloc()` in PSRAM. Large LVGL widget allocations are routed to PSRAM via `heap_caps_malloc_extmem_enable(8192)`.
- Screen transitions use `lv_async_call()` to ensure the current LVGL event completes before the old screen is deleted.

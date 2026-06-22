# Bank of Dad

A touch-screen pocket-money manager for kids, running on the **CYD (Cheap Yellow Display)** — an ESP32 dev board with a built-in 2.8" ILI9341 TFT (240×320) and XPT2046 resistive touchscreen.

---

## Features

| Area | Details |
|---|---|
| **Accounts** | 4 child accounts, each PIN-protected |
| **Balance** | Stored in pennies (GBP), displayed as £x.xx |
| **History** | Rolling 6-entry transaction log per account |
| **Goals** | Visual progress bar toward a savings target |
| **Chores** | Kids request chores; Dad approves and pays out |
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

Install all three via the Arduino Library Manager or `libraries.txt`:

| Library | Tested version |
|---|---|
| `Adafruit GFX Library` | ≥ 1.11 |
| `Adafruit ILI9341` | ≥ 1.6 |
| `XPT2046_Touchscreen` | ≥ 1.4 |

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

## Sketch structure

```
BankOfDad/
└── BankOfDad.ino   ← single-file sketch
```

Open `BankOfDad/BankOfDad.ino` in the Arduino IDE, select **ESP32 Dev Module**, and upload.

---

## Notes

- All money values are stored in **pennies** throughout the sketch to avoid floating-point rounding.
- The `moneyText()` helper formats pennies as `GBP x.xx` for display.
- Touch calibration constants (`200`/`3800`) may need tweaking for your specific CYD unit — adjust the `map()` call in `getTouch()`.
- Data is **not** persisted across resets (no EEPROM/SPIFFS write). Add NVS or SPIFFS save/load to make balances survive power cycles.

# Bank of Dad

![Bank of Dad social card](assets/bank-of-dad-social-card.png)

**Bank of Dad** is a retro-style family chore economy system where children can earn pocket money, XP, levels, badges, streaks and prize-shop rewards.

It is designed for parents who want to make chores, saving and rewards feel fun, fair and visible without turning pocket money into an argument.

> **Where chores become achievements, and savings become adventures.**

## What it does

Bank of Dad gives families a simple reward system built around earning, saving and approval.

Children can:

- View available chores
- Complete tasks for rewards
- Build XP, levels and streaks
- Unlock badges
- Save towards prize-shop rewards
- Learn basic budgeting and delayed gratification

Parents can:

- Approve completed chores before money is awarded
- Control chore values
- Manage child accounts
- Create reward-shop goals
- Encourage kindness, consistency and no-reminder bonuses
- Keep the whole system local and family-controlled

## Current features

- Child account dashboard
- Household chore list with rewards capped at £4.50
- Dad approval queue before money is awarded
- XP and level system
- Achievement badges
- Daily streak rewards
- 2% savings interest button
- Prize shop
- Kindness and no-reminder bonuses
- Dad Editor mode for chores, shop prices, and child accounts
- Local browser storage using `localStorage`
- Retro arcade / 80s computer styling
- ESP32-S3 LVGL touchscreen firmware for a physical Bank of Dad terminal

## Who it is for

Bank of Dad is for:

- Parents who want a fun way to manage chores and pocket money
- Children who respond well to goals, levels and visible progress
- Families who want to encourage saving rather than instant spending
- Makers who like the idea of a physical home banking terminal for children
- Anyone who wants a lightweight, local-first alternative to complicated reward apps

## How to run

Open `index.html` in a browser.

No install step is needed yet.

## Files

- `index.html` - main app layout
- `style.css` - retro dashboard styling
- `app.js` - child accounts, chores, approval queue, XP, badges, shop and storage logic
- `BankOfDad/` - ESP32-S3 LVGL firmware for the JC8048W550C touchscreen
- `docs/` - product, launch and marketing documentation
- `assets/` - launch images and visual assets

## Documentation

- [Product Overview](docs/PRODUCT_OVERVIEW.md)
- [Launch Plan](docs/LAUNCH_PLAN.md)
- [Marketing Copy](docs/MARKETING_COPY.md)
- [Press Kit](docs/PRESS_KIT.md)
- [Roadmap](docs/ROADMAP.md)
- [Beta Tester Guide](docs/BETA_TESTER_GUIDE.md)
- [Feature Audit](docs/FEATURE_AUDIT.md)
- [Hardware Terminal Ideas](docs/HARDWARE_TERMINAL_IDEAS.md)
- [Brand Notes](docs/BRAND_NOTES.md)
- [Asset Guide](assets/README.md)

## ESP32 firmware notes

Open the `BankOfDad/` folder as your Arduino sketch (entry point: **`BankOfDadLVGL.ino`**).

**Sync from git:** copy or pull the whole `BankOfDad/` folder into your Arduino project directory, replacing the module files there. Keep your local `lv_conf.h` if you already have one configured.

- Connect to the **BankOfDad** hotspot (`pocket123`) and open `http://192.168.4.1/wifi` to save your home WiFi credentials.
- After joining home WiFi, use the device IP from your router (shown on the screen) for phone access.
- OTA updates: Arduino IDE → **Upload Using OTA**, hostname `BankOfDad`, password `BANKOFDAD`.

## Product direction

Bank of Dad is currently a local-first family reward system. The next phase is to polish the product, test it with real families, collect feedback and prepare a public launch.

Planned future ideas include:

- Parent PIN login
- Child PIN login
- Photo proof uploads
- Custom chore editor
- Custom reward shop editor
- Export / backup data
- Firebase sync
- PWA install mode
- Public landing page
- Beta tester feedback loop
- Optional physical touchscreen terminal build guide

## Tagline options

- Where chores become achievements, and savings become adventures.
- Teach. Earn. Save. Reward.
- Pocket money with purpose.
- A family bank for chores, goals and rewards.

## Status

Bank of Dad is in active development and preparing for beta testing.

# Bank of Dad

A retro-style family chore economy system where children can earn pocket money, XP, levels, badges, streaks and prize-shop rewards.

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
- Local browser storage using `localStorage`
- Retro arcade / 80s computer styling

## How to run

Open `index.html` in a browser.

No install step is needed yet.

## Files

- `index.html` - main app layout
- `style.css` - retro dashboard styling
- `app.js` - child accounts, chores, approval queue, XP, badges, shop and storage logic
- `BankOfDad/` - ESP32-S3 LVGL firmware for the JC8048W550C touchscreen

### ESP32 firmware notes

- Connect to the **BankOfDad** hotspot (`pocket123`) and open `http://192.168.4.1/wifi` to save your home WiFi credentials.
- After joining home WiFi, use the device IP from your router (shown on the screen) for phone access.
- OTA updates: Arduino IDE → **Upload Using OTA**, hostname `BankOfDad`, password `BANKOFDAD`.

## Next ideas

- Parent PIN login
- Child PIN login
- Photo proof uploads
- Custom chore editor
- Custom reward shop editor
- Export / backup data
- Firebase sync
- PWA install mode

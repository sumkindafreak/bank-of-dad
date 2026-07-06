# Bank of Dad - Hardware Terminal Ideas

## Purpose

Bank of Dad has a strong opportunity to become more than a web app. The optional touchscreen terminal can turn it into a physical family banking station that children recognise as part of the household routine.

## Current hardware direction

The repository already includes an ESP32-S3 LVGL firmware folder for a JC8048W550C touchscreen display module.

The current README notes:

- Open the `BankOfDad/` folder as the Arduino sketch.
- Entry point: `BankOfDadLVGL.ino`.
- Device hotspot: `BankOfDad`.
- Setup password: `pocket123`.
- WiFi setup page: `http://192.168.4.1/wifi`.
- OTA hostname: `BankOfDad`.
- OTA password: `BANKOFDAD`.

## Physical product vision

A small countertop family terminal where children can:

- Choose their account
- View their balance
- See XP and level progress
- Mark chores complete
- Check their savings goal
- Browse rewards
- Celebrate badges

Parents can:

- Approve chores
- Reject chores
- Add bonuses
- Manage values
- Check household activity

## Enclosure ideas

### Retro bank terminal

A small angled desktop case inspired by a cash machine, till or 1980s computer terminal.

### Mini arcade cabinet

A playful upright case with the screen at the top and a few optional physical buttons below.

### Wall-mounted family hub

A kitchen or hallway mounted terminal for quick chore check-ins.

### 3D printed wedge case

A simple wedge-shaped enclosure that is easy to print and sits on a shelf or desk.

## Possible physical controls

- Parent approval button
- Back button
- Confirm button
- Child selector buttons
- Rotary encoder for scrolling
- RFID or NFC reader for account selection
- Small speaker for reward sounds
- NeoPixel strip for achievement animations

## Possible feedback effects

- Coin sound when money is awarded
- Level-up jingle
- Badge unlock flash
- Green light for approval
- Amber light for pending approval
- Red light for rejected chore
- Savings goal celebration

## Optional hardware enhancements

### Thermal receipt printer

Print a small receipt when a chore is approved.

Example receipt:

- Child name
- Chore completed
- Reward earned
- XP earned
- New balance

### RFID child cards

Each child has a card or token to open their account.

### Parent admin card

A parent card opens approval or editor mode.

### Physical coin slot prop

A fun non-banking prop where a parent can add pocket money or bonuses manually.

### LED status bar

A visible household status indicator:

- Green: all clear
- Amber: chores waiting for approval
- Pink: reward available
- Blue: savings goal close

## Safety notes

- The terminal should be treated as a household reward device, not a banking product.
- Do not connect it to real bank accounts without a proper compliance review.
- Avoid exposing private family data outside the home network unless cloud sync is deliberately designed later.
- Add parent PIN protection before any serious home use.

## Suggested hardware roadmap

1. Stable touchscreen dashboard
2. Child account selector
3. Parent PIN screen
4. Approval queue on touchscreen
5. Reward shop view
6. WiFi setup polish
7. OTA update guide
8. 3D printable case
9. Optional speaker sounds
10. Optional RFID account login

## Kit idea

A future Bank of Dad hardware kit could include:

- ESP32-S3 touchscreen module
- 3D printed case files
- Setup guide
- USB power cable
- Optional speaker
- Optional NFC reader
- Sticker sheet
- Child account cards

## Product angle

The hardware terminal could become the most memorable part of Bank of Dad. Apps are common, but a glowing retro family bank on the kitchen side feels unique, shareable and marketable.

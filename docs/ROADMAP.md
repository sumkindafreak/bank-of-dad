# Bank of Dad - Roadmap and Future Enhancements

## Roadmap purpose

This roadmap captures possible future enhancements for Bank of Dad without changing the current codebase. It is intended as a planning document for product direction, beta feedback and future development.

## Current foundation

The current app already includes:

- Local browser storage
- Child accounts
- Chore list
- Chore approval queue
- Pocket-money balance
- XP
- Levels
- Badges
- Streaks
- Prize shop
- Savings goal tracking
- Kindness bonus
- No-reminder bonus
- Interest button
- Dad Editor mode
- Demo reset
- Optional ESP32-S3 touchscreen firmware

## Priority 1 - Safety, trust and parent control

### Parent PIN

Add a parent PIN before opening Dad Editor mode, approving chores, applying bonuses, changing balances or resetting data.

### Child PIN

Allow each child to have a simple PIN or avatar login to prevent siblings using the wrong account.

### Approval notes

Let parents add a note when approving or rejecting a chore.

Example:

- Great job
- Try again: under the bed still needs tidying
- Approved with bonus

### Undo last action

Add a simple undo for accidental approvals, reward purchases or balance edits.

### Audit log

Create a full parent-visible transaction history showing:

- Chores submitted
- Chores approved
- Chores rejected
- Rewards bought
- Bonuses awarded
- Manual balance edits
- Interest added

## Priority 2 - Better family experience

### Custom avatars

Let each child choose an avatar, colour or icon.

### Child themes

Give each child a personal dashboard theme while keeping the main retro style.

### Savings jars

Allow multiple goals per child.

Examples:

- Toy fund
- Trip fund
- Game fund
- Sweet fund
- Long-term savings

### Chore difficulty ratings

Add easy, medium and hard chore types to help children choose suitable tasks.

### Age-based chore lists

Let parents mark chores as suitable for different age ranges.

### Chore cooldowns

Prevent repeat claiming the same chore too often.

Example:

- Make bed: once per day
- Wash car: once per week
- Deep clean bathroom: once per week

### Recurring chores

Support daily, weekly and one-off chores.

### Family challenges

Create shared goals where all children contribute.

Examples:

- Family movie night
- Pizza night
- Day out fund

## Priority 3 - Motivation and game systems

### Expanded badge system

Add more achievements:

- First £5 saved
- First £20 saved
- 5 chores in a week
- 30-day saver
- Kindness champion
- No-reminder legend
- Room rescue hero
- Kitchen helper

### Badge rarity

Use rarity levels:

- Common
- Uncommon
- Rare
- Epic
- Legendary

### Level rewards

Unlock perks when children level up.

Examples:

- Bonus XP day
- Custom avatar item
- Extra reward-shop option
- Parent-defined treat

### Quest board

Add themed weekly quests.

Examples:

- Complete 3 kitchen chores
- Earn 100 XP this week
- Save £5 without spending
- Help someone without being asked

### Streak protection

Give children a limited number of streak saves so they do not lose motivation after one missed day.

### Random bonus cards

Parent-triggered surprise bonuses.

Examples:

- Double XP for next chore
- Mystery 50p bonus
- Reward-shop discount

## Priority 4 - Money learning

### Save, spend and give pots

Split balances into:

- Spend
- Save
- Give

### Interest rules

Replace the manual interest button with configurable parent rules.

Examples:

- 2% every Sunday
- 5% monthly
- Parent-approved interest only

### Budget lessons

Add simple child-friendly lessons about:

- Saving
- Waiting
- Spending wisely
- Earning
- Helping others

### Reward reservation

Let a child reserve a reward so they can see how close they are.

### Savings milestone celebration

Trigger a celebration when a child reaches 25%, 50%, 75% and 100% of a savings goal.

## Priority 5 - Data and backup

### Export data

Allow parents to export family data as JSON or CSV.

### Import data

Allow restoring data from a backup file.

### Print report

Generate a printable weekly or monthly report.

### Local device sync

Allow multiple devices on the same home network to view the same household data.

### Cloud sync

Possible future cloud sync for families who want multi-device access.

Important: cloud sync should be designed carefully because the app involves children.

## Priority 6 - Progressive Web App

### Installable PWA

Make Bank of Dad installable on phones, tablets and computers.

### Offline mode

Ensure the app works without internet once loaded.

### Home screen icon

Create app icons for phone installation.

### Push reminders

Optional reminders for chores, streaks and reward goals.

## Priority 7 - Parent dashboard

### Weekly summary

Show parent-friendly stats:

- Total earned
- Total chores completed
- Most active child
- Most popular chore
- Pending approvals
- Rewards purchased

### Child comparison without shame

Show progress carefully and positively. Avoid making children feel publicly ranked in a negative way.

### Household economy controls

Let parents set:

- Maximum daily earnings
- Maximum weekly earnings
- Bonus limits
- Interest limits
- Chore categories

## Priority 8 - Hardware terminal

### Touchscreen kiosk mode

Turn the ESP32-S3 screen into a dedicated family banking terminal.

### Parent approval mode on terminal

Allow parent PIN approval directly from the device.

### Child account selector

Large child buttons for touchscreen use.

### Physical feedback

Possible hardware feedback:

- Beeps
- LED animations
- Coin sound
- Badge unlock sound
- Reward purchase sound

### Receipt printer idea

A future fun enhancement: print small thermal receipts for approved chores or reward purchases.

### RFID/NFC child login

Children tap a card or token to open their account.

### Coin slot prop mode

A playful physical coin-slot prop that adds a parent-approved amount.

## Priority 9 - Community and sharing

### Shareable chore templates

Parents can share chore lists.

### Reward shop templates

Share reward ideas by age range.

### Family setup wizard

A guided setup that asks:

- How many children?
- Ages?
- Pocket-money style?
- Weekly budget?
- Reward preferences?

### Parent community ideas

Create a page where parents suggest chores, rewards and badge ideas.

## Priority 10 - Monetisation possibilities

### Free local version

Keep a simple local version free to build trust.

### Family Pro

Possible paid features:

- Multiple devices
- Backups
- Advanced reports
- Custom themes
- Expanded reward shop
- PIN protection
- Printable reports

### Physical kit

Future hardware product:

- Prebuilt Bank of Dad touchscreen terminal
- DIY kit
- 3D printable enclosure
- Setup guide

### Template packs

Optional packs:

- Ages 5-7 chore pack
- Ages 8-10 chore pack
- Ages 11-14 chore pack
- Kindness challenge pack
- School holiday pack

## Important product principles

- Parent controlled
- Child friendly
- Privacy aware
- No real banking integration without serious compliance review
- Fun before complicated
- Encouraging rather than punishing
- Local-first where possible
- Clear, simple and visual

## Suggested next build priorities

1. Parent PIN
2. Child PIN or avatar login
3. Export and import backup
4. PWA install mode
5. Better reward shop flow
6. Recurring chores
7. Expanded transaction history
8. Beta feedback form
9. Landing page
10. Hardware terminal guide

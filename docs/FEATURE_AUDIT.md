# Bank of Dad - Feature Audit

This document records the current behaviour observed from the repository so marketing and future planning stay aligned with the actual product.

## Storage

The web app stores its state in the browser using localStorage under the key `bankOfDadStateV1`.

## Default accounts

The app starts with four example accounts:

- Hadley
- Oakley
- Maddison
- Nixon

Each account includes:

- Name
- Balance
- XP
- Streak
- Completed chore count
- Badges
- Savings goal
- Recent activity history

## Default savings goal

Each new account starts with a savings goal:

- Goal name: Nintendo Game
- Target: £60.00

## Chore system

The app includes a default chore list with categories including:

- Daily
- Bedroom
- Pets
- Home
- Kitchen
- Cleaning
- Laundry
- Bathroom
- Outdoor
- Garden

Rewards are stored in pence and displayed as pounds.

The current editor caps chore rewards at £4.50.

## Approval queue

When a chore is marked complete, it is not paid immediately.

Instead, it is added to the approval queue with:

- Account ID
- Chore ID
- Submission time

A parent can then approve or reject it.

## Approval behaviour

When approved:

- The account balance increases by the chore reward.
- XP increases by the chore XP value.
- Completed chore count increases.
- A recent activity entry is added.
- Achievement checks run.
- The approval is removed from the queue.

When rejected:

- The approval is removed from the queue.
- A message tells the user to try again.

## XP and levels

The app includes five rank names:

1. Starter Helper
2. House Apprentice
3. Chore Hero
4. Bank Builder
5. Household Legend

The current level calculation advances by XP bands of 150.

## Achievements

Current achievements include:

- First Chore
- 10 Chores
- 100 XP
- £10 Saver
- 3 Day Streak
- 7 Day Streak

## Prize shop

The current default rewards include:

- Chocolate bar
- Pick tonight's film
- Stay up 30 minutes later
- McDonald's treat
- Cinema trip
- Video game fund

If the account does not have enough balance, the app shows how much more is needed.

## Bonuses

The app includes buttons for:

- 50p kindness bonus
- £1 no-reminder bonus

Bonuses also award XP.

## Interest

The app includes a manual interest button that adds 2% interest to the selected account balance.

If the calculated interest is zero, the app asks the user to save a little first.

## Streaks

The app includes a streak button that increases the selected account streak by one day.

Current streak bonuses:

- 3-day streak: 50p bonus
- 7-day streak: £1 bonus

## Dad Editor mode

Dad Editor mode currently allows editing:

- Chore name
- Chore category
- Chore reward
- Chore XP
- Reward-shop item name
- Reward-shop price
- Account name
- Account balance
- Savings goal name
- Savings goal target

It also allows removing chores, rewards and accounts, while keeping at least one account.

## Demo reset

The app includes a demo reset button that clears localStorage and reloads the default state.

## Current best marketing claims supported by the app

- Parent-approved chores
- Child accounts
- Pocket-money balance tracking
- XP and levels
- Achievement badges
- Daily streaks
- Savings goals
- Reward shop
- Editable chores and rewards
- Local browser storage
- Retro family reward system

## Claims to avoid for now

Unless future code adds them, avoid claiming:

- Cloud sync
- Bank account connection
- Real payments
- Automatic recurring chores
- PIN protection
- Photo proof uploads
- Mobile app store release
- Multi-device sync
- Push notifications

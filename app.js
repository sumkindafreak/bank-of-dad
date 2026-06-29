// Bank of Dad - Retro family chore economy
// This app stores everything in the browser using localStorage.

const STORAGE_KEY = "bankOfDadStateV1";

const defaultChores = [
  { id: crypto.randomUUID(), name: "Make your bed", rewardPence: 20, xp: 5, category: "Daily" },
  { id: crypto.randomUUID(), name: "Put dirty clothes in laundry basket", rewardPence: 20, xp: 5, category: "Daily" },
  { id: crypto.randomUUID(), name: "Tidy bedroom", rewardPence: 50, xp: 12, category: "Bedroom" },
  { id: crypto.randomUUID(), name: "Empty bedroom bin", rewardPence: 50, xp: 10, category: "Bedroom" },
  { id: crypto.randomUUID(), name: "Feed pets", rewardPence: 50, xp: 10, category: "Pets" },
  { id: crypto.randomUUID(), name: "Water indoor plants", rewardPence: 50, xp: 10, category: "Home" },
  { id: crypto.randomUUID(), name: "Set the dinner table", rewardPence: 50, xp: 10, category: "Kitchen" },
  { id: crypto.randomUUID(), name: "Clear the dinner table", rewardPence: 50, xp: 10, category: "Kitchen" },
  { id: crypto.randomUUID(), name: "Wipe kitchen sides", rewardPence: 75, xp: 15, category: "Kitchen" },
  { id: crypto.randomUUID(), name: "Vacuum one room", rewardPence: 100, xp: 20, category: "Cleaning" },
  { id: crypto.randomUUID(), name: "Sweep hard floors", rewardPence: 100, xp: 20, category: "Cleaning" },
  { id: crypto.randomUUID(), name: "Dust downstairs", rewardPence: 100, xp: 20, category: "Cleaning" },
  { id: crypto.randomUUID(), name: "Unload dishwasher", rewardPence: 100, xp: 20, category: "Kitchen" },
  { id: crypto.randomUUID(), name: "Load dishwasher", rewardPence: 125, xp: 24, category: "Kitchen" },
  { id: crypto.randomUUID(), name: "Fold clean washing", rewardPence: 125, xp: 24, category: "Laundry" },
  { id: crypto.randomUUID(), name: "Hang washing out", rewardPence: 125, xp: 24, category: "Laundry" },
  { id: crypto.randomUUID(), name: "Bring washing in", rewardPence: 125, xp: 24, category: "Laundry" },
  { id: crypto.randomUUID(), name: "Clean bathroom sink and mirror", rewardPence: 150, xp: 30, category: "Bathroom" },
  { id: crypto.randomUUID(), name: "Mop kitchen floor", rewardPence: 150, xp: 30, category: "Kitchen" },
  { id: crypto.randomUUID(), name: "Hoover whole downstairs", rewardPence: 200, xp: 40, category: "Cleaning" },
  { id: crypto.randomUUID(), name: "Clean the toilet", rewardPence: 200, xp: 45, category: "Bathroom" },
  { id: crypto.randomUUID(), name: "Help prepare dinner", rewardPence: 200, xp: 40, category: "Kitchen" },
  { id: crypto.randomUUID(), name: "Wash the car", rewardPence: 250, xp: 55, category: "Outdoor" },
  { id: crypto.randomUUID(), name: "Mow the lawn", rewardPence: 300, xp: 65, category: "Garden" },
  { id: crypto.randomUUID(), name: "Clean all windows inside", rewardPence: 350, xp: 75, category: "Cleaning" },
  { id: crypto.randomUUID(), name: "Deep clean bathroom", rewardPence: 400, xp: 90, category: "Bathroom" },
  { id: crypto.randomUUID(), name: "Deep clean kitchen", rewardPence: 450, xp: 100, category: "Kitchen" }
];

const rewardShopItems = [
  { id: crypto.randomUUID(), name: "Chocolate bar", costPence: 200 },
  { id: crypto.randomUUID(), name: "Pick tonight's film", costPence: 500 },
  { id: crypto.randomUUID(), name: "Stay up 30 minutes later", costPence: 750 },
  { id: crypto.randomUUID(), name: "McDonald's treat", costPence: 1500 },
  { id: crypto.randomUUID(), name: "Cinema trip", costPence: 3000 },
  { id: crypto.randomUUID(), name: "Video game fund", costPence: 5000 }
];

const levelNames = [
  "Starter Helper",
  "House Apprentice",
  "Chore Hero",
  "Bank Builder",
  "Household Legend"
];

const defaultChildren = [
  createChild("Hadley"),
  createChild("Oakley"),
  createChild("Maddison"),
  createChild("Nixon")
];

let state = loadState();
let selectedChildId = state.children[0]?.id;

const childrenList = document.getElementById("childrenList");
const childDashboard = document.getElementById("childDashboard");
const choresList = document.getElementById("choresList");
const approvalQueue = document.getElementById("approvalQueue");
const rewardShop = document.getElementById("rewardShop");
const addChildForm = document.getElementById("addChildForm");
const newChildName = document.getElementById("newChildName");
const toast = document.getElementById("toast");

function createChild(name) {
  return {
    id: crypto.randomUUID(),
    name,
    balancePence: 0,
    xp: 0,
    streak: 0,
    completedChores: 0,
    badges: [],
    savingsGoal: {
      name: "Nintendo Game",
      targetPence: 6000
    },
    history: []
  };
}

function loadState() {
  const saved = localStorage.getItem(STORAGE_KEY);

  if (saved) {
    return JSON.parse(saved);
  }

  return {
    children: defaultChildren,
    chores: defaultChores,
    approvals: [],
    shop: rewardShopItems
  };
}

function saveState() {
  localStorage.setItem(STORAGE_KEY, JSON.stringify(state));
}

function money(pence) {
  return `£${(pence / 100).toFixed(2)}`;
}

function getSelectedChild() {
  return state.children.find(child => child.id === selectedChildId) || state.children[0];
}

function getLevel(xp) {
  return Math.min(Math.floor(xp / 150), levelNames.length - 1);
}

function showToast(message) {
  toast.textContent = message;
  toast.classList.add("show");
  setTimeout(() => toast.classList.remove("show"), 2200);
}

function addHistory(child, text) {
  child.history.unshift({ id: crypto.randomUUID(), text, date: new Date().toLocaleString("en-GB") });
  child.history = child.history.slice(0, 8);
}

function checkAchievements(child) {
  const unlocked = [];

  const achievements = [
    { id: "first-chore", label: "First Chore", test: () => child.completedChores >= 1 },
    { id: "ten-chores", label: "10 Chores", test: () => child.completedChores >= 10 },
    { id: "hundred-xp", label: "100 XP", test: () => child.xp >= 100 },
    { id: "saving-star", label: "£10 Saver", test: () => child.balancePence >= 1000 },
    { id: "streak-3", label: "3 Day Streak", test: () => child.streak >= 3 },
    { id: "streak-7", label: "7 Day Streak", test: () => child.streak >= 7 }
  ];

  achievements.forEach(achievement => {
    if (!child.badges.includes(achievement.id) && achievement.test()) {
      child.badges.push(achievement.id);
      unlocked.push(achievement.label);
    }
  });

  if (unlocked.length > 0) {
    showToast(`Achievement unlocked: ${unlocked.join(", ")}`);
  }
}

function requestChore(choreId) {
  const child = getSelectedChild();
  const chore = state.chores.find(item => item.id === choreId);

  state.approvals.push({
    id: crypto.randomUUID(),
    childId: child.id,
    choreId: chore.id,
    requestedAt: new Date().toLocaleString("en-GB")
  });

  saveState();
  render();
  showToast(`${child.name} submitted ${chore.name} for approval`);
}

function approveChore(approvalId) {
  const approval = state.approvals.find(item => item.id === approvalId);
  const child = state.children.find(item => item.id === approval.childId);
  const chore = state.chores.find(item => item.id === approval.choreId);

  child.balancePence += chore.rewardPence;
  child.xp += chore.xp;
  child.completedChores += 1;
  addHistory(child, `Earned ${money(chore.rewardPence)} and ${chore.xp} XP for ${chore.name}`);
  checkAchievements(child);

  state.approvals = state.approvals.filter(item => item.id !== approvalId);
  saveState();
  render();
  showToast(`Approved: ${child.name} earned ${money(chore.rewardPence)}`);
}

function rejectChore(approvalId) {
  state.approvals = state.approvals.filter(item => item.id !== approvalId);
  saveState();
  render();
  showToast("Chore sent back for another try");
}

function buyReward(itemId) {
  const child = getSelectedChild();
  const item = state.shop.find(reward => reward.id === itemId);

  if (child.balancePence < item.costPence) {
    showToast(`${child.name} needs ${money(item.costPence - child.balancePence)} more`);
    return;
  }

  child.balancePence -= item.costPence;
  addHistory(child, `Bought reward: ${item.name} for ${money(item.costPence)}`);
  saveState();
  render();
  showToast(`${item.name} purchased`);
}

function awardBonus(amountPence, reason) {
  const child = getSelectedChild();
  child.balancePence += amountPence;
  child.xp += Math.max(5, Math.round(amountPence / 10));
  addHistory(child, `Bonus: ${reason} ${money(amountPence)}`);
  checkAchievements(child);
  saveState();
  render();
  showToast(`${child.name} got ${money(amountPence)} bonus`);
}

function applyInterest() {
  const child = getSelectedChild();
  const interest = Math.floor(child.balancePence * 0.02);

  if (interest <= 0) {
    showToast("No interest yet — save a little first");
    return;
  }

  child.balancePence += interest;
  addHistory(child, `Sunday interest added: ${money(interest)}`);
  saveState();
  render();
  showToast(`Interest added: ${money(interest)}`);
}

function addStreakDay() {
  const child = getSelectedChild();
  child.streak += 1;

  if (child.streak === 3) {
    child.balancePence += 50;
    addHistory(child, "3 day streak bonus: £0.50");
  }

  if (child.streak === 7) {
    child.balancePence += 100;
    addHistory(child, "7 day streak bonus: £1.00");
  }

  checkAchievements(child);
  saveState();
  render();
  showToast(`${child.name}'s streak is now ${child.streak}`);
}

function renderChildren() {
  childrenList.innerHTML = "";

  state.children.forEach(child => {
    const button = document.createElement("button");
    button.className = `child-button ${child.id === selectedChildId ? "active" : ""}`;
    button.textContent = `${child.name} · ${money(child.balancePence)}`;
    button.addEventListener("click", () => {
      selectedChildId = child.id;
      render();
    });
    childrenList.appendChild(button);
  });
}

function renderDashboard() {
  const child = getSelectedChild();
  const level = getLevel(child.xp);
  const nextLevelXp = (level + 1) * 150;
  const levelProgress = level === levelNames.length - 1 ? 100 : Math.min(100, Math.round((child.xp / nextLevelXp) * 100));
  const goalProgress = Math.min(100, Math.round((child.balancePence / child.savingsGoal.targetPence) * 100));

  childDashboard.innerHTML = `
    <div class="hero-card">
      <div>
        <p class="eyebrow">Active Account</p>
        <h2>${child.name}</h2>
        <div class="balance">${money(child.balancePence)}</div>
      </div>

      <div class="stats-grid">
        <div class="stat-box"><span class="stat-label">Level</span><span class="stat-value">${level + 1}</span></div>
        <div class="stat-box"><span class="stat-label">Rank</span><span class="stat-value">${levelNames[level]}</span></div>
        <div class="stat-box"><span class="stat-label">XP</span><span class="stat-value">${child.xp}</span></div>
        <div class="stat-box"><span class="stat-label">Streak</span><span class="stat-value">${child.streak} days</span></div>
        <div class="stat-box"><span class="stat-label">Completed</span><span class="stat-value">${child.completedChores}</span></div>
      </div>

      <div class="progress-wrap">
        <strong>XP Progress</strong>
        <div class="progress-bar"><div class="progress-fill" style="width:${levelProgress}%"></div></div>
      </div>

      <div class="progress-wrap">
        <strong>${child.savingsGoal.name}: ${money(child.balancePence)} / ${money(child.savingsGoal.targetPence)}</strong>
        <div class="progress-bar"><div class="progress-fill" style="width:${goalProgress}%"></div></div>
      </div>

      <div>
        <h3>Badges</h3>
        <div class="badges">
          ${child.badges.length ? child.badges.map(id => `<span class="badge">${id.replaceAll("-", " ")}</span>`).join("") : `<span class="badge">No badges yet</span>`}
        </div>
      </div>

      <div>
        <h3>Recent Activity</h3>
        <div class="card-list">
          ${child.history.length ? child.history.map(item => `<div class="card"><strong>${item.text}</strong><small>${item.date}</small></div>`).join("") : `<div class="card">No activity yet. Complete a chore to start earning.</div>`}
        </div>
      </div>
    </div>
  `;
}

function renderChores() {
  choresList.innerHTML = "";

  state.chores.forEach(chore => {
    const card = document.createElement("div");
    card.className = "card";
    card.innerHTML = `
      <div class="card-title-row">
        <strong>${chore.name}</strong>
        <span class="price">${money(chore.rewardPence)}</span>
      </div>
      <small>${chore.category} · ${chore.xp} XP</small>
      <button>Mark Complete</button>
    `;

    card.querySelector("button").addEventListener("click", () => requestChore(chore.id));
    choresList.appendChild(card);
  });
}

function renderApprovals() {
  approvalQueue.innerHTML = "";

  if (state.approvals.length === 0) {
    approvalQueue.innerHTML = `<div class="card">No chores waiting for approval.</div>`;
    return;
  }

  state.approvals.forEach(approval => {
    const child = state.children.find(item => item.id === approval.childId);
    const chore = state.chores.find(item => item.id === approval.choreId);
    const card = document.createElement("div");
    card.className = "card";
    card.innerHTML = `
      <div class="card-title-row">
        <strong>${child.name}</strong>
        <span class="price">${money(chore.rewardPence)}</span>
      </div>
      <span>${chore.name}</span>
      <small>Submitted: ${approval.requestedAt}</small>
      <button class="approve">Approve</button>
      <button class="danger reject">Try Again</button>
    `;

    card.querySelector(".approve").addEventListener("click", () => approveChore(approval.id));
    card.querySelector(".reject").addEventListener("click", () => rejectChore(approval.id));
    approvalQueue.appendChild(card);
  });
}

function renderShop() {
  rewardShop.innerHTML = "";

  state.shop.forEach(item => {
    const card = document.createElement("div");
    card.className = "card";
    card.innerHTML = `
      <div class="card-title-row">
        <strong>${item.name}</strong>
        <span class="price">${money(item.costPence)}</span>
      </div>
      <button>Buy Reward</button>
    `;

    card.querySelector("button").addEventListener("click", () => buyReward(item.id));
    rewardShop.appendChild(card);
  });
}

function render() {
  renderChildren();
  renderDashboard();
  renderChores();
  renderApprovals();
  renderShop();
}

addChildForm.addEventListener("submit", event => {
  event.preventDefault();
  const child = createChild(newChildName.value.trim());
  state.children.push(child);
  selectedChildId = child.id;
  newChildName.value = "";
  saveState();
  render();
  showToast(`${child.name} added`);
});

document.getElementById("bonus50Btn").addEventListener("click", () => awardBonus(50, "Kindness"));
document.getElementById("bonus100Btn").addEventListener("click", () => awardBonus(100, "No reminders needed"));
document.getElementById("interestBtn").addEventListener("click", applyInterest);
document.getElementById("streakBtn").addEventListener("click", addStreakDay);

document.getElementById("resetDemoBtn").addEventListener("click", () => {
  localStorage.removeItem(STORAGE_KEY);
  state = loadState();
  selectedChildId = state.children[0]?.id;
  render();
  showToast("Demo reset complete");
});

render();

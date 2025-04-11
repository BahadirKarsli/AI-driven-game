# 🧵 Multithreaded Demon Slayer Game

## 📌 Overview

This project is a multithreaded *Demon Slayer* game developed for the **BIL 222**. It uses **POSIX threads** for concurrent behavior and **SDL2** for visualization. The game simulates AI-controlled players and demons battling on a **20x20 grid**. Players aim to defeat demons to grow stronger, while demons evolve over time and can eliminate players. The implementation satisfies all functionality and visualization requirements, resulting in a dynamic, competitive gameplay experience.

---

## Demo Video
https://github.com/user-attachments/assets/65de6526-6441-48aa-9b72-7ec6e7cc6cb3

## 🎮 Core Functionality

### 👣 Movement Strategies

Three player movement strategies add diversity and strategic depth:

- **Random Walk:**  
  Moves randomly in x/y direction (`-1, 0, or +1`).  
  ➤ Simple and unpredictable baseline behavior.

- **Demon Seeking:**  
  Calculates Euclidean distance to all demons, moves toward the closest one.  
  ➤ Encourages aggressive, efficient hunting.

- **Strength Control:**  
  Scans a 5x5 grid for the strongest demon nearby and moves accordingly.  
  ➤ Balances risk and reward by targeting high-value zones.

---

### ⚔️ Combat Resolution

Follows this logic:

```c
if (player->score >= demon->score) {
    player->score += demon->score;
    remove_demon(demon);
} else {
    demon->score += player->score;
    remove_player(player);
}
```

- Combat occurs when a player and a demon occupy the same cell.
- Checked in both player and demon threads.
- Players have a slight edge (`>=`) to prevent early elimination.

---

### 🧵 Thread Management

- **Each entity** (player or demon) runs in its own thread.
- Managed by dedicated **player_generator** and **demon_generator** threads.

#### 🧐 Initial Spawns
- 3 players + 3 demons spawned at startup.

#### 🔁 Ongoing Spawns
- A new **player every 4 seconds**, **demon every 5 seconds**.

#### ✅ Clean Termination
- Threads terminate cleanly.
- Freed memory upon entity removal or game end.

---

### ⚖️ Game Balancing

- **Initial Strengths:**  
  Players ~12, Demons ~6 (to encourage early aggression).
  
- **Demon Growth:**  
  +1 strength every 5 ticks.

- **Delays:**  
  - Player moves: `usleep(200000)` (0.2s)  
  - Demon moves: `usleep(300000)` (0.3s)

- **Termination Conditions:**
  - All demons or players defeated.
  - OR after 120 seconds (score-based winner).

---

## 🖼️ Visualization

### 🔵 Players

- Shape: Square  
- Size: `10 + score / 5` pixels  
- Color: Blue, intensity `100 + score * 5` (max 255)

### 🔴 Demons

- Shape: Circle  
- Size: `5 + score / 5` pixels  
- Color: Red, intensity `100 + score * 5` (max 255)

### 🗺️ Grid

- 20x20 grid (CELL_SIZE = 30 → 600x600 window)
- Grid lines enhance clarity.
- Rendering at ~60 FPS (`usleep(16666)`)

### ❌ No Text

- SDL_ttf excluded for simplicity.
- Names/scores omitted to reduce dependencies.

---

## 🧠 Memory Management

### 📦 Allocation

- Players and demons allocated with `malloc`.
- Stored in global arrays: `players[]`, `demons[]`.

### 🛉 Removal

- `remove_player()` and `remove_demon()`:
  - Shifts array contents to maintain continuity.
  - Updates counters.

### 🛼 Cleanup at Game End

- All threads joined.
- Memory freed.
- SDL context closed with `close_view()`.

---

## 🔚 Game Termination

Game ends under the following conditions:

- ✅ **All Demons Defeated:**  
  → Players win.
- ❌ **All Players Defeated:**  
  → Demons win.
- ⏳ **120-Second Timeout:**  
  → Winner is the side with the higher total score.
- ❎ **Manual Quit:**  
  → SDL_QUIT triggers graceful shutdown.

---

## ⚠️ Challenges & Notes

- **Game Balancing:**  
  Fine-tuning strengths/spawn rates was key for lasting matches.
  
- **No Mutexes Allowed:**  
  Combat race conditions reduced by using time delays (not eliminated).

- **Early Exits:**  
  Early builds ended too quickly.  
  ➤ Initial entity count and time limits extended game duration.


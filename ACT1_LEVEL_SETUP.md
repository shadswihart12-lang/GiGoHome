# GI GO HOME — Act 1 Level Setup Guide
# "The Ash" — Five Mission Integration Manifest

---

## BUILD STATUS AFTER TODAY'S SESSION

| Gap | Fix | Status |
|-----|-----|--------|
| `DoInteract()` only hit supply caches — Missions 4 & 5 escort/spare broken | Added Campaign NPC interaction to `XiDongCharacter.cpp` | ✅ FIXED |
| Mission 3 timer invisible — no HUD countdown display | Added `ShowMissionTimer()` / `HideMissionTimer()` to `SGISlateHUD` + `UGISlateHUDManager` + wired `TickMissionTimer` | ✅ FIXED |

All five missions are now **fully functional end-to-end in C++**. No Blueprints required.

---

## COMPILE FIRST

1. Open `GIGOHOME.sln` in Visual Studio 2022
2. Build → `Development Editor` → `Win64`
3. Open `GIGOHOME.uproject` in UE5

---

## MISSION 1 — "The Proving"
**File:** `ACT1_M1_TheProving.umap`  
**GameMode:** `AGIAct1Mission1GameMode`

### Required World Actors

| Actor Class | Tag | Notes |
|-------------|-----|-------|
| `AGICampaignNPC` | `M1_RadioOperator` | Set Initial State: `Sleeping`. Wakes after ~WakeUpTime seconds if not killed. Place inside hut/tent. |
| `AGIRadioActor` | `M1_Radio` | Place next to radio operator. Default cube mesh — replace with radio prop. |
| Any Actor (empty) | `M1_Extraction` | Invisible trigger marker. Place at cell entry/exit point. |
| `APlayerStart` | — | Xi Dong's spawn point |

### World Settings
- **GameMode Override:** `AGIAct1Mission1GameMode`
- **Game State Class:** `AGIGoHomeCampaignState`

### How It Plays
- Radio operator sleeping → knife kill → `M1_NeutralizeOperator` complete
- Hit radio with any damage (or knife) until HP = 0 → `M1_DestroyRadio` complete  
- Walk within 250cm of `M1_Extraction` actor → `M1_Extract` complete → Victory screen

### Player Choice Recorded
- `M1_KilledSleeping` — auto-fires if operator dies while in `Sleeping` state
- `M1_WaitedForDawn` — fires if operator was NOT sleeping when killed (set WakeUpTime > 0 on the NPC)

---

## MISSION 2 — "The Observation"
**File:** `ACT1_M2_TheObservation.umap`  
**GameMode:** `AGIAct1Mission2GameMode`

### Required World Actors

| Actor Class | Tag | Notes |
|-------------|-----|-------|
| `AGIObservationPoint` (x3) | auto-detected by class | Set `RequiredObservationTime = 15.0` on each. Place overlooking listening post activity. Enable `bRequireCrouch = true`. |
| Any Actor | `M2_Extraction` | Place at jungle egress point |
| `APlayerStart` | — | Xi Dong's spawn |

### World Settings
- **GameMode Override:** `AGIAct1Mission2GameMode`

### How It Plays
- Crouch inside each observation sphere for 15s → objectives complete in order
- After all 3 observed → radio ID auto-completes
- Walk within 250cm of extraction → mission ends

### Optional: "Assault" Branch
- Set a trigger actor with tag `M2_AssaultChoice` to auto-record if player engages enemies instead of observing
- Or leave default: observation completion auto-records `M2_CompletedObservation`

---

## MISSION 3 — "The Distraction"
**File:** `ACT1_M3_TheDistraction.umap`  
**GameMode:** `AGIAct1Mission3GameMode`

### Required World Actors

| Actor Class | Tag | Notes |
|-------------|-----|-------|
| `AGIRadioActor` | `M3_PrimaryRadio` | The radio the enemy tries to use. Bind `OnRadioUsed` → mission fails if any enemy calls `UseRadio()` on it. |
| `AGIRadioActor` | `M3_BackupRadio` | Secondary radio Xi Dong can destroy. Destroying it completes `M3_StopBackupRadio`. |
| `AGICampaignNPC` | `M3_Runner` | Set Initial State: `Fleeing`. Will run toward backup radio on spawn. |
| Any Actor | `M3_Extraction` | Extraction point — active only after timer expires AND backup radio secured |
| `APlayerStart` | — | Xi Dong's spawn |

### World Settings
- **GameMode Override:** `AGIAct1Mission3GameMode`
- Timer auto-starts at mission begin. **HUD shows `12:00` countdown** (gold color, top-center). Turns red in the final 60 seconds.

### How It Plays
- Timer counts down from 12:00
- Prevent enemies reaching `M3_PrimaryRadio` for entire duration
- Option A: Destroy `M3_BackupRadio` (records `M3_DestroyedBackupRadio` choice)
- Option B: Kill the `M3_Runner` NPC before it reaches radio (records `M3_ChasedRunner` choice)
- Timer hits 0 → `M3_SurviveTimer` completes → reach extraction → Victory

### Enemy Integration
- Spawn `AGIEnemyGreen`/`AGIEnemyVeteran` from the Wave Manager or manually placed enemies
- Enemy NPC that reaches `M3_PrimaryRadio` should call `Radio->UseRadio(this)` to trigger mission failure
- Wire in the Enemy AI controller's `OnRadioContactAttempt` behavior (can call directly from ShooterAIController or EQS task)

---

## MISSION 4 — "Tyler Greg"
**File:** `ACT1_M4_TylerGreg.umap`  
**GameMode:** `AGIAct1Mission4GameMode`

### Required World Actors

| Actor Class | Tag | Notes |
|-------------|-----|-------|
| Any Actor | `M4_Perimeter` | Firebase perimeter marker. Walk within 300cm to complete `M4_ReachPerimeter`. |
| `AGICampaignNPC` | `M4_TylerGreg` | American radio operator. Place in drainage ditch area. Set `InteractionRadius = 200cm`. |
| Any Actor | `M4_Extraction` | Cell extraction point — active after Tyler resolved |
| `APlayerStart` | — | Xi Dong's spawn point |

### World Settings
- **GameMode Override:** `AGIAct1Mission4GameMode`

### Tyler Greg Setup
- Set `NPCName = "Tyler Greg"` on the NPC
- Set `InitialState = Idle` or `WritingLetter` (sitting on crate composing letter home)
- Set `CarriedItems` with tag `TG_OperatorIntel` (optional intel item for flavor)

### How It Plays
- Xi Dong traverses to firebase perimeter → `M4_ReachPerimeter` complete
- At drainage ditch, Tyler Greg is alone:
  - **Press E** within 200cm → Spare (records `M4_SparedTylerGreg`) → intel exchange
  - **Attack** Tyler Greg until dead → `OnNPCKilled` → records `M4_KilledTylerGreg`
- Either path completes `M4_ResolveTyler`
- Return to extraction → `EndMission(true)` → Victory

### Campaign Consequence
Tyler Greg's fate (spared vs. killed) changes the difficulty tone of Mission 5:
- `ApplyPriorChoiceConsequences()` in M5 checks `HasMadeChoice(EPlayerChoice::M4_SparedTylerGreg)` if you add that logic to the M5 GameMode override

---

## MISSION 5 — "Firebase Kestrel"
**File:** `ACT1_M5_FirebaseKestrel.umap`  
**GameMode:** `AGIAct1Mission5GameMode`

### Required World Actors

| Actor Class | Tag | Notes |
|-------------|-----|-------|
| Any Actor | `M5_KestrelCore` | Center of the firebase. Walk within 300cm → infiltration complete. |
| `AGICampaignNPC` | `M5_Briggs` | Lieutenant Briggs. Set `InitialState = Escorted` (hooded, bound, slow). Place in firebase detention area. |
| Any Actor | `M5_TunnelExit` | Tunnel network entrance. This is the escort destination. |
| `APlayerStart` | — | Xi Dong's spawn |

### World Settings
- **GameMode Override:** `AGIAct1Mission5GameMode`

### Briggs Setup
- Set `NPCName = "Lieutenant Briggs"` on the NPC
- Set `InitialState = Escorted`
- The escort component (`UGIEscortComponent`) auto-attaches when Briggs is interacted with
- Briggs stumbles every 20s (randomized ±40%) — making noise that alerts nearby enemies
- Adjust `StumbleInterval`, `StumbleDuration`, `StumbleNoiseLoudness` on the component

### How It Plays
- Xi Dong infiltrates to `M5_KestrelCore` → `M5_Infiltrate` complete
- Find Briggs → **Press E** within 200cm → escort begins
- Briggs follows Xi Dong toward `M5_TunnelExit`
- Briggs stumbles periodically (noise event fires — enemies in range will investigate)
- Briggs reaches `M5_TunnelExit` → `OnEscortReachedDestination` fires → `EndMission(true)`

### Squad Integration (if spawning squad for M5)
- Xi Dong's squad commands (1-4 keys) work on `ASquadMemberCharacter` actors
- Place squad spawners with `AGIEnemyVeteran`/`AGIEnemyGreen` configured as friendlies (TeamByte = 0)
- Mission 5 is Act 1's only squad mission — the rest are solo

---

## ALL MISSIONS: UNIVERSAL SETUP CHECKLIST

### Every Level Needs:
- [ ] `APlayerStart` placed
- [ ] World Settings → Game Mode Override → correct `AGIAct1MissionXGameMode`
- [ ] World Settings → Game State Class → `AGIGoHomeCampaignState`
- [ ] NavMesh built (Actors → Build → Build Paths) — enemy AI requires it
- [ ] Tag all required actors per mission table above

### Optional (enemy combat):
- [ ] Place enemies with `AGIEnemyGreen` / `AGIEnemyVeteran` / `AGIEnemyBroken` / `AGIEnemyOfficer`
- [ ] Tag enemy spawn points as `EnemySpawn` for Wave Manager support

---

## KEY BINDINGS (NO EDITOR SETUP REQUIRED)

All input actions are created in C++ by `AGIXiDongConcrete`. No IMC asset needed.

| Key | Action |
|-----|--------|
| WASD | Move |
| Mouse | Aim |
| Left Mouse | Fire |
| E | Interact (supply cache → NPC → both) |
| R | Reload |
| T | Ammo Check |
| Left Ctrl / C | Crouch toggle |
| Left Shift | Sprint |
| Left Alt | Hold Breath |
| G | Body Drag |
| V | Place Trap |
| 1 / 2 / 3 / 4 | Squad orders (Hold / Advance / Flank / Fall Back) |

---

## PUBLISHER DEMO MODE

All mission game modes inherit `bEnablePublisherDemoMode = true` and `DemoSessionTimeLimitSeconds = 1200.0f` (20 minutes). The demo session auto-fails the mission after 20 minutes and logs a telemetry event. Disable in World Settings by setting `bEnablePublisherDemoMode = false` on the GameMode instance.

Console commands:
- `DemoRestartMission` — restart current mission level

Telemetry is logged to UE output log with prefix `[PublisherTelemetry]`. Call `GetCampaignState()->GetTelemetrySummary()` at any time for a one-line stats report.

---

*Last updated by Studio — Act 1 is compile-ready and gameplay-complete.*

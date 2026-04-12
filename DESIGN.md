# GI GO HOME
## Game Design Document — Publisher Edition
### Version 1.0 | Vertical Slice Build

---

## EXECUTIVE SUMMARY

**GI GO HOME** is a single-player stealth-action game set during the Vietnam War, experienced from the perspective of **Xi Dong**, a North Vietnamese guerrilla fighter defending his homeland against American occupation.

**Core Pitch:** *"You are not the hero. You are the resistance."*

This is not a power fantasy. This is survival. The player is outgunned, outnumbered, and operating in occupied territory. Victory comes through cunning — through traps, ambushes, and psychological warfare.

**Target Platform:** PC (Steam), Console (PS5/XSX)  
**Engine:** Unreal Engine 5  
**Target Rating:** M (Mature)  
**Development Timeline:** 24 months to full release  
**Budget Range:** $2-5M (indie AA)

---

## THE HOOK

### What Makes This Different

| Traditional War Game | GI GO HOME |
|---------------------|------------|
| Play as the superpower | Play as the underdog |
| Overwhelming firepower | Limited ammunition |
| Health bars and HUD | Health through screen effects |
| Respawn on death | Permadeath per mission |
| Faceless enemies | Enemies with fear and morale |

**The Player Experience:**
- Set punji traps on patrol routes, then wait
- Eliminate the officer first — watch the squad lose cohesion
- Drag bodies into the jungle before the next patrol arrives
- Hear the distant radio call — reinforcements are coming
- Choose: fight, hide, or flee through the tunnel network

---

## CORE PILLARS

### 1. ASYMMETRIC WARFARE
You are one fighter against squads of professional soldiers. Direct confrontation is suicide. Success requires:
- **Preparation:** Scout patrol routes, place traps, identify officers
- **Patience:** Wait for the perfect moment
- **Precision:** Every bullet counts — ammunition is scarce
- **Escape:** The tunnel network is your lifeline

### 2. PSYCHOLOGICAL COMBAT
Enemies are not bullet sponges. They are soldiers with:
- **Fear:** Accumulates from guerrilla activity. Fearful patrols move cautiously.
- **Morale:** Drops when allies die. At low morale, soldiers rout or surrender.
- **Archetypes:**
  - *Green* — Young, nervous rookies. Unpredictable.
  - *Veteran* — Dangerous. Check corners, communicate.
  - *Broken* — Crossed the line. No mercy.
  - *Officer* — Commands from rear. Priority target.

### 3. ENVIRONMENTAL AUTHENTICITY
Real terrain data from Quang Tri Province via Cesium:
- Latitude: 16.4637°
- Longitude: 107.0831°
- Dense jungle, river networks, village clearings
- Procedural foliage system creates authentic Vietnam jungle

### 4. MINIMAL HUD
Per the GDD philosophy: *"No kill counters, no XP bars, no progression metrics."*
- Health shown through screen effects and breathing
- Ammunition tracked mentally (manual ammo check)
- No waypoints — learn the land
- No tutorials — learn by doing

---

## GAMEPLAY SYSTEMS

### Combat

**Weapons:**
- AK-47 (Primary) — Full auto, loud, attracts attention
- SKS Rifle — Semi-auto, accurate, quieter
- Grenades — Precious, use only when necessary
- Knife — Silent kills, body disposal

**Traps:**
- Punji Pit — Concealed spike trap, causes bleeding
- Tripwire Grenade — Explosive ambush
- Pressure Mine — Area denial

**Stealth:**
- Crouch to reduce visibility
- Hold breath to steady aim (limited duration)
- Drag bodies to hide evidence
- Use foliage for concealment

### Enemy AI

**Patrol System:**
- Waypoint-based patrols with randomized timing
- Alert states: Patrolling → Alerted → Engaging → Searching → Routing
- Cone-based sight detection (60° half-angle, 30m range)
- Hearing system detects gunfire (50m range)

**Squad Dynamics:**
- Morale shared within awareness radius
- Officer elimination causes cohesion loss
- Low morale triggers routing (enemies flee)
- Routing enemies can escape and call reinforcements

### Progression

**Campaign Structure:**
- **Act 1: The Ash** — Xi Dong operates alone. Tutorial woven into narrative.
- **Act 2: The Ember** — Squad mechanics introduced.
- **Act 3: The Flame** — Large-scale operations.

**No Unlock Trees:** All tools available from start. Mastery comes from understanding the systems, not grinding.

---

## VERTICAL SLICE CONTENT

The publisher build demonstrates:

### Playable Systems
- [x] Xi Dong character (movement, crouch, sprint, hold breath)
- [x] Weapon system (AK-47 with reload, ammo check)
- [x] Trap placement (punji pit, tripwire)
- [x] Enemy AI (patrol, alert, engage, search, rout)
- [x] Morale/Fear systems
- [x] Wave-based mission structure (5 waves)
- [x] Death/respawn handling

### Environment
- [x] Cesium real-world terrain (Quang Tri Province)
- [x] Procedural Vietnam jungle foliage
- [x] Weather system (rain, fog)
- [x] Tunnel entrance system

### Pending for Full Vertical Slice
- [ ] Full audio pass (ambient jungle, Vietnamese voice lines)
- [ ] Body drag system polish
- [ ] Tunnel navigation gameplay
- [ ] Mission briefing/debrief screens

---

## TECHNICAL SPECIFICATIONS

### Engine Features Used
- **Cesium for Unreal** — Real-world terrain streaming
- **Enhanced Input** — Modern input handling
- **Hierarchical Instanced Static Meshes** — Efficient foliage rendering
- **AI StateTree** — Modular behavior trees
- **Chaos Physics** — Ragdoll death system

### Performance Targets
| Platform | Target FPS | Resolution |
|----------|-----------|------------|
| PC (High) | 60 | 4K |
| PC (Medium) | 60 | 1440p |
| PS5/XSX | 60 | 4K (Dynamic) |

### Code Architecture
- Pure C++ implementation (no Blueprint dependency)
- Component-based character systems
- Data-driven wave/mission configuration
- Modular AI through patrol components

---

## MARKET ANALYSIS

### Comparable Titles
| Title | Release | Sales | Similarity |
|-------|---------|-------|------------|
| *Sniper Elite 5* | 2022 | 2M+ | Tactical stealth |
| *A Plague Tale: Requiem* | 2022 | 1M+ | Asymmetric survival |
| *Spec Ops: The Line* | 2012 | 2M+ | War narrative |

### Unique Positioning
No major release has explored the Vietnam War from the Vietnamese perspective. This represents untapped narrative territory with significant differentiation potential.

### Target Audience
- Core: Tactical shooter fans (25-40, male-skewed)
- Secondary: Historical game enthusiasts
- Tertiary: Narrative-focused players

---

## TEAM REQUIREMENTS

### Current State
- Solo developer (design, programming)
- C++ codebase: ~15,000 lines
- Systems 80% complete

### Needed for Full Production
| Role | Count | Duration |
|------|-------|----------|
| Lead Programmer | 1 | 18 months |
| AI Programmer | 1 | 12 months |
| Environment Artist | 2 | 18 months |
| Character Artist | 1 | 12 months |
| Animator | 1 | 12 months |
| Sound Designer | 1 | 12 months |
| Composer | 1 (contract) | 6 months |
| QA | 2 | 12 months |

---

## BUDGET BREAKDOWN

| Category | Allocation |
|----------|------------|
| Personnel | 65% |
| Tools/Licenses | 5% |
| Audio/Music | 10% |
| Marketing | 15% |
| Contingency | 5% |

---

## MILESTONE SCHEDULE

| Milestone | Target | Deliverable |
|-----------|--------|-------------|
| M1: Vertical Slice | Month 0 | Current build |
| M2: Alpha (Act 1) | Month 8 | Full Act 1 playable |
| M3: Beta (All Acts) | Month 16 | Content complete |
| M4: Polish | Month 20 | Bug fixes, optimization |
| M5: Gold | Month 24 | Release candidate |

---

## APPENDIX: CONTROLS

| Action | Keyboard | Gamepad |
|--------|----------|---------|
| Move | WASD | Left Stick |
| Look | Mouse | Right Stick |
| Fire | LMB | RT |
| Aim | RMB | LT |
| Reload | R | X |
| Crouch | C | B |
| Sprint | Shift | L3 |
| Hold Breath | Alt | RB |
| Place Trap | T | D-Pad Down |
| Interact | E | A |
| Ammo Check | Tab | D-Pad Right |

---

## CONTACT

**Developer:** [Your Name]  
**Email:** [Your Email]  
**Build Access:** [Steam/Platform Key]

---

*"We live in the glory together."*  
— GI GO HOME

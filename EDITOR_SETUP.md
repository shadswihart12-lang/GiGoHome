# GI GO HOME - Editor Setup Notes

## KNOWN ISSUES & FIXES

### 1. IMC Sprint Index[2] Empty Trigger

**Problem:** The Input Mapping Context has an empty trigger at Index[2] for the Sprint action, causing warnings.

**Fix:**
1. Open the Editor
2. Navigate to `Content/Input/IMC_XiDong` (or your main IMC)
3. Find the Sprint action
4. Look for Index [2] in the Triggers array
5. Either:
   - **Delete** the empty trigger entry, OR
   - **Add** a valid trigger (like `Pressed` or `Released`)
6. Save the asset

### 2. GIEnemyCharacter Mesh Invisible

**Problem:** Enemy meshes not appearing due to first-person visibility inheritance.

**Fix Applied (C++):**
- Added `UGIForcedVisibilityComponent` to all enemy characters
- Forces visibility in `OnRegister()`, `PostInitializeComponents()`, and `BeginPlay()`
- No Blueprint setup required

**If Still Invisible:**
1. Check that the enemy Blueprint has a Skeletal Mesh assigned
2. Verify the mesh has valid LODs
3. Check that the mesh isn't culled by distance (adjust bounds scale)

### 3. Nanite Warning

**Problem:** Nanite warnings in the output log.

**Check:**
1. Open `Project Settings > Rendering > Nanite`
2. Ensure Nanite is enabled if using Nanite meshes
3. If not using Nanite, disable it to suppress warnings

**For Cesium Terrain:**
Cesium terrain tiles don't use Nanite by default. The warning may be from imported static meshes. Either:
- Enable Nanite on your foliage/prop meshes
- Ignore the warning (it's non-blocking)

---

## BLUEPRINT SETUP CHECKLIST

### BP_XiDong (Player Character)

Required Input Action References:
- `IA_Move` — WASD movement
- `IA_Look` — Mouse look
- `IA_Jump` — Spacebar
- `IA_Fire` — Left Mouse Button
- `IA_Crouch` — C
- `IA_Sprint` — Shift (FIX THE EMPTY TRIGGER)
- `IA_Reload` — R
- `IA_AmmoCheck` — Tab
- `IA_Interact` — E
- `IA_PlaceTrap` — T
- `IA_HoldBreath` — Alt

### BP_Enemy_Green / Veteran / Broken / Officer

Required:
- Skeletal Mesh assigned (e.g., `SK_Mannequin` or custom soldier mesh)
- Weapon Class assigned (e.g., `BP_Weapon_AK47`)
- Animation Blueprint assigned (uses `UGIEnemyAnimInstance` by default)

### BP_WaveManager

Required:
- Place in level
- Assign enemy class references for each archetype
- Place actors with tag `EnemySpawn` for spawn points (or it will auto-generate around PlayerStart)

---

## CESIUM TERRAIN SETUP

The project uses Cesium for Unreal with real-world coordinates:

**Location:** Quang Tri Province, Vietnam
- Latitude: 16.4637
- Longitude: 107.0831

**Setup:**
1. Install Cesium for Unreal plugin
2. Add `CesiumGeoreference` to the level
3. Set origin to the coordinates above
4. Add `Cesium3DTileset` with Cesium World Terrain or Google Photorealistic 3D Tiles
5. Enable `CesiumSunSky` for accurate lighting

---

## BUILD COMMANDS

### Development Build
```
RunUAT.bat BuildCookRun -project="C:\MG\modglad\GIGOHOME\GIGOHOME.uproject" -platform=Win64 -clientconfig=Development -cook -stage -pak
```

### Shipping Build (Publisher Demo)
```
RunUAT.bat BuildCookRun -project="C:\MG\modglad\GIGOHOME\GIGOHOME.uproject" -platform=Win64 -clientconfig=Shipping -cook -stage -pak -archive
```

---

## CONTACT

For code questions, reference the source at:
`C:\MG\modglad\GIGOHOME\Source\GIGOHOME\`

Key files:
- `GIGoHome/Player/XiDongCharacter.cpp` — Player implementation
- `GIGoHome/AI/GIEnemyCharacter.cpp` — Enemy implementation
- `GIGoHome/Wave/GIWaveManager.cpp` — Wave spawning
- `GIGoHome/GIGoHomeSliceGameMode.cpp` — Mission logic

// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Vertical Slice Game Mode
// Per GDD: Minimal HUD, no tutorials, learn by doing
// PRODUCTION-READY: Auto-bootstraps all required systems

#pragma once

#include "CoreMinimal.h"
#include "GIGoHomeGameMode.h"
#include "GIGoHomeSliceGameMode.generated.h"

class AGIWaveManager;
class UGISlateHUDManager;

UENUM(BlueprintType)
enum class EMissionObjective : uint8
{
	SurviveAllWaves,
	EliminateOfficer,
	DestroyRadio,
	PlaceTraps,
	Completed
};

USTRUCT(BlueprintType)
struct FMissionObjectiveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMissionObjective Type = EMissionObjective::SurviveAllWaves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentProgress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredProgress = 1;

	UPROPERTY(BlueprintReadOnly)
	bool bCompleted = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWaveChangedDelegate, int32, WaveNumber, int32, TotalWaves);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectiveUpdatedDelegate, const FMissionObjectiveData&, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMissionCompleteDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMissionFailedDelegate);

/**
 * Concrete game mode for the GI GO HOME vertical slice
 * Per GDD Section 7.1 Act 1 - The Ash:
 * - Xi Dong operates alone
 * - Tutorial woven into narrative - no UI prompts, learn by doing
 * - Introduces stealth mechanics, basic combat
 * - Harassment operations, small ambushes
 * 
 * This mode manages:
 * - Wave progression (5 escalating waves per default config)
 * - Mission objectives (survive, eliminate officers, set traps)
 * - Victory condition (all waves cleared)
 * - Defeat condition (player death - no respawn per GDD)
 * 
 * AUTO-BOOTSTRAPS:
 * - Pure Slate HUD (no Blueprint required)
 * - WaveManager with concrete enemy classes
 * - Player input mapping
 */
UCLASS()
class GIGOHOME_API AGIGoHomeSliceGameMode : public AGIGoHomeGameMode
{
	GENERATED_BODY()

public:

	/** Called when wave changes - for internal tracking */
	UPROPERTY(BlueprintAssignable, Category="GIGoHome")
	FWaveChangedDelegate OnWaveChanged;

	/** Called when any objective is updated */
	UPROPERTY(BlueprintAssignable, Category="GIGoHome")
	FObjectiveUpdatedDelegate OnObjectiveUpdated;

	/** Called when mission is completed successfully */
	UPROPERTY(BlueprintAssignable, Category="GIGoHome")
	FMissionCompleteDelegate OnMissionComplete;

	/** Called when mission fails (player death) */
	UPROPERTY(BlueprintAssignable, Category="GIGoHome")
	FMissionFailedDelegate OnMissionFailed;

protected:

	/** Mission objectives for this slice */
	UPROPERTY(EditAnywhere, Category="Mission")
	TArray<FMissionObjectiveData> MissionObjectives;

	/** Total number of waves in this mission */
	UPROPERTY(EditAnywhere, Category="Mission")
	int32 TotalWaves = 5;

	/** Current wave (1-indexed for display) */
	UPROPERTY(BlueprintReadOnly, Category="Mission")
	int32 CurrentWave = 1;

	/** Pure Slate HUD Manager - no Blueprint required */
	UPROPERTY()
	UGISlateHUDManager* SlateHUD;

	/** Reference to the wave manager in the level (auto-spawned if missing) */
	UPROPERTY()
	AGIWaveManager* WaveManager;

	/** Whether mission is complete */
	bool bMissionComplete = false;

	// --- Publisher Demo ---

	/** Cap demo sessions at this many seconds (default 20 min). 0 = disabled. */
	UPROPERTY(EditAnywhere, Category="Publisher Demo", meta=(ClampMin=0.0f, Units="s"))
	float DemoSessionTimeLimitSeconds = 1200.0f;

	FTimerHandle DemoSessionTimerHandle;

public:

	AGIGoHomeSliceGameMode();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void OnPlayerDeath() override;

	/** Called by WaveManager when a new wave starts */
	UFUNCTION(BlueprintCallable, Category="GIGoHome")
	void NotifyWaveStarted(int32 WaveIndex);

	/** Called by WaveManager when all waves are cleared */
	UFUNCTION(BlueprintCallable, Category="GIGoHome")
	void NotifyAllWavesCleared();

	/** Called when an officer is eliminated - per GDD causes squad cohesion loss */
	UFUNCTION(BlueprintCallable, Category="GIGoHome")
	void NotifyOfficerEliminated();

	/** Called when a radio is destroyed - prevents reinforcements */
	UFUNCTION(BlueprintCallable, Category="GIGoHome")
	void NotifyRadioDestroyed();

	/** Called when a trap is placed */
	UFUNCTION(BlueprintCallable, Category="GIGoHome")
	void NotifyTrapPlaced();

	/** Called by player for ammo check - triggers HUD display */
	UFUNCTION(BlueprintCallable, Category="GIGoHome")
	void ShowAmmoCheck(int32 CurrentAmmo, int32 MagazineSize);

	/** Returns current mission objectives */
	UFUNCTION(BlueprintPure, Category="GIGoHome")
	const TArray<FMissionObjectiveData>& GetMissionObjectives() const { return MissionObjectives; }

	/** Returns current wave (1-indexed) */
	UFUNCTION(BlueprintPure, Category="GIGoHome")
	int32 GetCurrentWave() const { return CurrentWave; }

	/** Returns total waves */
	UFUNCTION(BlueprintPure, Category="GIGoHome")
	int32 GetTotalWaves() const { return TotalWaves; }

protected:

	void SetupDefaultObjectives();
	void UpdateObjective(EMissionObjective Type, int32 ProgressDelta = 1);
	void CheckMissionComplete();

	/** Auto-spawns WaveManager if none exists in level */
	void EnsureWaveManager();

	/** Creates pure Slate HUD */
	void CreateSlateHUD();

	/** Starts the publisher session cap timer */
	void StartDemoTimer();

	/** Called when publisher demo timer expires */
	void OnDemoTimerExpired();
};

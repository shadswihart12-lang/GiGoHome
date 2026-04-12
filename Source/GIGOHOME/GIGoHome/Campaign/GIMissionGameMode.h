// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Base Campaign Mission Game Mode
// Extends AGIGoHomeGameMode with objective tracking, choices, timers, narrative moments

#pragma once

#include "CoreMinimal.h"
#include "GIGoHome/GIGoHomeGameMode.h"
#include "GIGoHome/Campaign/GICampaignTypes.h"
#include "GIMissionGameMode.generated.h"

class UGINarrativeDirector;
class AGICampaignNPC;
class UGISlateHUDManager;
class AActor;

/**
 * Base game mode for all campaign missions in GI Go Home
 * 
 * Provides:
 * - Mission objective tracking (hidden from player per GDD)
 * - Narrative choice presentation and recording
 * - Mission timer support (Mission 3: 12-minute hold)
 * - Narrative moment queue (cutscenes, scripted camera, fade)
 * - NPC spawning and management
 * - Campaign state integration
 * 
 * Subclass per-mission to configure objectives, NPCs, choices, and mechanics.
 * All logic is pure C++. Cutscene content is authored in Sequencer (timeline tool).
 */
UCLASS(abstract)
class GIGOHOME_API AGIMissionGameMode : public AGIGoHomeGameMode
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category="Campaign")
	FOnObjectiveCompleted OnObjectiveCompleted;

	UPROPERTY(BlueprintAssignable, Category="Campaign")
	FOnObjectiveUpdated OnObjectiveUpdated;

	UPROPERTY(BlueprintAssignable, Category="Campaign")
	FOnChoiceMade OnChoiceMade;

	UPROPERTY(BlueprintAssignable, Category="Campaign")
	FOnMissionStarted OnMissionStarted;

	UPROPERTY(BlueprintAssignable, Category="Campaign")
	FOnMissionEnded OnMissionEnded;

	UPROPERTY(BlueprintAssignable, Category="Campaign")
	FOnTimerTick OnTimerTick;

protected:

	/** Which mission this game mode represents */
	UPROPERTY(EditAnywhere, Category="Mission")
	ECampaignMission MissionID = ECampaignMission::None;

	/** Mission objectives — tracked internally, hidden from player per GDD */
	UPROPERTY(EditAnywhere, Category="Mission")
	TArray<FGIMissionObjective> Objectives;

	/** Narrative choices available in this mission */
	UPROPERTY(EditAnywhere, Category="Mission")
	TArray<FMissionChoice> Choices;

	/** Narrative moments (cutscenes) queued for this mission */
	UPROPERTY(EditAnywhere, Category="Mission")
	TArray<FNarrativeMoment> NarrativeMoments;

	/** NPC spawn configurations */
	UPROPERTY(EditAnywhere, Category="Mission")
	TArray<FMissionNPCSpawn> NPCSpawns;

	/** Spawned NPCs tracked by tag */
	UPROPERTY()
	TMap<FName, AGICampaignNPC*> SpawnedNPCs;

	/** Narrative director for cutscenes and camera */
	UPROPERTY()
	UGINarrativeDirector* NarrativeDirector;

	/** Slate HUD for minimal feedback */
	UPROPERTY()
	UGISlateHUDManager* SlateHUD;

	// --- Timer ---

	/** Whether this mission uses a survival timer */
	UPROPERTY(EditAnywhere, Category="Mission|Timer")
	bool bUseMissionTimer = false;

	/** Total timer duration in seconds */
	UPROPERTY(EditAnywhere, Category="Mission|Timer", meta=(EditCondition="bUseMissionTimer", ClampMin=1.0f))
	float MissionTimerDuration = 720.0f;  // 12 minutes default

	/** Current remaining time */
	float MissionTimerRemaining = 0.0f;

	/** Timer tick handle */
	FTimerHandle MissionTimerHandle;

	/** Mission state */
	bool bMissionActive = false;
	bool bMissionComplete = false;
	bool bMissionFailed = false;

	/** Mission start time for telemetry */
	double MissionStartTimeSeconds = 0.0;

	/** Publisher demo mode controls */
	UPROPERTY(EditAnywhere, Category="Publisher Demo")
	bool bEnablePublisherDemoMode = true;

	/** Hard session cap for a publisher hands-on playthrough */
	UPROPERTY(EditAnywhere, Category="Publisher Demo", meta=(EditCondition="bEnablePublisherDemoMode", ClampMin=60.0f, Units="s"))
	float DemoSessionTimeLimitSeconds = 1200.0f;

	/** Timer for publisher demo session cap */
	FTimerHandle DemoSessionTimerHandle;

	// --- Difficulty modifier from prior choices ---
	UPROPERTY(EditAnywhere, Category="Mission")
	float DifficultyModifier = 1.0f;

public:

	AGIMissionGameMode();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnPlayerDeath() override;

	// --- Mission Flow ---

	/** Starts the mission — spawns NPCs, plays opening narrative, activates objectives */
	UFUNCTION(BlueprintCallable, Category="Campaign")
	virtual void StartMission();

	/** Ends the mission — records choices, plays closing narrative */
	UFUNCTION(BlueprintCallable, Category="Campaign")
	virtual void EndMission(bool bSuccess);

	/** Restarts current map for rapid publisher demo iteration */
	UFUNCTION(BlueprintCallable, Category="Publisher Demo")
	void RestartCurrentMission();

	/** Console command alias for restart flow */
	UFUNCTION(Exec)
	void DemoRestartMission();

	// --- Objectives ---

	/** Updates progress on an objective by tag */
	UFUNCTION(BlueprintCallable, Category="Campaign")
	void UpdateObjectiveProgress(FName ObjectiveTag, int32 ProgressDelta = 1);

	/** Directly completes an objective by tag */
	UFUNCTION(BlueprintCallable, Category="Campaign")
	void CompleteObjective(FName ObjectiveTag);

	/** Fails the mission — called when critical objective becomes impossible */
	UFUNCTION(BlueprintCallable, Category="Campaign")
	void FailMission();

	/** Returns true if all required objectives are complete */
	UFUNCTION(BlueprintPure, Category="Campaign")
	bool AreRequiredObjectivesComplete() const;

	/** Returns objective by tag (nullptr if not found) */
	FGIMissionObjective* FindObjective(FName ObjectiveTag);

	/** Returns true when objective exists and is complete */
	bool IsObjectiveCompleted(FName ObjectiveTag) const;

	// --- Choices ---

	/** Records a choice made by the player */
	UFUNCTION(BlueprintCallable, Category="Campaign")
	void MakeChoice(FName ChoiceTag, EAct1Choice ChosenOption);

	/** Returns true if a specific choice has been made */
	UFUNCTION(BlueprintPure, Category="Campaign")
	bool HasChoiceBeenMade(FName ChoiceTag) const;

	/** Returns the chosen option for a choice tag */
	UFUNCTION(BlueprintPure, Category="Campaign")
	EAct1Choice GetChoiceResult(FName ChoiceTag) const;

	// --- Timer ---

	/** Starts the mission countdown timer */
	UFUNCTION(BlueprintCallable, Category="Campaign")
	void StartMissionTimer();

	/** Stops the mission timer */
	UFUNCTION(BlueprintCallable, Category="Campaign")
	void StopMissionTimer();

	/** Returns remaining timer seconds */
	UFUNCTION(BlueprintPure, Category="Campaign")
	float GetTimerRemaining() const { return MissionTimerRemaining; }

	// --- Narrative ---

	/** Plays a narrative moment by tag */
	UFUNCTION(BlueprintCallable, Category="Campaign")
	void PlayNarrativeMoment(FName MomentTag);

	/** Triggers a fade to black (for transitions) */
	UFUNCTION(BlueprintCallable, Category="Campaign")
	void FadeToBlack(float Duration = 1.0f);

	/** Triggers a fade from black */
	UFUNCTION(BlueprintCallable, Category="Campaign")
	void FadeFromBlack(float Duration = 1.0f);

	/** Update persistent ammo HUD readout */
	UFUNCTION(BlueprintCallable, Category="Campaign")
	void ShowAmmoCheck(int32 CurrentAmmo, int32 MagazineSize);

	// --- NPC Access ---

	/** Returns a spawned NPC by tag */
	UFUNCTION(BlueprintPure, Category="Campaign")
	AGICampaignNPC* GetNPCByTag(FName NPCTag) const;

	/** Resolves NPC by tag from spawned cache, then world tag fallback */
	AGICampaignNPC* ResolveMissionNPC(FName NPCTag) const;

	/** Finds a world actor by tag with optional class filter */
	AActor* FindWorldActorByTag(FName Tag, TSubclassOf<AActor> RequiredClass = nullptr) const;

	/** Returns the narrative director */
	UGINarrativeDirector* GetNarrativeDirector() const { return NarrativeDirector; }

protected:

	/** Override in subclass to configure objectives, NPCs, choices before mission start */
	virtual void SetupMission();

	/** Override for mission-specific start logic */
	virtual void OnMissionBegin() {}

	/** Override for mission-specific end logic */
	virtual void OnMissionEnd(bool bSuccess) {}

	/** Override for mission-specific timer expired logic */
	virtual void OnMissionTimerExpired();

	/** Spawns all configured NPCs */
	void SpawnMissionNPCs();

	/** Cleans up spawned NPCs */
	void CleanupMissionNPCs();

	/** Creates the narrative director */
	void CreateNarrativeDirector();

	/** Creates the Slate HUD */
	void CreateMissionHUD();

	/** Timer tick callback */
	void TickMissionTimer();

	/** Checks if all objectives are done and auto-completes mission */
	void CheckMissionCompletion();

	/** Applies campaign choice consequences from previous missions */
	virtual void ApplyPriorChoiceConsequences();

	/** Starts publisher demo timeout timer */
	void StartDemoSessionTimer();

	/** Stops publisher demo timeout timer */
	void StopDemoSessionTimer();

	/** Called when publisher demo timeout expires */
	void OnDemoSessionTimeout();

	/** Logs telemetry events for publisher review */
	void LogTelemetryEvent(const FString& EventName) const;
};

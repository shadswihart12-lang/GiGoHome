// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GIGoHome/Campaign/GICampaignTypes.h"
#include "GIGoHomeCampaignState.generated.h"

/** Campaign acts matching the GDD structure */
UENUM(BlueprintType)
enum class ECampaignAct : uint8
{
	Act1_TheAsh,
	Act2_TheSquad,
	Act3_TheLongWar
};

/** Narrative choices made by the player - determines ending */
UENUM(BlueprintType)
enum class EPlayerChoice : uint8
{
	None,
	SparedSurrenderedSoldier,
	KilledSurrenderedSoldier,
	SabotagedRadioWithoutKilling,
	LetSquadMemberLeadRiskyAttack,
	ChoseToKeepFighting,
  ChoseToDisappearIntoJungle,
	// Act 1 mission branch records
	M1_KilledSleeping,
	M1_WaitedForDawn,
	M2_AttackedListeningPost,
	M2_CompletedObservation,
	M3_ChasedRunner,
	M3_DestroyedBackupRadio,
	M4_KilledTylerGreg,
	M4_SparedTylerGreg,
	M5_FirebaseComplete
};

/**
 * Persistent campaign state for GI Go Home
 * Tracks act progression, player choices, and squad member status
 * These decisions affect the final ending
 */
UCLASS()
class GIGOHOME_API AGIGoHomeCampaignState : public AGameStateBase
{
	GENERATED_BODY()

public:

	/** Current act of the campaign */
	UPROPERTY(BlueprintReadOnly, Category="Campaign")
	ECampaignAct CurrentAct = ECampaignAct::Act1_TheAsh;

	/** All choices made by the player throughout the campaign */
	UPROPERTY(BlueprintReadOnly, Category="Campaign")
	TArray<EPlayerChoice> PlayerChoices;

	/** Member tags of squad members permanently killed */
	UPROPERTY(BlueprintReadOnly, Category="Campaign")
	TArray<FName> PermanentlyDeadSquadMembers;

	/** Member tags of squad members currently wounded (unavailable this mission) */
	UPROPERTY(BlueprintReadOnly, Category="Campaign")
	TArray<FName> WoundedSquadMembers;

	/** Publisher telemetry: total mission starts */
	UPROPERTY(BlueprintReadOnly, Category="Campaign|Telemetry")
	int32 TotalMissionAttempts = 0;

	/** Publisher telemetry: total completed missions */
	UPROPERTY(BlueprintReadOnly, Category="Campaign|Telemetry")
	int32 TotalMissionSuccesses = 0;

	/** Publisher telemetry: total failed missions */
	UPROPERTY(BlueprintReadOnly, Category="Campaign|Telemetry")
	int32 TotalMissionFailures = 0;

	/** Publisher telemetry: total manual restarts */
	UPROPERTY(BlueprintReadOnly, Category="Campaign|Telemetry")
	int32 TotalMissionRestarts = 0;

	/** Last mission duration in seconds */
	UPROPERTY(BlueprintReadOnly, Category="Campaign|Telemetry")
	float LastMissionDurationSeconds = 0.0f;

	/** Last mission id for telemetry */
	UPROPERTY(BlueprintReadOnly, Category="Campaign|Telemetry")
	ECampaignMission LastMissionId = ECampaignMission::None;

public:

	/** Advances to the next campaign act */
	UFUNCTION(BlueprintCallable, Category="Campaign")
	void AdvanceAct();

	/** Records a narrative player choice */
	UFUNCTION(BlueprintCallable, Category="Campaign")
	void RecordChoice(EPlayerChoice Choice);

	/** Records mission start telemetry */
	UFUNCTION(BlueprintCallable, Category="Campaign|Telemetry")
	void RecordMissionStarted(ECampaignMission MissionId);

	/** Records mission completion telemetry */
	UFUNCTION(BlueprintCallable, Category="Campaign|Telemetry")
	void RecordMissionEnded(ECampaignMission MissionId, bool bSuccess, float DurationSeconds);

	/** Records a manual mission restart event */
	UFUNCTION(BlueprintCallable, Category="Campaign|Telemetry")
	void RecordMissionRestarted(ECampaignMission MissionId);

	/** Returns a concise publisher-ready telemetry summary */
	UFUNCTION(BlueprintPure, Category="Campaign|Telemetry")
	FString GetTelemetrySummary() const;

	/** Clears telemetry counters for a fresh demo session */
	UFUNCTION(BlueprintCallable, Category="Campaign|Telemetry")
	void ResetTelemetry();

	/** Marks a squad member as permanently dead */
	UFUNCTION(BlueprintCallable, Category="Campaign")
	void MarkSquadMemberDead(FName MemberTag);

	/** Marks a squad member as wounded - recovers between missions */
	UFUNCTION(BlueprintCallable, Category="Campaign")
	void MarkSquadMemberWounded(FName MemberTag);

	/** Clears wounded status for a squad member (recovered between missions) */
	UFUNCTION(BlueprintCallable, Category="Campaign")
	void ClearSquadMemberWound(FName MemberTag);

	/** Returns true if the player has made the given choice */
	UFUNCTION(BlueprintPure, Category="Campaign")
	bool HasMadeChoice(EPlayerChoice Choice) const;

	/** Returns true if the squad member is permanently dead */
	UFUNCTION(BlueprintPure, Category="Campaign")
	bool IsSquadMemberDead(FName MemberTag) const;

	/**
	 * Calculates the ending variant index based on player choices (0-2):
	 * 0 = Xi Dong walks away into the jungle
	 * 1 = Xi Dong keeps fighting (default)
	 * 2 = Ambiguous - player crossed both sides of the line
	 */
	UFUNCTION(BlueprintPure, Category="Campaign")
	int32 GetEndingIndex() const;
};

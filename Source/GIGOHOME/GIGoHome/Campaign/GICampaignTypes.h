// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Campaign Type Definitions
// All shared enums, structs, and delegates for the Act 1 campaign system

#pragma once

#include "CoreMinimal.h"
#include "GICampaignTypes.generated.h"

class ULevelSequence;
class AGICampaignNPC;

// ============================================================================
// MISSION IDENTIFIERS
// ============================================================================

UENUM(BlueprintType)
enum class ECampaignMission : uint8
{
	None,
	// Act 1 - The Ash: Xi Dong alone, the jungle teaches him or kills him
	Act1_Mission1_TheProving,       // Knife + crossbow. Kill radio operator. Destroy radio. Extract.
	Act1_Mission2_TheObservation,   // First partner. Observe listening post. Map shifts. Extract.
	Act1_Mission3_TheDistraction,   // 12-minute hold. Hit and run. Prevent radio contact.
	Act1_Mission4_TylerGreg,        // Travel mission. Firebase perimeter. The drainage ditch.
	Act1_Mission5_FirebaseKestrel   // First squad. Infiltrate. Extract Briggs. Reach tunnel.
};

// ============================================================================
// MISSION OBJECTIVE TYPES
// ============================================================================

UENUM(BlueprintType)
enum class EGIMissionObjectiveType : uint8
{
	Eliminate,          // Kill a specific target or group
	Destroy,            // Destroy a specific object (radio, equipment)
	ReachLocation,      // Reach a specific point in the level
	Observe,            // Stay at observation point for required time
	SurviveTimer,       // Survive for a duration (Mission 3: 12 minutes)
	Extract,            // Return to extraction point after completing objectives
	Escort,             // Escort an NPC to a location (Mission 5: Briggs)
	PreventAction,      // Stop enemy from reaching a goal (radio contact)
	Custom              // Mission-specific logic
};

// ============================================================================
// NARRATIVE CHOICE IDENTIFIERS — ACT 1
// ============================================================================

UENUM(BlueprintType)
enum class EAct1Choice : uint8
{
	None,

	// Mission 1: Radio operator is sleeping. Kill him asleep or wait for dawn.
	M1_KilledSleeping,
	M1_WaitedForDawn,

	// Mission 2: Partner suggests attacking. Complete observation or push forward.
	M2_AttackedListeningPost,
	M2_CompletedObservation,

	// Mission 3: Runner breaks for backup radio. Chase or find alternate route.
	M3_ChasedRunner,
	M3_DestroyedBackupRadio,

	// Mission 4: Tyler Greg in the drainage ditch. Kill or spare.
	M4_KilledTylerGreg,
	M4_SparedTylerGreg,

	// Mission 5: No embedded player choice — completing firebase is the milestone
	M5_FirebaseComplete
};

// ============================================================================
// NPC STATES — for scripted narrative behavior
// ============================================================================

UENUM(BlueprintType)
enum class ECampaignNPCState : uint8
{
	Idle,
	Sleeping,
	Alert,
	WritingLetter,   // Mission 2: Soldier on crate outside listening post
	Fleeing,
	Patrolling,
	Escorted,        // Mission 5: Briggs hooded and bound
	Dead
};

// ============================================================================
// MISSION OBJECTIVE DATA
// ============================================================================

USTRUCT(BlueprintType)
struct FGIMissionObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGIMissionObjectiveType Type = EGIMissionObjectiveType::Custom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ObjectiveTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentProgress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredProgress = 1;

	UPROPERTY(BlueprintReadOnly)
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOptional = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHidden = true;  // Per GDD: no UI objectives displayed
};

// ============================================================================
// NARRATIVE MOMENT — a scripted sequence (cutscene, camera, dialogue)
// ============================================================================

USTRUCT(BlueprintType)
struct FNarrativeMoment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MomentTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<ULevelSequence> Sequence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SubtitleText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSkippable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDisablePlayerInput = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeInDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeOutDuration = 0.5f;
};

// ============================================================================
// NPC SPAWN CONFIGURATION
// ============================================================================

USTRUCT(BlueprintType)
struct FMissionNPCSpawn
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AGICampaignNPC> NPCClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform SpawnTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NPCTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECampaignNPCState InitialState = ECampaignNPCState::Idle;
};

// ============================================================================
// MISSION CHOICE — presented to the player at specific moments
// ============================================================================

USTRUCT(BlueprintType)
struct FMissionChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChoiceTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAct1Choice ChoiceA = EAct1Choice::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAct1Choice ChoiceB = EAct1Choice::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DescriptionA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DescriptionB;

	UPROPERTY(BlueprintReadOnly)
	bool bPresented = false;

	UPROPERTY(BlueprintReadOnly)
	EAct1Choice ChosenOption = EAct1Choice::None;
};

// ============================================================================
// DELEGATES
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompleted, const FGIMissionObjective&, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveUpdated, const FGIMissionObjective&, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChoiceMade, EAct1Choice, Choice);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMissionStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionEnded, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarrativeMomentStarted, FName, MomentTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarrativeMomentEnded, FName, MomentTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerTick, float, RemainingSeconds);

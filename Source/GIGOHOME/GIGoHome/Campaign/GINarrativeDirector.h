// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Narrative Director
// Manages cutscene playback, camera transitions, and subtitle display

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GIGoHome/Campaign/GICampaignTypes.h"
#include "GINarrativeDirector.generated.h"

class ULevelSequence;
class ULevelSequencePlayer;
class ALevelSequenceActor;
class ACameraActor;

/**
 * Manages narrative sequences for campaign missions
 * 
 * Handles:
 * - Level Sequence playback (authored in Sequencer timeline tool)
 * - Camera transitions between gameplay and cinematic cameras
 * - Fade to/from black transitions
 * - Subtitle text display via Slate
 * - Input disable/enable during cutscenes
 * 
 * Owned by AGIMissionGameMode. No Blueprint dependency.
 */
UCLASS()
class GIGOHOME_API UGINarrativeDirector : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category="Narrative")
	FOnNarrativeMomentStarted OnMomentStarted;

	UPROPERTY(BlueprintAssignable, Category="Narrative")
	FOnNarrativeMomentEnded OnMomentEnded;

protected:

	/** World reference */
	TWeakObjectPtr<UWorld> CachedWorld;

	/** Currently playing Level Sequence */
	UPROPERTY()
	ULevelSequencePlayer* ActiveSequencePlayer;

	UPROPERTY()
	ALevelSequenceActor* ActiveSequenceActor;

	/** Currently active narrative moment */
	FNarrativeMoment ActiveMoment;

	/** Whether a moment is currently playing */
	bool bIsPlaying = false;

	/** Timer for duration-based moments (no sequence) */
	FTimerHandle DurationTimer;

	/** Subtitle Slate widget (added/removed from viewport) */
	TSharedPtr<SWidget> SubtitleWidget;

public:

	/** Initialize with world reference */
	void Initialize(UWorld* InWorld);

	/** Play a narrative moment (sequence + subtitle + fade) */
	void PlayMoment(const FNarrativeMoment& Moment);

	/** Stop current moment immediately */
	void StopCurrentMoment();

	/** Returns true if a narrative moment is currently playing */
	bool IsPlaying() const { return bIsPlaying; }

	/** Blend camera to a specific camera actor over duration */
	void BlendToCamera(ACameraActor* TargetCamera, float BlendTime = 1.0f);

	/** Return camera to player character */
	void ReturnCameraToPlayer(float BlendTime = 0.5f);

	/** Show subtitle text on screen */
	void ShowSubtitle(const FText& Text);

	/** Hide current subtitle */
	void HideSubtitle();

protected:

	/** Called when a Level Sequence finishes playing */
	void OnSequenceFinished();

	/** Called when a duration-based moment expires */
	void OnDurationExpired();

	/** Common cleanup after any moment ends */
	void FinishMoment();

	/** Disable/enable player input */
	void SetPlayerInputEnabled(bool bEnabled);

	/** Creates the subtitle Slate widget */
	void CreateSubtitleWidget(const FText& Text);

	/** Removes the subtitle widget from viewport */
	void RemoveSubtitleWidget();
};

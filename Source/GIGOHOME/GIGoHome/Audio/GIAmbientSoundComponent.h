// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GIAmbientSoundComponent.generated.h"

class UAudioComponent;

/**
 * Manages ambient sound playback for GI Go Home levels.
 * Attach to any actor (e.g. game mode default pawn, player character, or a placed actor)
 * to have ambient sounds play when the level begins.
 *
 * Supports a looping ambient track and a set of randomized one-shot stingers
 * that fire at random intervals to bring the Vietnam jungle to life.
 */
UCLASS(ClassGroup=(GIGoHome), meta=(BlueprintSpawnableComponent))
class GIGOHOME_API UGIAmbientSoundComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UGIAmbientSoundComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

protected:

	/** Main looping ambient track (jungle atmosphere) */
	UPROPERTY(EditAnywhere, Category="Ambient|Loop")
	USoundBase* AmbientLoopSound = nullptr;

	/** Volume for the ambient loop */
	UPROPERTY(EditAnywhere, Category="Ambient|Loop", meta=(ClampMin=0.0f, ClampMax=2.0f))
	float AmbientLoopVolume = 0.5f;

	/** One-shot stinger sounds played at random intervals (bird calls, twig snaps, etc.) */
	UPROPERTY(EditAnywhere, Category="Ambient|Stingers")
	TArray<USoundBase*> StingerSounds;

	/** Minimum delay between stingers in seconds */
	UPROPERTY(EditAnywhere, Category="Ambient|Stingers", meta=(ClampMin=1.0f, Units="s"))
	float StingerMinInterval = 4.0f;

	/** Maximum delay between stingers in seconds */
	UPROPERTY(EditAnywhere, Category="Ambient|Stingers", meta=(ClampMin=1.0f, Units="s"))
	float StingerMaxInterval = 15.0f;

	/** Volume for stinger one-shots */
	UPROPERTY(EditAnywhere, Category="Ambient|Stingers", meta=(ClampMin=0.0f, ClampMax=2.0f))
	float StingerVolume = 0.6f;

	/** Maximum random distance from the listener to spawn stinger sounds */
	UPROPERTY(EditAnywhere, Category="Ambient|Stingers", meta=(ClampMin=0.0f, Units="cm"))
	float StingerSpawnRadius = 2000.0f;

private:

	UPROPERTY()
	UAudioComponent* LoopAudioComponent = nullptr;

	FTimerHandle StingerTimer;

	void PlayRandomStinger();
	void ScheduleNextStinger();
};

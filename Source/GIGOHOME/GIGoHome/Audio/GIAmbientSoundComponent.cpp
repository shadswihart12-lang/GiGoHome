// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Audio/GIAmbientSoundComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"

UGIAmbientSoundComponent::UGIAmbientSoundComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGIAmbientSoundComponent::BeginPlay()
{
	Super::BeginPlay();

	// Start ambient loop
	if (AmbientLoopSound)
	{
		LoopAudioComponent = UGameplayStatics::SpawnSound2D(this, AmbientLoopSound, AmbientLoopVolume);
		if (LoopAudioComponent)
		{
			LoopAudioComponent->bAutoDestroy = false;
			LoopAudioComponent->bIsUISound = true;
		}
	}

	// Schedule first stinger
	if (StingerSounds.Num() > 0)
	{
		ScheduleNextStinger();
	}
}

void UGIAmbientSoundComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (LoopAudioComponent)
	{
		LoopAudioComponent->Stop();
		LoopAudioComponent->DestroyComponent();
		LoopAudioComponent = nullptr;
	}

	GetWorld()->GetTimerManager().ClearTimer(StingerTimer);
}

void UGIAmbientSoundComponent::PlayRandomStinger()
{
	if (StingerSounds.Num() == 0) return;

	const int32 Index = FMath::RandRange(0, StingerSounds.Num() - 1);
	USoundBase* Sound = StingerSounds[Index];

	if (Sound)
	{
		// Play near the listener with random offset for spatial variety
		APawn* ListenerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		FVector PlayLocation = GetOwner()->GetActorLocation();
		if (ListenerPawn)
		{
			PlayLocation = ListenerPawn->GetActorLocation();
		}

		const FVector Offset = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(100.0f, StingerSpawnRadius);
		PlayLocation += Offset;

		UGameplayStatics::PlaySoundAtLocation(this, Sound, PlayLocation, StingerVolume);
	}

	ScheduleNextStinger();
}

void UGIAmbientSoundComponent::ScheduleNextStinger()
{
	const float Delay = FMath::RandRange(StingerMinInterval, StingerMaxInterval);
	GetWorld()->GetTimerManager().SetTimer(StingerTimer, this, &UGIAmbientSoundComponent::PlayRandomStinger, Delay, false);
}

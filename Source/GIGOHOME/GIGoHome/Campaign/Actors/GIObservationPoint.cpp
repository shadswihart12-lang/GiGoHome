// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Observation Point Implementation

#include "GIGoHome/Campaign/Actors/GIObservationPoint.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

AGIObservationPoint::AGIObservationPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	ObservationVolume = CreateDefaultSubobject<USphereComponent>(TEXT("ObservationVolume"));
	ObservationVolume->SetupAttachment(Root);
	ObservationVolume->SetSphereRadius(ObservationRadius);
	ObservationVolume->SetCollisionProfileName(TEXT("OverlapAll"));
	ObservationVolume->SetGenerateOverlapEvents(true);

	// Invisible trigger — per GDD: no UI markers, learn by doing
	ObservationVolume->SetHiddenInGame(true);
}

void AGIObservationPoint::BeginPlay()
{
	Super::BeginPlay();

	ObservationVolume->SetSphereRadius(ObservationRadius);

	ObservationVolume->OnComponentBeginOverlap.AddDynamic(
		this, &AGIObservationPoint::OnVolumeOverlapBegin);
	ObservationVolume->OnComponentEndOverlap.AddDynamic(
		this, &AGIObservationPoint::OnVolumeOverlapEnd);
}

void AGIObservationPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCompleted || !bPlayerInVolume) return;

	if (IsPlayerValidForObservation())
	{
		CurrentObservationTime += DeltaTime;
		OnObservationProgress.Broadcast(this, GetProgress());

		if (CurrentObservationTime >= RequiredObservationTime)
		{
			bCompleted = true;
			SetActorTickEnabled(false);
			OnObservationComplete.Broadcast(this);
		}
	}
}

float AGIObservationPoint::GetProgress() const
{
	if (RequiredObservationTime <= 0.0f) return 1.0f;
	return FMath::Clamp(CurrentObservationTime / RequiredObservationTime, 0.0f, 1.0f);
}

void AGIObservationPoint::OnVolumeOverlapBegin(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (bCompleted) return;

	if (OtherActor && OtherActor->ActorHasTag(FName("Player")))
	{
		bPlayerInVolume = true;
		SetActorTickEnabled(true);
	}
}

void AGIObservationPoint::OnVolumeOverlapEnd(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->ActorHasTag(FName("Player")))
	{
		bPlayerInVolume = false;
		// Don't disable tick immediately — observation pauses but doesn't reset
	}
}

bool AGIObservationPoint::IsPlayerValidForObservation() const
{
	if (!bRequireCrouch) return true;

	// Find the player character and check if crouching
	for (TObjectIterator<ACharacter> It; It; ++It)
	{
		if (It->ActorHasTag(FName("Player")))
		{
			return It->GetCharacterMovement()->IsCrouching();
		}
	}

	return false;
}

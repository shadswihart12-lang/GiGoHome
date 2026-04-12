// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Foliage/FoliageCoverVolume.h"
#include "GIGoHome/Player/Components/StealthComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"

AFoliageCoverVolume::AFoliageCoverVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	CoverTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("CoverTrigger"));
	RootComponent = CoverTrigger;

	// Default size for a foliage cluster
	CoverTrigger->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
	CoverTrigger->SetCollisionProfileName(FName("Trigger"));
	CoverTrigger->SetGenerateOverlapEvents(true);
}

void AFoliageCoverVolume::BeginPlay()
{
	Super::BeginPlay();

	CoverTrigger->OnComponentBeginOverlap.AddDynamic(this, &AFoliageCoverVolume::OnCoverBeginOverlap);
	CoverTrigger->OnComponentEndOverlap.AddDynamic(this, &AFoliageCoverVolume::OnCoverEndOverlap);
}

void AFoliageCoverVolume::OnCoverBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || !OtherActor->ActorHasTag(PlayerTag))
	{
		return;
	}

	// Find stealth component on the player
	UStealthComponent* StealthComp = OtherActor->FindComponentByClass<UStealthComponent>();
	if (StealthComp)
	{
		StealthComp->SetInFoliageCover(true);
	}
}

void AFoliageCoverVolume::OnCoverEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!OtherActor || !OtherActor->ActorHasTag(PlayerTag))
	{
		return;
	}

	UStealthComponent* StealthComp = OtherActor->FindComponentByClass<UStealthComponent>();
	if (StealthComp)
	{
		StealthComp->SetInFoliageCover(false);
	}
}

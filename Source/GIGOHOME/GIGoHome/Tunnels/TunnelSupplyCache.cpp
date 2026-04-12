// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Tunnels/TunnelSupplyCache.h"
#include "GIGoHome/Player/XiDongCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"

ATunnelSupplyCache::ATunnelSupplyCache()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	CacheMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CacheMesh"));
	CacheMesh->SetupAttachment(Root);

	InteractionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(Root);
	InteractionVolume->SetSphereRadius(120.0f);
	InteractionVolume->SetCollisionProfileName(FName("Trigger"));
}

void ATunnelSupplyCache::BeginPlay()
{
	Super::BeginPlay();

	InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &ATunnelSupplyCache::OnBeginOverlap);
	InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &ATunnelSupplyCache::OnEndOverlap);
}

int32 ATunnelSupplyCache::TakeAmmo()
{
	if (!bHasAmmo || AmmoReloadsAvailable <= 0)
	{
		return 0;
	}

	const int32 Reloads = AmmoReloadsAvailable;
	AmmoReloadsAvailable = 0;
	bHasAmmo = false;

	if (!HasSupplies())
	{
     OnCacheLootedEffect();
	}

	return Reloads;
}

float ATunnelSupplyCache::TakeMedicalSupplies()
{
	if (!bHasMedicalSupplies)
	{
		return 0.0f;
	}

	bHasMedicalSupplies = false;

	if (!HasSupplies())
	{
     OnCacheLootedEffect();
	}

	return MedicalHPRestore;
}

void ATunnelSupplyCache::OnBeginOverlap(
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

	if (HasSupplies())
	{
		// Register this cache with the player for interaction
		if (AXiDongCharacter* XiDong = Cast<AXiDongCharacter>(OtherActor))
		{
			XiDong->SetNearbySupplyCache(this);
		}

     OnPlayerInRangeEffect(OtherActor);
	}
}

void ATunnelSupplyCache::OnEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!OtherActor || !OtherActor->ActorHasTag(PlayerTag))
	{
		return;
	}

	// Unregister this cache from the player
	if (AXiDongCharacter* XiDong = Cast<AXiDongCharacter>(OtherActor))
	{
		XiDong->ClearNearbySupplyCache(this);
	}

   OnPlayerLeftRangeEffect(OtherActor);
}

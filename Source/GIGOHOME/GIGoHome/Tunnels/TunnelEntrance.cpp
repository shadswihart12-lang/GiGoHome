// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Tunnels/TunnelEntrance.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

ATunnelEntrance::ATunnelEntrance()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	EntranceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EntranceMesh"));
	EntranceMesh->SetupAttachment(Root);

	InteractionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(Root);
	InteractionVolume->SetSphereRadius(80.0f);
	InteractionVolume->SetCollisionProfileName(FName("Trigger"));
}

void ATunnelEntrance::BeginPlay()
{
	Super::BeginPlay();

	InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &ATunnelEntrance::OnInteractionVolumeOverlap);
}

void ATunnelEntrance::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(CooldownTimer);
}

bool ATunnelEntrance::CanUse() const
{
	return !bIsBlocked && !bOnCooldown && IsValid(LinkedEntrance) && !LinkedEntrance->bIsBlocked;
}

void ATunnelEntrance::BlockEntrance()
{
	bIsBlocked = true;
}

void ATunnelEntrance::OnInteractionVolumeOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!OtherActor || !CanUse())
	{
		return;
	}

	// only Xi Dong (player-tagged pawn) can use tunnels
	if (!OtherActor->ActorHasTag(PlayerTag))
	{
		return;
	}

	TeleportActorToLinkedEntrance(OtherActor);
}

void ATunnelEntrance::TeleportActorToLinkedEntrance(AActor* ActorToTeleport)
{
	if (!IsValid(LinkedEntrance) || !ActorToTeleport)
	{
		return;
	}

	// start cooldown to prevent bouncing between entrances
	bOnCooldown = true;
	LinkedEntrance->bOnCooldown = true;

	// teleport the actor to the exit entrance location
	const FVector ExitLocation = LinkedEntrance->GetActorLocation();
	const FRotator ExitRotation = LinkedEntrance->GetActorRotation();

	ActorToTeleport->TeleportTo(ExitLocation, ExitRotation);

 OnPlayerEnteredTunnelEffect(ActorToTeleport);

	// clear cooldown after delay
	GetWorld()->GetTimerManager().SetTimer(
		CooldownTimer,
		this,
		&ATunnelEntrance::ClearCooldown,
		UseCooldown,
		false
	);
}

void ATunnelEntrance::ClearCooldown()
{
	bOnCooldown = false;
	if (IsValid(LinkedEntrance))
	{
		LinkedEntrance->bOnCooldown = false;
	}
}

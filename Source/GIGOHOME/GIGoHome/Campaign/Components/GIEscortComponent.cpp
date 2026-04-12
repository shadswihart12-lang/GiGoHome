// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Escort Component Implementation

#include "GIGoHome/Campaign/Components/GIEscortComponent.h"
#include "GIGoHome/Campaign/Actors/GICampaignNPC.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UGIEscortComponent::UGIEscortComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UGIEscortComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGIEscortComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsEscorting || bReachedDestination) return;

	TickFollow(DeltaTime);
	CheckDestinationReached();
}

// ============================================================================
// ESCORT CONTROL
// ============================================================================

void UGIEscortComponent::StartEscort(AActor* Leader, const FVector& InDestination)
{
	if (!Leader) return;

	FollowTarget = Leader;
	Destination = InDestination;
	bIsEscorting = true;
	bReachedDestination = false;
	bIsStumbling = false;

	SetComponentTickEnabled(true);

	// Set initial speed
	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
	{
		OwnerChar->GetCharacterMovement()->MaxWalkSpeed = EscortSpeed;
	}

	// Schedule first stumble
	ScheduleNextStumble();
}

void UGIEscortComponent::StopEscort()
{
	bIsEscorting = false;
	SetComponentTickEnabled(false);

	GetWorld()->GetTimerManager().ClearTimer(StumbleTimer);
	GetWorld()->GetTimerManager().ClearTimer(StumbleRecoveryTimer);
}

// ============================================================================
// FOLLOW BEHAVIOR
// ============================================================================

void UGIEscortComponent::TickFollow(float DeltaTime)
{
	if (!FollowTarget.IsValid()) return;

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar) return;

	const FVector OwnerLocation = OwnerChar->GetActorLocation();
	const FVector TargetLocation = FollowTarget->GetActorLocation();
	const float DistToTarget = FVector::Dist(OwnerLocation, TargetLocation);

	// Only move if beyond follow distance
	if (DistToTarget > FollowDistance)
	{
		const FVector Direction = (TargetLocation - OwnerLocation).GetSafeNormal();
		OwnerChar->AddMovementInput(Direction, 1.0f);
	}
}

// ============================================================================
// STUMBLE
// ============================================================================

void UGIEscortComponent::ScheduleNextStumble()
{
	if (!bIsEscorting) return;

	// Randomize interval
	const float NextStumble = StumbleInterval * FMath::FRandRange(0.6f, 1.4f);

	GetWorld()->GetTimerManager().SetTimer(
		StumbleTimer, this, &UGIEscortComponent::TriggerStumble, NextStumble, false);
}

void UGIEscortComponent::TriggerStumble()
{
	if (!bIsEscorting || bIsStumbling || bReachedDestination) return;

	bIsStumbling = true;

	// Slow down
	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
	{
		OwnerChar->GetCharacterMovement()->MaxWalkSpeed = StumbleSpeed;

		// Make noise — alerting nearby enemies
		OwnerChar->MakeNoise(StumbleNoiseLoudness, OwnerChar,
			OwnerChar->GetActorLocation(), StumbleNoiseRange, FName("Stumble"));
	}

	OnEscorteeStumbled.Broadcast();

	// Schedule recovery
	GetWorld()->GetTimerManager().SetTimer(
		StumbleRecoveryTimer, this, &UGIEscortComponent::RecoverFromStumble, StumbleDuration, false);
}

void UGIEscortComponent::RecoverFromStumble()
{
	bIsStumbling = false;

	// Restore speed
	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
	{
		OwnerChar->GetCharacterMovement()->MaxWalkSpeed = EscortSpeed;
	}

	// Schedule next stumble
	ScheduleNextStumble();
}

// ============================================================================
// DESTINATION CHECK
// ============================================================================

void UGIEscortComponent::CheckDestinationReached()
{
	if (bReachedDestination) return;

	const float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Destination);

	if (Dist <= ArrivalThreshold)
	{
		bReachedDestination = true;
		StopEscort();
		OnEscortReachedDestination.Broadcast();
	}
}

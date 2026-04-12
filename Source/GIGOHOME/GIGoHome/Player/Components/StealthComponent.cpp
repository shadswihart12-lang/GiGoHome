// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Player/Components/StealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

UStealthComponent::UStealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// cache owner references
	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
	{
		CachedMovement = OwnerChar->GetCharacterMovement();
		OwnerPawn = OwnerChar;
	}

	CachedNoiseEmitter = GetOwner()->FindComponentByClass<UPawnNoiseEmitterComponent>();

	// begin periodic noise emission
	GetWorld()->GetTimerManager().SetTimer(
		NoiseEmitTimer,
		this,
		&UStealthComponent::EmitMovementNoise,
		NoiseEmitInterval,
		true
	);
}

void UStealthComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(NoiseEmitTimer);
}

void UStealthComponent::SetInFoliageCover(bool bInCover)
{
	bInFoliageCover = bInCover;
	CurrentVisibilityLevel = CalculateVisibilityLevel();
	OnStealthLevelChanged.Broadcast(CurrentNoiseLevel, CurrentVisibilityLevel);
}

void UStealthComponent::StartHoldBreath()
{
	bIsHoldingBreath = true;
}

void UStealthComponent::StopHoldBreath()
{
	bIsHoldingBreath = false;
}

void UStealthComponent::EmitMovementNoise()
{
	if (!CachedMovement || !OwnerPawn)
	{
		return;
	}

	// no noise emitted if standing still
	if (CachedMovement->Velocity.SizeSquared() < 100.0f)
	{
		if (CurrentNoiseLevel > 0.0f)
		{
			CurrentNoiseLevel = 0.0f;
			OnStealthLevelChanged.Broadcast(CurrentNoiseLevel, CurrentVisibilityLevel);
		}
		return;
	}

	CurrentNoiseLevel = CalculateNoiseLevel();
	CurrentVisibilityLevel = CalculateVisibilityLevel();

	// report noise to AI hearing perception system
	if (CachedNoiseEmitter && CurrentNoiseLevel > 0.01f)
	{
		OwnerPawn->MakeNoise(
			CurrentNoiseLevel,
			OwnerPawn,
			OwnerPawn->GetActorLocation(),
			MovementNoiseRange,
			FName("Footstep")
		);
	}

	OnStealthLevelChanged.Broadcast(CurrentNoiseLevel, CurrentVisibilityLevel);
}

float UStealthComponent::CalculateNoiseLevel() const
{
	if (!CachedMovement)
	{
		return 0.0f;
	}

	float Noise = 0.0f;

	if (CachedMovement->IsCrouching())
	{
		Noise = CrouchNoiseLoudness;
	}
	else
	{
		// compare speed to walk speed to detect sprinting
		const float Speed = CachedMovement->Velocity.Size();
		const float WalkSpeed = CachedMovement->MaxWalkSpeed;

		Noise = (Speed > WalkSpeed * 1.1f) ? SprintNoiseLoudness : WalkNoiseLoudness;
	}

	// breath hold halves noise signature
	if (bIsHoldingBreath)
	{
		Noise *= 0.5f;
	}

	// rain masks movement noise
	Noise *= (1.0f - FMath::Clamp(RainMaskFactor, 0.0f, 1.0f));

	return FMath::Clamp(Noise, 0.0f, 1.0f);
}

float UStealthComponent::CalculateVisibilityLevel() const
{
	if (!CachedMovement)
	{
		return 1.0f;
	}

	float Visibility = 1.0f;

	// dense foliage cover dramatically reduces visibility
	if (bInFoliageCover)
	{
		Visibility *= 0.25f;
	}

	// crouching reduces silhouette
	if (CachedMovement->IsCrouching())
	{
		Visibility *= 0.6f;
	}

	// movement increases visibility against foliage
	const float Speed = CachedMovement->Velocity.Size();
	if (Speed > 10.0f)
	{
		const float SpeedFactor = FMath::Clamp(Speed / CachedMovement->MaxWalkSpeed, 0.0f, 2.0f);
		Visibility = FMath::Min(1.0f, Visibility + (SpeedFactor * 0.3f));
	}

	// rain slightly reduces enemy visibility too
	Visibility *= (1.0f - RainMaskFactor * 0.3f);

	return FMath::Clamp(Visibility, 0.0f, 1.0f);
}

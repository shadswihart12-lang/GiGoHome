// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Player/GIPlayerAnimInstance.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UGIPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwningCharacter = Cast<ACharacter>(GetOwningActor());
}

void UGIPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwningCharacter)
	{
		OwningCharacter = Cast<ACharacter>(GetOwningActor());
		if (!OwningCharacter) return;
	}

	UCharacterMovementComponent* Movement = OwningCharacter->GetCharacterMovement();
	if (!Movement) return;

	// Horizontal speed from velocity
	const FVector Velocity = Movement->Velocity;
	GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();

	bIsInAir = Movement->IsFalling();
	bIsMoving = GroundSpeed > 10.0f;

	// Check death via ShooterCharacter interface
	if (AShooterCharacter* Shooter = Cast<AShooterCharacter>(OwningCharacter))
	{
		bIsDead = Shooter->IsDead();
	}
}

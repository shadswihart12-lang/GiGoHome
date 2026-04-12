// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/AI/GIEnemyAnimInstance.h"
#include "GIGoHome/AI/GIEnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UGIEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwningEnemy = Cast<AGIEnemyCharacter>(GetOwningActor());
}

void UGIEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwningEnemy)
	{
		OwningEnemy = Cast<AGIEnemyCharacter>(GetOwningActor());
		if (!OwningEnemy) return;
	}

	UCharacterMovementComponent* Movement = OwningEnemy->GetCharacterMovement();
	if (!Movement) return;

	// Horizontal speed from injected velocity
	const FVector Velocity = Movement->Velocity;
	GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();

	bIsInAir = Movement->IsFalling();
	bIsMoving = GroundSpeed > 10.0f;
	bIsDead = OwningEnemy->GetIsDead();
}

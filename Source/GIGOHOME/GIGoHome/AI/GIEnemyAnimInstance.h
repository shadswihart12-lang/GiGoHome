// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GIEnemyAnimInstance.generated.h"

/**
 * Animation instance for GI Go Home enemy soldiers.
 * Reads velocity and state directly from the owning GIEnemyCharacter
 * and exposes them as Blueprint-readable properties for blend spaces.
 */
UCLASS()
class GIGOHOME_API UGIEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** Ground speed - drives walk/run blend space */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Animation")
	float GroundSpeed = 0.0f;

	/** True when the enemy is in the air */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Animation")
	bool bIsInAir = false;

	/** True when the enemy is moving */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Animation")
	bool bIsMoving = false;

	/** True when the enemy is dead */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Animation")
	bool bIsDead = false;

private:

	UPROPERTY()
	TObjectPtr<class AGIEnemyCharacter> OwningEnemy;
};

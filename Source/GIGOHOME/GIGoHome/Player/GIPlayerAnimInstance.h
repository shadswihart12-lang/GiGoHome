// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GIPlayerAnimInstance.generated.h"

/**
 * Animation instance for the player character (Xi Dong).
 * Reads velocity and state from any ACharacter via its movement component,
 * so it works as a C++ fallback when no Animation Blueprint is available.
 */
UCLASS()
class GIGOHOME_API UGIPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** Ground speed - drives walk/run blend space */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Animation")
	float GroundSpeed = 0.0f;

	/** True when the character is in the air */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Animation")
	bool bIsInAir = false;

	/** True when the character is moving */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Animation")
	bool bIsMoving = false;

	/** True when the character is dead */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Animation")
	bool bIsDead = false;

private:

	UPROPERTY()
	TObjectPtr<class ACharacter> OwningCharacter;
};

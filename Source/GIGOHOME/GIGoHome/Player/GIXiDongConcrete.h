// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Concrete Player Character
// Pure C++ - No Blueprint required

#pragma once

#include "CoreMinimal.h"
#include "GIGoHome/Player/XiDongCharacter.h"
#include "GIXiDongConcrete.generated.h"

/**
 * Concrete C++ player character for the vertical slice.
 * Creates all Input Actions in C++ constructor.
 * Wires all meshes, animations, and defaults via ConstructorHelpers.
 * No Blueprint subclass needed.
 */
UCLASS()
class GIGOHOME_API AGIXiDongConcrete : public AXiDongCharacter
{
	GENERATED_BODY()

public:
	AGIXiDongConcrete();

protected:
	virtual void BeginPlay() override;

protected:
	/** Override to provide C++ ammo check feedback (replaces BlueprintImplementableEvent) */
	virtual void DoAmmoCheck() override;
};

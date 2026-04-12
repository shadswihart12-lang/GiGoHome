// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GIGoHome/Traps/TrapBase.h"
#include "PunjiPitTrap.generated.h"

/**
 * Punji pit trap - a covered pit of sharpened bamboo stakes
 * Enemies that fall in are damaged and have their movement severely slowed
 * The pit cover mesh is hidden on trigger to reveal the pit visually
 */
UCLASS()
class GIGOHOME_API APunjiPitTrap : public ATrapBase
{
	GENERATED_BODY()

	/** Pit cover mesh (camouflaged leaves/branches) hidden on trigger */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* PitCoverMesh;

	/** Pit mesh revealed after triggering */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* PitMesh;

protected:

	/** Movement speed multiplier applied to the victim (0.3 = 30% of normal speed) */
	UPROPERTY(EditAnywhere, Category="PunjiPit", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float MovementSlowMultiplier = 0.3f;

	/** Duration of the movement slow effect (seconds) */
	UPROPERTY(EditAnywhere, Category="PunjiPit", meta=(ClampMin=0.0f, Units="s"))
	float SlowDuration = 5.0f;

public:

	APunjiPitTrap();

protected:

	virtual void OnTriggered(AActor* TriggeringActor) override;
};

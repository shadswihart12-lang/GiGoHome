// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GIGoHome/Traps/TrapBase.h"
#include "PressureMineTrap.generated.h"

/**
 * Improvised pressure mine - buried in the ground, explodes when stepped on
 * Deals heavy damage and applies a knockback impulse to the victim
 * Can be concealed under leaves/dirt (cover mesh hidden at arm time)
 */
UCLASS()
class GIGOHOME_API APressureMineTrap : public ATrapBase
{
	GENERATED_BODY()

	/** Surface concealment mesh - hidden when armed to suggest burying */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* ConceaimentMesh;

protected:

	/** Upward and outward impulse applied to the victim on detonation */
	UPROPERTY(EditAnywhere, Category="Mine", meta=(ClampMin=0.0f, Units="cm/s"))
	float BlastImpulse = 1200.0f;

	/** Area blast radius (smaller than tripwire - more focused) */
	UPROPERTY(EditAnywhere, Category="Mine", meta=(ClampMin=0.0f, Units="cm"))
	float BlastRadius = 200.0f;

public:

	APressureMineTrap();

protected:

	/** Hide the concealment mesh when the mine arms */
	virtual void BeginPlay() override;

	virtual void OnTriggered(AActor* TriggeringActor) override;

private:

	/** Applies radial blast and impulse to the victim */
	void Detonate(AActor* DirectVictim);
};

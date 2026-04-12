// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GIGoHome/Traps/TrapBase.h"
#include "TripwireTrap.generated.h"

class UBoxComponent;

/**
 * Tripwire grenade trap
 * A wire strung across a path triggers a grenade explosion
 * Uses an elongated box trigger to simulate the wire line
 * Deals high area damage - effective in chokepoints and doorways
 */
UCLASS()
class GIGOHOME_API ATripwireTrap : public ATrapBase
{
	GENERATED_BODY()

	/** Elongated box component simulating the tripwire */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UBoxComponent* WireTrigger;

	/** Visual mesh for the grenade body */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* GrenadeMesh;

protected:

	/** Explosion radius - all actors within this range take splash damage */
	UPROPERTY(EditAnywhere, Category="Tripwire", meta=(ClampMin=0.0f, Units="cm"))
	float ExplosionRadius = 400.0f;

	/** Damage falloff at the edge of the explosion (multiplied by 0-1 based on distance) */
	UPROPERTY(EditAnywhere, Category="Tripwire", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float EdgeDamageFalloff = 0.3f;

public:

	ATripwireTrap();

protected:

	/** Bind the box trigger overlap in BeginPlay */
	virtual void BeginPlay() override;

	virtual void OnTriggered(AActor* TriggeringActor) override;

private:

	/** Called when an actor overlaps the wire trigger box */
	UFUNCTION()
	void OnWireTriggerOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	/** Applies radial damage to all actors within the explosion radius */
	void ExplodeRadial();
};

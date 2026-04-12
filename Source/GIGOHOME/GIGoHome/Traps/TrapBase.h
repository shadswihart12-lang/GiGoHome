// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrapBase.generated.h"

class USphereComponent;

/**
 * Base class for all booby traps Xi Dong can craft and place
 * Traps are placed by XiDongCharacter and arm after a brief delay
 * When triggered, they deal damage and apply a type-specific effect
 *
 * Subclasses implement OnTriggered() for specific trap behavior.
 * All traps are single-use and destroy themselves after triggering.
 */
UCLASS(abstract)
class GIGOHOME_API ATrapBase : public AActor
{
	GENERATED_BODY()

	/** Trigger volume - actors entering this area trigger the trap */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USphereComponent* TriggerVolume;

protected:

	/** Base damage dealt when this trap triggers */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trap", meta=(ClampMin=0.0f))
	float Damage = 100.0f;

	/** Damage type class applied when this trap triggers */
	UPROPERTY(EditAnywhere, Category="Trap")
	TSubclassOf<UDamageType> DamageTypeClass;

	/** Delay after placement before the trap becomes armed (seconds) */
	UPROPERTY(EditAnywhere, Category="Trap", meta=(ClampMin=0.0f, Units="s"))
	float ArmDelay = 2.0f;

	/** Tag that prevents Xi Dong from triggering his own traps */
	UPROPERTY(EditAnywhere, Category="Trap")
	FName FriendlyTag = FName("Player");

	/** If true, this trap is armed and ready to trigger */
	UPROPERTY(BlueprintReadOnly, Category="Trap")
	bool bIsArmed = false;

	/** If true, this trap has already been triggered */
	bool bHasTriggered = false;

	/** Arm delay timer */
	FTimerHandle ArmTimer;

public:

	ATrapBase();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:

	/** Returns true if this trap is armed */
	UFUNCTION(BlueprintPure, Category="Trap")
	bool IsArmed() const { return bIsArmed; }

	/** Returns the base sphere trigger volume - subclasses may disable it */
	USphereComponent* GetTriggerVolume() const { return TriggerVolume; }

protected:

	/** Arms the trap after the arm delay */
	void Arm();

	/**
	 * Called when an enemy enters the trigger volume while armed
	 * Subclasses implement this to define trap-specific behavior
	 */
	virtual void OnTriggered(AActor* TriggeringActor);

	/** Applies base damage to the triggering actor */
	void ApplyTrapDamage(AActor* Target);

	/** Called after trap triggers - override for additional effects */
	virtual void OnTrapTriggeredEffect(AActor* TriggeringActor) {}

private:

	UFUNCTION()
	void OnTriggerVolumeOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};

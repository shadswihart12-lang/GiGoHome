// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class AShooterWeapon;

/**
 * Droppable weapon pickup - spawned when enemies die or placed in the world
 * When Xi Dong overlaps, he acquires the weapon class
 * 
 * GDD: "Scavenging American weapons" - M16, M60, M79 from dead enemies
 */
UCLASS()
class GIGOHOME_API AWeaponPickup : public AActor
{
	GENERATED_BODY()

	/** Interaction trigger volume */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USphereComponent* PickupTrigger;

	/** Visual mesh for the weapon on the ground */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* WeaponMesh;

protected:

	/** The weapon class to grant when picked up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup")
	TSubclassOf<AShooterWeapon> WeaponClass;

	/** Ammo to grant (if weapon supports partial ammo) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup", meta=(ClampMin=0))
	int32 AmmoAmount = 30;

	/** Tag required on actors to pick up (default: Player) */
	UPROPERTY(EditAnywhere, Category="Pickup")
	FName PlayerTag = FName("Player");

	/** If true, destroy after pickup */
	UPROPERTY(EditAnywhere, Category="Pickup")
	bool bDestroyOnPickup = true;

	/** Sound to play on pickup */
	UPROPERTY(EditAnywhere, Category="Pickup")
	USoundBase* PickupSound = nullptr;

public:

	AWeaponPickup();

	/** Set the weapon class this pickup grants (called when spawning from dead enemy) */
	UFUNCTION(BlueprintCallable, Category="Pickup")
	void SetWeaponClass(TSubclassOf<AShooterWeapon> NewWeaponClass, int32 Ammo = 30);

	/** Returns the weapon class this pickup grants */
	UFUNCTION(BlueprintPure, Category="Pickup")
	TSubclassOf<AShooterWeapon> GetWeaponClass() const { return WeaponClass; }

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnPickupOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	/** Called when a player picks up this weapon - override for custom effects */
	virtual void OnWeaponPickedUp(AActor* Player) {}
};

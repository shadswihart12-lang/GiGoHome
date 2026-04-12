// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TunnelSupplyCache.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class AShooterCharacter;

/**
 * Underground supply cache in the Cu Chi tunnel system
 * Xi Dong can interact with these to replenish ammo and health
 * Caches can be partially looted (ammo taken but medical supplies left, etc.)
 * Once fully looted they remain empty for the rest of the mission
 */
UCLASS()
class GIGOHOME_API ATunnelSupplyCache : public AActor
{
	GENERATED_BODY()

	/** Root component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USceneComponent* Root;

	/** Visual mesh for the cache (wooden crates, wrapped supplies) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* CacheMesh;

	/** Interaction trigger volume */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USphereComponent* InteractionVolume;

protected:

	/** Number of weapon reloads this cache provides */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cache")
	int32 AmmoReloadsAvailable = 3;

	/** HP to restore when medical supplies are used */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cache")
	float MedicalHPRestore = 150.0f;

	/** If true, medical supplies are still available */
	UPROPERTY(BlueprintReadOnly, Category="Cache")
	bool bHasMedicalSupplies = true;

	/** If true, ammo is still available */
	UPROPERTY(BlueprintReadOnly, Category="Cache")
	bool bHasAmmo = true;

	/** Tag players must carry to interact */
	UPROPERTY(EditAnywhere, Category="Cache")
	FName PlayerTag = FName("Player");

public:

	ATunnelSupplyCache();

protected:

	virtual void BeginPlay() override;

public:

	/** Returns true if this cache has any supplies remaining */
	UFUNCTION(BlueprintPure, Category="Cache")
	bool HasSupplies() const { return bHasAmmo || bHasMedicalSupplies; }

	/** Called by Xi Dong to take ammo from the cache. Returns number of reloads given. */
	UFUNCTION(BlueprintCallable, Category="Cache")
	int32 TakeAmmo();

	/** Called by Xi Dong to take medical supplies. Returns HP restored. */
	UFUNCTION(BlueprintCallable, Category="Cache")
	float TakeMedicalSupplies();

 /** Called when player is within range and can interact */
	virtual void OnPlayerInRangeEffect(AActor* Player) {}

   /** Called when player left interaction range */
	virtual void OnPlayerLeftRangeEffect(AActor* Player) {}

   /** Called when cache is successfully looted */
	virtual void OnCacheLootedEffect() {}

protected:

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};

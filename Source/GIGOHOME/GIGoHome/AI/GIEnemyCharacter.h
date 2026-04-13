// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "GIEnemyCharacter.generated.h"

class AWeaponPickup;
class UGIForcedVisibilityComponent;
class UNavigationInvokerComponent;

/**
 * Four enemy archetypes from the GDD
 * Each archetype drives different AI behavior through StateTree
 */
UENUM(BlueprintType)
enum class EEnemyArchetype : uint8
{
	/** Young, nervous rookies - unpredictable, make mistakes, feel fear quickly */
	Green,
	/** Battle-hardened veterans - check corners, communicate, most dangerous */
	Veteran,
	/** Soldiers who crossed the line - burn villages, no mercy, no ambiguity */
	Broken,
	/** Commands from the rear - eliminating causes squad cohesion loss */
	Officer
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnemyMoraleChangedDelegate, float, NewMorale);

/**
 * American enemy character for GI Go Home
 * Extends ShooterNPC with archetype, fear system, morale, and radio call
 *
 * Fear: accumulated by guerrilla activity in the area. Makes patrols more
 *   cautious - higher fear = more careful movement, tighter formations.
 *   Read by StateTree for behavioral branching.
 *
 * Morale: decreases as squad members die. At low morale, enemies rout or
 *   surrender. Officers have high morale resistance.
 *
 * Radio: if bHasRadio is true and this enemy reaches a radio actor,
 *   they call for reinforcements. Eliminating the radio or this enemy
 *   before they reach it prevents the call.
 */
UCLASS(abstract)
class GIGOHOME_API AGIEnemyCharacter : public AShooterNPC
{
	GENERATED_BODY()

	/** Forces mesh visibility - solves inheritance issue from player character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UGIForcedVisibilityComponent* ForcedVisibilityComponent;

	/** Generates nav mesh tiles around this enemy so NavMesh pathfinding works */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UNavigationInvokerComponent* NavInvoker;

public:

	/** Called when morale changes - drives StateTree rout behavior */
	UPROPERTY(BlueprintAssignable, Category="Enemy")
	FEnemyMoraleChangedDelegate OnMoraleChanged;

protected:

	/** Archetype determines base AI behavioral profile */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy")
	EEnemyArchetype Archetype = EEnemyArchetype::Green;

	/**
	 * Fear level (0=unaware/normal, 1=maximally paranoid)
	 * Increased by hearing gunfire, finding bodies, or extended guerrilla activity
	 * StateTree reads this to make patrols more careful
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float FearLevel = 0.0f;

	/**
	 * Morale level (0=routing, 1=full fighting spirit)
	 * Decreased when nearby allies die
	 * At or below RoutMoraleThreshold, this enemy routes or surrenders
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float MoraleLevel = 1.0f;

	/** Morale level at or below which this enemy routes */
	UPROPERTY(EditAnywhere, Category="Enemy", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float RoutMoraleThreshold = 0.2f;

	/** How much morale is lost per nearby ally death */
	UPROPERTY(EditAnywhere, Category="Enemy", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float MoralePerDeathLoss = 0.25f;

	/** If true, this enemy can reach a radio actor to call reinforcements */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy")
	bool bHasRadio = false;

	/** If true, this enemy has successfully called for reinforcements */
	UPROPERTY(BlueprintReadOnly, Category="Enemy")
	bool bHasCalledReinforcements = false;

	/** If true, this enemy is currently routing (fleeing) */
	UPROPERTY(BlueprintReadOnly, Category="Enemy")
	bool bIsRouting = false;

	/** Radius within which nearby ally deaths reduce this enemy's morale */
	UPROPERTY(EditAnywhere, Category="Enemy", meta=(ClampMin=0.0f, Units="cm"))
	float MoraleAwarenessRadius = 2000.0f;

	/** Weapon pickup class to spawn when this enemy dies (for scavenging) */
	UPROPERTY(EditAnywhere, Category="Enemy")
	TSubclassOf<AWeaponPickup> WeaponPickupClass;

	// --- Combat Parameters (accessible to subclasses) ---

	/** How close before shooting */
	UPROPERTY(EditAnywhere, Category="Combat")
	float ShootRange = 2000.0f;

	/** How close before stopping movement */
	UPROPERTY(EditAnywhere, Category="Combat")
	float StopDistance = 300.0f;

	/** How often to shoot in seconds */
	UPROPERTY(EditAnywhere, Category="Combat")
	float ShootInterval = 1.5f;

private:

	/** Timer for shooting interval */
	float ShootTimer = 0.0f;

	/** Timer handle for stopping the burst fire */
	FTimerHandle StopShootingTimer;

protected:

	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

public:

	AGIEnemyCharacter();

	virtual void Tick(float DeltaTime) override;

	/** Returns true if this enemy is dead */
	UFUNCTION(BlueprintPure, Category="Enemy")
	bool GetIsDead() const { return bIsDead; }

	/** Returns the archetype for this enemy */
	UFUNCTION(BlueprintPure, Category="Enemy")
	EEnemyArchetype GetArchetype() const { return Archetype; }

	/** Returns current fear level (0-1) */
	UFUNCTION(BlueprintPure, Category="Enemy")
	float GetFearLevel() const { return FearLevel; }

	/** Returns current morale level (0-1) */
	UFUNCTION(BlueprintPure, Category="Enemy")
	float GetMoraleLevel() const { return MoraleLevel; }

	/** Returns true if this enemy is routing */
	UFUNCTION(BlueprintPure, Category="Enemy")
	bool IsRouting() const { return bIsRouting; }

	/** Returns true if this enemy carries a radio */
	UFUNCTION(BlueprintPure, Category="Enemy")
	bool HasRadio() const { return bHasRadio; }

	/** Returns true if reinforcements have been called */
	UFUNCTION(BlueprintPure, Category="Enemy")
	bool HasCalledReinforcements() const { return bHasCalledReinforcements; }

	/** Adds fear to this enemy (called by AI controller on hearing gunfire/bodies) */
	UFUNCTION(BlueprintCallable, Category="Enemy")
	void AddFear(float Amount);

	/** Marks this enemy as having called reinforcements */
	UFUNCTION(BlueprintCallable, Category="Enemy")
	void SetCalledReinforcements();

	/** Called by nearby dying enemies to reduce this enemy's morale */
	void NotifyNearbyAllyDied();

protected:

	/** Override death to notify nearby enemies' morale */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** Checks morale level and triggers rout if below threshold */
	void EvaluateMorale();

	/** Force mesh to be visible - called at multiple points to ensure visibility */
	void ForceMeshVisible();

	/** Called when morale drops below rout threshold - override for custom behavior */
	virtual void OnStartRouting() {}

	/** Called when fear level changes significantly - override for custom behavior */
	virtual void OnFearChanged(float NewFearLevel) {}
};

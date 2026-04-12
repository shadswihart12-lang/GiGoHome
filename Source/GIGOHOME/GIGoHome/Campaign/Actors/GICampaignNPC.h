// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Campaign NPC
// Base class for narrative NPCs in campaign missions

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GIGoHome/Campaign/GICampaignTypes.h"
#include "GICampaignNPC.generated.h"

class USphereComponent;
class UGIEscortComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCInteracted, AGICampaignNPC*, NPC, AActor*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNPCStateChanged, ECampaignNPCState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNPCKilled, AGICampaignNPC*, NPC);

/**
 * Base character for narrative NPCs in campaign missions
 * 
 * Supports scripted states:
 * - Sleeping (Mission 1: radio operator)
 * - WritingLetter (Mission 2: soldier on crate)
 * - Patrolling (firebase guards)
 * - Escorted (Mission 5: Briggs hooded and bound)
 * - Fleeing (runner in Mission 3)
 * 
 * Has interaction volume for player proximity detection.
 * Campaign NPCs are not full AI combatants — they use simple
 * state-driven behavior for narrative moments.
 */
UCLASS()
class GIGOHOME_API AGICampaignNPC : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USphereComponent* InteractionVolume;

public:

	UPROPERTY(BlueprintAssignable, Category="NPC")
	FOnNPCInteracted OnNPCInteracted;

	UPROPERTY(BlueprintAssignable, Category="NPC")
	FOnNPCStateChanged OnNPCStateChanged;

	UPROPERTY(BlueprintAssignable, Category="NPC")
	FOnNPCKilled OnNPCKilled;

protected:

	// --- Identity ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NPC|Identity")
	FText NPCName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NPC|Identity")
	FName NPCTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NPC|Identity")
	FText Description;

	// --- State ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NPC|State")
	ECampaignNPCState CurrentState = ECampaignNPCState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NPC|State")
	ECampaignNPCState InitialState = ECampaignNPCState::Idle;

	// --- Health ---

	UPROPERTY(EditAnywhere, Category="NPC|Combat", meta=(ClampMin=0.0f))
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category="NPC|Combat")
	float CurrentHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category="NPC|Combat")
	bool bIsDead = false;

	// --- Interaction ---

	UPROPERTY(EditAnywhere, Category="NPC|Interaction", meta=(ClampMin=50.0f, Units="cm"))
	float InteractionRadius = 200.0f;

	UPROPERTY(BlueprintReadOnly, Category="NPC|Interaction")
	bool bPlayerInRange = false;

	// --- Patrol ---

	UPROPERTY(EditAnywhere, Category="NPC|Patrol")
	TArray<FVector> PatrolPoints;

	UPROPERTY(EditAnywhere, Category="NPC|Patrol", meta=(ClampMin=50.0f, Units="cm/s"))
	float PatrolSpeed = 150.0f;

	int32 CurrentPatrolIndex = 0;
	FTimerHandle PatrolTimer;

	// --- Flee ---

	UPROPERTY(EditAnywhere, Category="NPC|Flee")
	FVector FleeTarget;

	UPROPERTY(EditAnywhere, Category="NPC|Flee", meta=(ClampMin=50.0f, Units="cm/s"))
	float FleeSpeed = 400.0f;

	// --- Scripted Behavior ---

	UPROPERTY(EditAnywhere, Category="NPC|Scripted")
	float WakeUpTime = 0.0f;  // Seconds after mission start to wake up (0 = never auto-wake)

	FTimerHandle WakeUpTimer;

	/** Items this NPC carries (e.g., photograph) — referenced by tag */
	UPROPERTY(EditAnywhere, Category="NPC|Items")
	TMap<FName, FText> CarriedItems;

public:

	AGICampaignNPC();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	// --- State Management ---

	UFUNCTION(BlueprintCallable, Category="NPC")
	void SetNPCState(ECampaignNPCState NewState);

	UFUNCTION(BlueprintPure, Category="NPC")
	ECampaignNPCState GetNPCState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category="NPC")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category="NPC")
	bool IsPlayerInRange() const { return bPlayerInRange; }

	// --- Actions ---

	UFUNCTION(BlueprintCallable, Category="NPC")
	void WakeUp();

	UFUNCTION(BlueprintCallable, Category="NPC")
	void StartFleeing(const FVector& Target);

	UFUNCTION(BlueprintCallable, Category="NPC")
	void Kill();

	/** Player interacts with this NPC */
	UFUNCTION(BlueprintCallable, Category="NPC")
	void InteractWith(AActor* Interactor);

	/** Returns true if NPC carries an item with the given tag */
	UFUNCTION(BlueprintPure, Category="NPC")
	bool HasItem(FName ItemTag) const;

	// --- Accessors ---

	const FText& GetNPCName() const { return NPCName; }
	FName GetNPCTag() const { return NPCTag; }

protected:

	UFUNCTION()
	void OnInteractionOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** State behavior implementations */
	virtual void OnEnterState(ECampaignNPCState State);
	virtual void OnExitState(ECampaignNPCState State);
	virtual void TickPatrol(float DeltaTime);
	virtual void TickFlee(float DeltaTime);

	void ScheduleWakeUp();
	void OnWakeUpTimerFired();
};

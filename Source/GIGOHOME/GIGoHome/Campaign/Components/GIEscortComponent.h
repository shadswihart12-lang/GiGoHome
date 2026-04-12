// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Escort Component
// Manages escort behavior for NPCs being led through hostile territory

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GIEscortComponent.generated.h"

class AGICampaignNPC;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEscorteeStumbled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEscortReachedDestination);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEscorteeDetected, AActor*, Detector);

/**
 * Component for escorting NPCs through hostile territory
 * 
 * Mission 5: Briggs is hooded, bound, and slow. He stumbles.
 * The player must manage squad positioning, guard rotations,
 * and Briggs' pace simultaneously.
 * 
 * The escort target follows the player at a set distance.
 * Periodically stumbles (makes noise, slows down).
 * Must reach the destination to complete the escort.
 */
UCLASS(ClassGroup=(GIGoHome), meta=(BlueprintSpawnableComponent))
class GIGOHOME_API UGIEscortComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category="Escort")
	FOnEscorteeStumbled OnEscorteeStumbled;

	UPROPERTY(BlueprintAssignable, Category="Escort")
	FOnEscortReachedDestination OnEscortReachedDestination;

	UPROPERTY(BlueprintAssignable, Category="Escort")
	FOnEscorteeDetected OnEscorteeDetected;

protected:

	/** The player/leader this NPC follows */
	UPROPERTY(BlueprintReadOnly, Category="Escort")
	TWeakObjectPtr<AActor> FollowTarget;

	/** Distance to maintain behind the follow target */
	UPROPERTY(EditAnywhere, Category="Escort", meta=(ClampMin=50.0f, Units="cm"))
	float FollowDistance = 250.0f;

	/** Normal escort walking speed */
	UPROPERTY(EditAnywhere, Category="Escort", meta=(ClampMin=50.0f, Units="cm/s"))
	float EscortSpeed = 180.0f;

	/** Speed while stumbling (much slower) */
	UPROPERTY(EditAnywhere, Category="Escort", meta=(ClampMin=10.0f, Units="cm/s"))
	float StumbleSpeed = 50.0f;

	/** Average interval between stumbles (seconds) */
	UPROPERTY(EditAnywhere, Category="Escort", meta=(ClampMin=5.0f, Units="s"))
	float StumbleInterval = 20.0f;

	/** Duration of a stumble (seconds) */
	UPROPERTY(EditAnywhere, Category="Escort", meta=(ClampMin=0.5f, Units="s"))
	float StumbleDuration = 2.0f;

	/** Noise loudness when stumbling */
	UPROPERTY(EditAnywhere, Category="Escort", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float StumbleNoiseLoudness = 0.6f;

	/** Noise range when stumbling (cm) */
	UPROPERTY(EditAnywhere, Category="Escort", meta=(ClampMin=0.0f, Units="cm"))
	float StumbleNoiseRange = 800.0f;

	/** Destination — escort is complete when NPC reaches this location */
	UPROPERTY(EditAnywhere, Category="Escort")
	FVector Destination;

	/** Distance threshold to count as "arrived" at destination */
	UPROPERTY(EditAnywhere, Category="Escort", meta=(ClampMin=50.0f, Units="cm"))
	float ArrivalThreshold = 200.0f;

	/** Whether escorting is currently active */
	bool bIsEscorting = false;

	/** Whether currently stumbling */
	bool bIsStumbling = false;

	/** Whether the destination has been reached */
	bool bReachedDestination = false;

	FTimerHandle StumbleTimer;
	FTimerHandle StumbleRecoveryTimer;

public:

	UGIEscortComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** Begin escorting — sets follow target and enables tick */
	UFUNCTION(BlueprintCallable, Category="Escort")
	void StartEscort(AActor* Leader, const FVector& InDestination);

	/** Stop escorting */
	UFUNCTION(BlueprintCallable, Category="Escort")
	void StopEscort();

	UFUNCTION(BlueprintPure, Category="Escort")
	bool IsEscorting() const { return bIsEscorting; }

	UFUNCTION(BlueprintPure, Category="Escort")
	bool IsStumbling() const { return bIsStumbling; }

	UFUNCTION(BlueprintPure, Category="Escort")
	bool HasReachedDestination() const { return bReachedDestination; }

protected:

	virtual void BeginPlay() override;

	void TickFollow(float DeltaTime);
	void TriggerStumble();
	void RecoverFromStumble();
	void ScheduleNextStumble();
	void CheckDestinationReached();
};

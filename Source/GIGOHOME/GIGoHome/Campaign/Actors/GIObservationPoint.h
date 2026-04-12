// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Observation Point
// Trigger volume for observation objectives (Mission 2)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GIObservationPoint.generated.h"

class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObservationComplete, AGIObservationPoint*, Point);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObservationProgress, AGIObservationPoint*, Point, float, Progress);

/**
 * Observation point for intelligence-gathering missions
 * Player must remain in the trigger volume while crouching/prone
 * for a set duration to complete the observation.
 * 
 * Mission 2: Three of these around the listening post.
 * Per GDD: Observation requires patience. Patience requires stillness.
 */
UCLASS()
class GIGOHOME_API AGIObservationPoint : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USphereComponent* ObservationVolume;

public:

	UPROPERTY(BlueprintAssignable, Category="Observation")
	FOnObservationComplete OnObservationComplete;

	UPROPERTY(BlueprintAssignable, Category="Observation")
	FOnObservationProgress OnObservationProgress;

protected:

	/** Unique tag for mission objective tracking */
	UPROPERTY(EditAnywhere, Category="Observation")
	FName PointTag;

	/** Time required to complete observation (seconds) */
	UPROPERTY(EditAnywhere, Category="Observation", meta=(ClampMin=1.0f, Units="s"))
	float RequiredObservationTime = 15.0f;

	/** Current observation progress */
	float CurrentObservationTime = 0.0f;

	/** Whether this point has been completed */
	UPROPERTY(BlueprintReadOnly, Category="Observation")
	bool bCompleted = false;

	/** Whether the player is currently in the volume */
	bool bPlayerInVolume = false;

	/** Radius of the observation area */
	UPROPERTY(EditAnywhere, Category="Observation", meta=(ClampMin=100.0f, Units="cm"))
	float ObservationRadius = 300.0f;

	/** Whether player must be crouching to observe */
	UPROPERTY(EditAnywhere, Category="Observation")
	bool bRequireCrouch = true;

	/** Description of what the player observes here */
	UPROPERTY(EditAnywhere, Category="Observation")
	FText ObservationDescription;

public:

	AGIObservationPoint();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category="Observation")
	bool IsCompleted() const { return bCompleted; }

	UFUNCTION(BlueprintPure, Category="Observation")
	float GetProgress() const;

	FName GetPointTag() const { return PointTag; }

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnVolumeOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnVolumeOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	bool IsPlayerValidForObservation() const;
};

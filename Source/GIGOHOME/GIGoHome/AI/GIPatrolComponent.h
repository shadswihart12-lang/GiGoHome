// Copyright Epic Games, Inc. All Rights Reserved.
// GIPatrolComponent — Far Cry style Vietnam patrol system
// Enemies walk routes, detect player, alert squad, search on contact loss, rout when morale breaks

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GIPatrolComponent.generated.h"

UENUM(BlueprintType)
enum class EPatrolState : uint8
{
	Patrolling,   // Walking waypoint route
	Alerted,      // Heard/saw something — moving to investigate
	Engaging,     // Has eyes on player — moving and shooting
	Searching,    // Lost player — searching last known position
	Routing,      // Morale broken — fleeing the combat area
	Dead
};

UCLASS(ClassGroup=(GIGoHome), meta=(BlueprintSpawnableComponent))
class GIGOHOME_API UGIPatrolComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UGIPatrolComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Called by GIWaveManager to assign this soldier's patrol waypoints
	void SetPatrolWaypoints(const TArray<FVector>& Waypoints);

	// Called when a nearby ally fires — puts this soldier on alert
	void OnAllyShotFired(const FVector& ShotLocation);

	// Called when this soldier's morale breaks — triggers rout
	void TriggerRout();

	// Returns current state for external queries
	EPatrolState GetState() const { return State; }

private:

	// --- Config ---

	/** How far this soldier can see the player */
	UPROPERTY(EditAnywhere, Category="Patrol")
	float SightRange = 3000.0f;

	/** Cone angle for sight (degrees half-angle) */
	UPROPERTY(EditAnywhere, Category="Patrol")
	float SightAngle = 60.0f;

	/** How far this soldier can hear gunfire */
	UPROPERTY(EditAnywhere, Category="Patrol")
	float HearingRange = 5000.0f;

	/** Walk speed while patrolling */
	UPROPERTY(EditAnywhere, Category="Patrol")
	float PatrolSpeed = 180.0f;

	/** Run speed while engaging */
	UPROPERTY(EditAnywhere, Category="Patrol")
	float EngageSpeed = 420.0f;

	/** Sprint speed while routing (fleeing) */
	UPROPERTY(EditAnywhere, Category="Patrol")
	float RoutSpeed = 550.0f;

	/** How long to search before returning to patrol */
	UPROPERTY(EditAnywhere, Category="Patrol")
	float SearchDuration = 8.0f;

	/** How close to waypoint before advancing to next */
	UPROPERTY(EditAnywhere, Category="Patrol")
	float WaypointAcceptRadius = 150.0f;

	/** How close to stop and shoot */
	UPROPERTY(EditAnywhere, Category="Patrol")
	float EngageStopDistance = 600.0f;

	/** How far to flee when routing before despawning/stopping */
	UPROPERTY(EditAnywhere, Category="Patrol")
	float RoutFleeDistance = 3000.0f;

	// --- State ---

	EPatrolState State = EPatrolState::Patrolling;

	TArray<FVector> PatrolWaypoints;
	int32 CurrentWaypointIndex = 0;

	FVector LastKnownPlayerLocation = FVector::ZeroVector;
	FVector RoutStartLocation = FVector::ZeroVector;
	FVector RoutFleeDirection = FVector::ZeroVector;
	float SearchTimer = 0.0f;

	// Patrol pause at waypoints
	float WaypointPauseTimer = 0.0f;
	bool bPausingAtWaypoint = false;

	UPROPERTY(EditAnywhere, Category="Patrol")
	float WaypointPauseDuration = 2.0f;

	/** Minimum seconds between MoveToLocation calls (avoids per-tick spam) */
	UPROPERTY(EditAnywhere, Category="Patrol")
	float MoveRequestCooldown = 0.25f;

	float LastMoveRequestTime = -999.0f;

	// Cached owner as character
	class AGIEnemyCharacter* OwnerEnemy = nullptr;

	// --- Internal ---

	void TickPatrolling(float DeltaTime);
	void TickAlerted(float DeltaTime);
	void TickEngaging(float DeltaTime);
	void TickSearching(float DeltaTime);
	void TickRouting(float DeltaTime);

	bool CanSeePlayer(APawn* Player) const;
	void MoveToward(const FVector& Target, float Speed);
	void MoveInDirection(const FVector& Direction, float Speed);
	void StopCurrentMove();
	void FaceToward(const FVector& Target);
	void FaceDirection(const FVector& Direction);
	void TransitionTo(EPatrolState NewState);
	void AlertNearbyAllies(const FVector& AlertLocation);
	FVector CalculateFleeDirection() const;
};

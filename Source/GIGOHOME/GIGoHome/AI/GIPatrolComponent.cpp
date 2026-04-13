// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/AI/GIPatrolComponent.h"
#include "GIGoHome/AI/GIEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "AIController.h"

UGIPatrolComponent::UGIPatrolComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGIPatrolComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerEnemy = Cast<AGIEnemyCharacter>(GetOwner());

	if (OwnerEnemy)
	{
		OwnerEnemy->GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

void UGIPatrolComponent::SetPatrolWaypoints(const TArray<FVector>& Waypoints)
{
	PatrolWaypoints = Waypoints;
	CurrentWaypointIndex = 0;
}

void UGIPatrolComponent::OnAllyShotFired(const FVector& ShotLocation)
{
	if (State == EPatrolState::Patrolling || State == EPatrolState::Searching)
	{
		LastKnownPlayerLocation = ShotLocation;
		TransitionTo(EPatrolState::Alerted);
	}
}

void UGIPatrolComponent::TriggerRout()
{
	// Only trigger if not already routing or dead
	if (State != EPatrolState::Routing && State != EPatrolState::Dead)
	{
		if (OwnerEnemy && OwnerEnemy->IsRouting())
		{
			TransitionTo(EPatrolState::Routing);
		}
	}
}

void UGIPatrolComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerEnemy || OwnerEnemy->GetIsDead())
	{
		State = EPatrolState::Dead;
		return;
	}

	// Check for morale-triggered rout
	if (OwnerEnemy->IsRouting() && State != EPatrolState::Routing && State != EPatrolState::Dead)
	{
		TransitionTo(EPatrolState::Routing);
		return;
	}

	switch (State)
	{
	case EPatrolState::Patrolling: TickPatrolling(DeltaTime); break;
	case EPatrolState::Alerted:    TickAlerted(DeltaTime);    break;
	case EPatrolState::Engaging:   TickEngaging(DeltaTime);   break;
	case EPatrolState::Searching:  TickSearching(DeltaTime);  break;
	case EPatrolState::Routing:    TickRouting(DeltaTime);    break;
	default: break;
	}
}

void UGIPatrolComponent::TickPatrolling(float DeltaTime)
{
	// Check for player every tick
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (Player && IsValid(Player) && CanSeePlayer(Player))
	{
		LastKnownPlayerLocation = Player->GetActorLocation();
		AlertNearbyAllies(LastKnownPlayerLocation);
		TransitionTo(EPatrolState::Engaging);
		return;
	}

	if (PatrolWaypoints.Num() == 0) return;

	// Pause at waypoint
	if (bPausingAtWaypoint)
	{
		WaypointPauseTimer -= DeltaTime;
		if (WaypointPauseTimer <= 0.0f)
		{
			bPausingAtWaypoint = false;
			CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
		}
		return;
	}

	const FVector Target = PatrolWaypoints[CurrentWaypointIndex];
	const float Dist = FVector::Dist(OwnerEnemy->GetActorLocation(), Target);

	if (Dist < WaypointAcceptRadius)
	{
		// Reached waypoint — pause before moving to next
		bPausingAtWaypoint = true;
		WaypointPauseTimer = WaypointPauseDuration;
		return;
	}

	MoveToward(Target, PatrolSpeed);
}

void UGIPatrolComponent::TickAlerted(float DeltaTime)
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (Player && IsValid(Player) && CanSeePlayer(Player))
	{
		LastKnownPlayerLocation = Player->GetActorLocation();
		AlertNearbyAllies(LastKnownPlayerLocation);
		TransitionTo(EPatrolState::Engaging);
		return;
	}

	// Move toward alert location
	const float Dist = FVector::Dist(OwnerEnemy->GetActorLocation(), LastKnownPlayerLocation);
	if (Dist < WaypointAcceptRadius)
	{
		// Reached alert location — start searching
		TransitionTo(EPatrolState::Searching);
		return;
	}

	MoveToward(LastKnownPlayerLocation, EngageSpeed * 0.7f);
}

void UGIPatrolComponent::TickEngaging(float DeltaTime)
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	// Player dead or gone
	if (!Player || !IsValid(Player))
	{
		OwnerEnemy->StopShooting();
		TransitionTo(EPatrolState::Patrolling);
		return;
	}

	// Check we still have LOS
	if (!CanSeePlayer(Player))
	{
		OwnerEnemy->StopShooting();
		SearchTimer = SearchDuration;
		LastKnownPlayerLocation = Player->GetActorLocation();
		TransitionTo(EPatrolState::Searching);
		return;
	}

	LastKnownPlayerLocation = Player->GetActorLocation();
	FaceToward(LastKnownPlayerLocation);

	const float Dist = FVector::Dist(OwnerEnemy->GetActorLocation(), LastKnownPlayerLocation);

	if (Dist > EngageStopDistance)
	{
		// Close in while shooting
		MoveToward(LastKnownPlayerLocation, EngageSpeed);
		OwnerEnemy->StartShooting(Player);
	}
	else
	{
		// In range — stand and shoot
		StopCurrentMove();
		OwnerEnemy->StartShooting(Player);
	}
}

void UGIPatrolComponent::TickSearching(float DeltaTime)
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (Player && IsValid(Player) && CanSeePlayer(Player))
	{
		LastKnownPlayerLocation = Player->GetActorLocation();
		AlertNearbyAllies(LastKnownPlayerLocation);
		TransitionTo(EPatrolState::Engaging);
		return;
	}

	// Move toward last known position
	const float Dist = FVector::Dist(OwnerEnemy->GetActorLocation(), LastKnownPlayerLocation);
	if (Dist > WaypointAcceptRadius)
	{
		MoveToward(LastKnownPlayerLocation, PatrolSpeed * 1.3f);
	}

	SearchTimer -= DeltaTime;
	if (SearchTimer <= 0.0f)
	{
		TransitionTo(EPatrolState::Patrolling);
	}
}

void UGIPatrolComponent::TickRouting(float DeltaTime)
{
	// Routing enemies flee away from the player at sprint speed
	// They don't engage, they just run

	// Check if we've fled far enough
	const float FledDistance = FVector::Dist(OwnerEnemy->GetActorLocation(), RoutStartLocation);
	if (FledDistance >= RoutFleeDistance)
	{
		// Despawn the routing enemy (they escaped)
		OwnerEnemy->Destroy();
		return;
	}

	// Keep running in the flee direction
	MoveInDirection(RoutFleeDirection, RoutSpeed);
}

bool UGIPatrolComponent::CanSeePlayer(APawn* Player) const
{
	if (!Player || !IsValid(Player) || !OwnerEnemy) return false;

	const FVector MyLoc = OwnerEnemy->GetActorLocation();
	const FVector PlayerLoc = Player->GetActorLocation();
	const float Distance = FVector::Dist(MyLoc, PlayerLoc);

	if (Distance > SightRange) return false;

	// Angle check
	const FVector ToPlayer = (PlayerLoc - MyLoc).GetSafeNormal();
	const FVector Forward = OwnerEnemy->GetActorForwardVector();
	const float DotProduct = FVector::DotProduct(Forward, ToPlayer);
	const float AngleCos = FMath::Cos(FMath::DegreesToRadians(SightAngle));

	if (DotProduct < AngleCos) return false;

	// Line of sight trace
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerEnemy);
	Params.AddIgnoredActor(Player);

	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
		Hit,
		MyLoc + FVector(0, 0, 50.0f),
		PlayerLoc + FVector(0, 0, 50.0f),
		ECC_Visibility,
		Params
	);

	return !bBlocked;
}

void UGIPatrolComponent::MoveToward(const FVector& Target, float Speed)
{
	if (!OwnerEnemy) return;

	OwnerEnemy->GetCharacterMovement()->MaxWalkSpeed = Speed;
	FaceToward(Target);

	AAIController* AIC = Cast<AAIController>(OwnerEnemy->GetController());
	if (!AIC) return;

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastMoveRequestTime < MoveRequestCooldown) return;
	LastMoveRequestTime = Now;

	AIC->MoveToLocation(Target, WaypointAcceptRadius, true, true, false, true);
}

void UGIPatrolComponent::MoveInDirection(const FVector& Direction, float Speed)
{
	if (!OwnerEnemy || Direction.IsNearlyZero()) return;

	OwnerEnemy->GetCharacterMovement()->MaxWalkSpeed = Speed;
	FaceDirection(Direction);

	AAIController* AIC = Cast<AAIController>(OwnerEnemy->GetController());
	if (!AIC) return;

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastMoveRequestTime < MoveRequestCooldown) return;
	LastMoveRequestTime = Now;

	const FVector FleeDestination = OwnerEnemy->GetActorLocation() + Direction.GetSafeNormal2D() * 2000.0f;
	AIC->MoveToLocation(FleeDestination, WaypointAcceptRadius, true, true, false, true);
}

void UGIPatrolComponent::StopCurrentMove()
{
	if (!OwnerEnemy) return;
	AAIController* AIC = Cast<AAIController>(OwnerEnemy->GetController());
	if (AIC) AIC->StopMovement();
}

void UGIPatrolComponent::FaceToward(const FVector& Target)
{
	if (!OwnerEnemy) return;
	const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(OwnerEnemy->GetActorLocation(), Target);
	OwnerEnemy->SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));
}

void UGIPatrolComponent::FaceDirection(const FVector& Direction)
{
	if (!OwnerEnemy || Direction.IsNearlyZero()) return;
	const FRotator LookAt = Direction.Rotation();
	OwnerEnemy->SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));
}

void UGIPatrolComponent::TransitionTo(EPatrolState NewState)
{
	State = NewState;

	if (!OwnerEnemy) return;

	switch (NewState)
	{
	case EPatrolState::Patrolling:
		OwnerEnemy->StopShooting();
		break;
	case EPatrolState::Alerted:
	case EPatrolState::Searching:
		OwnerEnemy->StopShooting();
		break;
	case EPatrolState::Engaging:
		break;
	case EPatrolState::Routing:
		// Calculate flee direction and store start location
		OwnerEnemy->StopShooting();
		RoutStartLocation = OwnerEnemy->GetActorLocation();
		RoutFleeDirection = CalculateFleeDirection();
		break;
	default:
		break;
	}
}

FVector UGIPatrolComponent::CalculateFleeDirection() const
{
	if (!OwnerEnemy) return FVector::ForwardVector;

	// Flee away from the player
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (Player && IsValid(Player))
	{
		FVector AwayFromPlayer = (OwnerEnemy->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal2D();
		
		// Add some randomness to prevent predictable flee paths
		const float RandomAngle = FMath::RandRange(-45.0f, 45.0f);
		FVector Rotated = AwayFromPlayer.RotateAngleAxis(RandomAngle, FVector::UpVector);
		return Rotated.GetSafeNormal2D();
	}

	// Fallback: flee in a random direction
	const float RandomAngle = FMath::RandRange(0.0f, 360.0f);
	return FVector(FMath::Cos(FMath::DegreesToRadians(RandomAngle)), FMath::Sin(FMath::DegreesToRadians(RandomAngle)), 0.0f);
}

void UGIPatrolComponent::AlertNearbyAllies(const FVector& AlertLocation)
{
	TArray<AActor*> AllEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGIEnemyCharacter::StaticClass(), AllEnemies);

	for (AActor* Actor : AllEnemies)
	{
		if (Actor == GetOwner()) continue;

		const float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
		if (Dist > HearingRange) continue;

		AGIEnemyCharacter* Enemy = Cast<AGIEnemyCharacter>(Actor);
		if (!Enemy || Enemy->GetIsDead()) continue;

		UGIPatrolComponent* PatrolComp = Enemy->FindComponentByClass<UGIPatrolComponent>();
		if (PatrolComp)
		{
			PatrolComp->OnAllyShotFired(AlertLocation);
		}
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/AI/ShooterAIController.h"
#include "ShooterNPC.h"
#include "Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "AI/Navigation/PathFollowingAgentInterface.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

AShooterAIController::AShooterAIController()
{
	// create the StateTree component
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
	StateTreeAI->SetStartLogicAutomatically(false);

	// create the AI perception component. It will be configured in BP
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	// subscribe to the AI perception delegates
	AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AShooterAIController::OnPerceptionUpdated);
	AIPerception->OnTargetPerceptionForgotten.AddDynamic(this, &AShooterAIController::OnPerceptionForgotten);
}

void AShooterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// ensure we're possessing an NPC
	if (AShooterNPC* NPC = Cast<AShooterNPC>(InPawn))
	{
		// add the team tag to the pawn
		NPC->Tags.Add(TeamTag);

		// subscribe to the pawn's OnDeath delegate
		NPC->OnPawnDeath.AddDynamic(this, &AShooterAIController::OnPawnDeath);

		// start AI logic
		StateTreeAI->StartLogic();
	}
}

void AShooterAIController::OnPawnDeath()
{
	// stop movement
	GetPathFollowingComponent()->AbortMove(*this, FPathFollowingResultFlags::UserAbort);

	// stop StateTree logic
	StateTreeAI->StopLogic(FString(""));

	// unpossess the pawn
	UnPossess();

	// destroy this controller
	Destroy();
}

void AShooterAIController::SetCurrentTarget(AActor* Target)
{
	TargetEnemy = Target;

	if (!Target || !IsValid(Target))
	{
		StopChasing();
	}
}

void AShooterAIController::ClearCurrentTarget()
{
	TargetEnemy = nullptr;
	StopChasing();
}

void AShooterAIController::StartChasing(AActor* Target, float AcceptanceRadius)
{
	if (!Target || !IsValid(Target)) return;

	TargetEnemy = Target;
	ChaseAcceptanceRadius = AcceptanceRadius;

	// Short delay before first move — allows nav invoker to generate tiles around the enemy
	FTimerHandle FirstMoveDelay;
	GetWorld()->GetTimerManager().SetTimer(
		FirstMoveDelay,
		[this]()
		{
			if (TargetEnemy && IsValid(TargetEnemy) && GetPawn())
			{
				MoveToActor(TargetEnemy, ChaseAcceptanceRadius, true, true, false);
			}
		},
		0.3f, false
	);

	// Then refresh path every 0.5s to track the moving player
	GetWorld()->GetTimerManager().SetTimer(
		ChaseTimer,
		[this]()
		{
			if (TargetEnemy && IsValid(TargetEnemy) && GetPawn())
			{
				MoveToActor(TargetEnemy, ChaseAcceptanceRadius, true, true, false);
			}
			else
			{
				StopChasing();
			}
		},
		0.5f, true
	);
}

void AShooterAIController::StopChasing()
{
	GetWorld()->GetTimerManager().ClearTimer(ChaseTimer);
	StopMovement();
}

void AShooterAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// pass the data to the StateTree delegate hook
	OnShooterPerceptionUpdated.ExecuteIfBound(Actor, Stimulus);
}

void AShooterAIController::OnPerceptionForgotten(AActor* Actor)
{
	// pass the data to the StateTree delegate hook
	OnShooterPerceptionForgotten.ExecuteIfBound(Actor);
}

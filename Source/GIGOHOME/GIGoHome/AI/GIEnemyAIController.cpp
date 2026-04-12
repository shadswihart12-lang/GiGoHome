// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/AI/GIEnemyAIController.h"
#include "GIGoHome/AI/GIEnemyCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AGIEnemyAIController::AGIEnemyAIController()
{
}

void AGIEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	PossessedEnemy = Cast<AGIEnemyCharacter>(InPawn);

	// bind an additional perception listener to handle fear propagation
	// (parent already binds OnPerceptionUpdated which fires StateTree delegates)
	if (UAIPerceptionComponent* Perception = GetPerceptionComponent())
	{
		Perception->OnTargetPerceptionUpdated.AddDynamic(this, &AGIEnemyAIController::OnEnemyPerceptionUpdated);
	}
}

void AGIEnemyAIController::NotifyBodyDiscovered()
{
	if (!PossessedEnemy)
	{
		return;
	}

	PossessedEnemy->AddFear(FearPerBodyFound);

	// finding a body is alarming - spread half the fear to nearby allies
	PropagateAlertToNearbyAllies(FearPerBodyFound * 0.5f, 1500.0f);
}

void AGIEnemyAIController::AttemptRadioCall()
{
	if (!PossessedEnemy || !PossessedEnemy->HasRadio())
	{
		return;
	}

	// find radio actors in the level
	TArray<AActor*> Radios;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), RadioActorTag, Radios);

	for (AActor* Radio : Radios)
	{
		const float Distance = FVector::Dist(PossessedEnemy->GetActorLocation(), Radio->GetActorLocation());
		if (Distance <= 300.0f)
		{
			PossessedEnemy->SetCalledReinforcements();
			return;
		}
	}
}

bool AGIEnemyAIController::IsPossessedEnemyRouting() const
{
	return PossessedEnemy && PossessedEnemy->IsRouting();
}

void AGIEnemyAIController::OnEnemyPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// convert gunshot hearing into fear on the possessed character
	if (Stimulus.Tag == FName("Shot") && PossessedEnemy)
	{
		PossessedEnemy->AddFear(FearPerGunshot);
	}
}

void AGIEnemyAIController::PropagateAlertToNearbyAllies(float FearAmount, float Radius)
{
	if (!PossessedEnemy)
	{
		return;
	}

	TArray<AActor*> NearbyActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGIEnemyCharacter::StaticClass(), NearbyActors);

	for (AActor* Actor : NearbyActors)
	{
		AGIEnemyCharacter* NearbyEnemy = Cast<AGIEnemyCharacter>(Actor);
		if (NearbyEnemy && NearbyEnemy != PossessedEnemy && !NearbyEnemy->ActorHasTag(FName("Dead")))
		{
			const float Distance = FVector::Dist(PossessedEnemy->GetActorLocation(), NearbyEnemy->GetActorLocation());
			if (Distance <= Radius)
			{
				NearbyEnemy->AddFear(FearAmount);
			}
		}
	}
}

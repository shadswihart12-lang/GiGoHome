// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Campaign NPC Implementation

#include "GIGoHome/Campaign/Actors/GICampaignNPC.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

AGICampaignNPC::AGICampaignNPC()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	InteractionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(GetRootComponent());
	InteractionVolume->SetSphereRadius(InteractionRadius);
	InteractionVolume->SetCollisionProfileName(TEXT("OverlapAll"));
	InteractionVolume->SetGenerateOverlapEvents(true);

	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AGICampaignNPC::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	InteractionVolume->OnComponentBeginOverlap.AddDynamic(
		this, &AGICampaignNPC::OnInteractionOverlapBegin);
	InteractionVolume->OnComponentEndOverlap.AddDynamic(
		this, &AGICampaignNPC::OnInteractionOverlapEnd);

	// Apply initial state
	SetNPCState(InitialState);

	// Schedule wake-up if configured
	if (WakeUpTime > 0.0f && InitialState == ECampaignNPCState::Sleeping)
	{
		ScheduleWakeUp();
	}
}

void AGICampaignNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (CurrentState)
	{
	case ECampaignNPCState::Patrolling:
		TickPatrol(DeltaTime);
		break;
	case ECampaignNPCState::Fleeing:
		TickFlee(DeltaTime);
		break;
	default:
		break;
	}
}

// ============================================================================
// DAMAGE
// ============================================================================

float AGICampaignNPC::TakeDamage(float Damage, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.0f;

	float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	CurrentHealth -= ActualDamage;

	if (CurrentHealth <= 0.0f)
	{
		Kill();
	}

	return ActualDamage;
}

void AGICampaignNPC::Kill()
{
	if (bIsDead) return;

	bIsDead = true;
	SetNPCState(ECampaignNPCState::Dead);
	OnNPCKilled.Broadcast(this);

	// Disable collision and movement
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();

	// Ragdoll
	if (GetMesh())
	{
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	}
}

// ============================================================================
// STATE
// ============================================================================

void AGICampaignNPC::SetNPCState(ECampaignNPCState NewState)
{
	if (CurrentState == NewState && CurrentState != ECampaignNPCState::Idle) return;

	ECampaignNPCState OldState = CurrentState;
	OnExitState(OldState);

	CurrentState = NewState;
	OnEnterState(NewState);
	OnNPCStateChanged.Broadcast(NewState);
}

void AGICampaignNPC::OnEnterState(ECampaignNPCState State)
{
	switch (State)
	{
	case ECampaignNPCState::Sleeping:
		GetCharacterMovement()->DisableMovement();
		SetActorTickEnabled(false);
		// Crouch to simulate lying down
		Crouch();
		break;

	case ECampaignNPCState::WritingLetter:
		GetCharacterMovement()->DisableMovement();
		SetActorTickEnabled(false);
		break;

	case ECampaignNPCState::Patrolling:
		GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
		SetActorTickEnabled(true);
		break;

	case ECampaignNPCState::Fleeing:
		GetCharacterMovement()->MaxWalkSpeed = FleeSpeed;
		SetActorTickEnabled(true);
		break;

	case ECampaignNPCState::Escorted:
		GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed * 0.6f;
		SetActorTickEnabled(false);
		break;

	case ECampaignNPCState::Alert:
		GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed * 1.5f;
		SetActorTickEnabled(false);
		break;

	case ECampaignNPCState::Dead:
		GetCharacterMovement()->DisableMovement();
		SetActorTickEnabled(false);
		break;

	default:
		SetActorTickEnabled(false);
		break;
	}
}

void AGICampaignNPC::OnExitState(ECampaignNPCState State)
{
	switch (State)
	{
	case ECampaignNPCState::Sleeping:
		UnCrouch();
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		break;
	default:
		break;
	}
}

// ============================================================================
// ACTIONS
// ============================================================================

void AGICampaignNPC::WakeUp()
{
	if (CurrentState == ECampaignNPCState::Sleeping)
	{
		SetNPCState(ECampaignNPCState::Alert);
	}
}

void AGICampaignNPC::StartFleeing(const FVector& Target)
{
	FleeTarget = Target;
	SetNPCState(ECampaignNPCState::Fleeing);
}

void AGICampaignNPC::InteractWith(AActor* Interactor)
{
	if (bIsDead) return;

	OnNPCInteracted.Broadcast(this, Interactor);
}

bool AGICampaignNPC::HasItem(FName ItemTag) const
{
	return CarriedItems.Contains(ItemTag);
}

// ============================================================================
// PATROL / FLEE TICK
// ============================================================================

void AGICampaignNPC::TickPatrol(float DeltaTime)
{
	if (PatrolPoints.Num() == 0) return;

	const FVector Target = PatrolPoints[CurrentPatrolIndex];
	const FVector CurrentLocation = GetActorLocation();
	const FVector Direction = (Target - CurrentLocation).GetSafeNormal();

	AddMovementInput(Direction, 1.0f);

	// Check if reached patrol point
	if (FVector::DistSquared(CurrentLocation, Target) < FMath::Square(100.0f))
	{
		CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
	}
}

void AGICampaignNPC::TickFlee(float DeltaTime)
{
	const FVector CurrentLocation = GetActorLocation();
	const FVector Direction = (FleeTarget - CurrentLocation).GetSafeNormal();

	AddMovementInput(Direction, 1.0f);

	// Check if reached flee target
	if (FVector::DistSquared(CurrentLocation, FleeTarget) < FMath::Square(150.0f))
	{
		SetNPCState(ECampaignNPCState::Idle);
	}
}

// ============================================================================
// INTERACTION OVERLAP
// ============================================================================

void AGICampaignNPC::OnInteractionOverlapBegin(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->ActorHasTag(FName("Player")))
	{
		bPlayerInRange = true;
	}
}

void AGICampaignNPC::OnInteractionOverlapEnd(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->ActorHasTag(FName("Player")))
	{
		bPlayerInRange = false;
	}
}

// ============================================================================
// WAKE-UP SCHEDULING
// ============================================================================

void AGICampaignNPC::ScheduleWakeUp()
{
	GetWorldTimerManager().SetTimer(
		WakeUpTimer, this, &AGICampaignNPC::OnWakeUpTimerFired, WakeUpTime, false);
}

void AGICampaignNPC::OnWakeUpTimerFired()
{
	WakeUp();
}

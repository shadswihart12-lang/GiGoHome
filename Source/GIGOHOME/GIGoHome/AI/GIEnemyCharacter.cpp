// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/AI/GIEnemyCharacter.h"
#include "GIGoHome/AI/GIEnemyAnimInstance.h"
#include "GIGoHome/AI/GIPatrolComponent.h"
#include "GIGoHome/AI/GIForcedVisibilityComponent.h"
#include "GIGoHome/Player/XiDongCharacter.h"
#include "GIGoHome/GIGoHomeSliceGameMode.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GIGoHome/Weapons/WeaponPickup.h"

AGIEnemyCharacter::AGIEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create the forced visibility component - this ensures mesh is always visible
	ForcedVisibilityComponent = CreateDefaultSubobject<UGIForcedVisibilityComponent>(TEXT("ForcedVisibility"));

	// The base class sets OwnerNoSee=true and FirstPersonPrimitiveType=WorldSpaceRepresentation
	// on GetMesh() for the player's self-hiding logic. Enemies must undo all of this.
	ForceMeshVisible();

	// Assign our custom anim instance so walk/idle/run drive correctly
	GetMesh()->SetAnimInstanceClass(UGIEnemyAnimInstance::StaticClass());

	// Hide the first person arms mesh — enemies don't need it
	GetFirstPersonMesh()->SetHiddenInGame(true);
	GetFirstPersonMesh()->SetVisibility(false);
	GetFirstPersonMesh()->SetOnlyOwnerSee(false);

	// Ensure enemies walk on the ground and don't fly
	GetCharacterMovement()->GravityScale = 1.0f;
	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->bConstrainToPlane = false;
}

void AGIEnemyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Force visibility again after all components are initialized
	// This catches any Blueprint or parent class overrides
	ForceMeshVisible();
}

void AGIEnemyCharacter::ForceMeshVisible()
{
	USkeletalMeshComponent* CharMesh = GetMesh();
	if (!CharMesh) return;

	CharMesh->SetOwnerNoSee(false);
	CharMesh->SetOnlyOwnerSee(false);
	CharMesh->bOwnerNoSee = false;
	CharMesh->bOnlyOwnerSee = false;
	CharMesh->SetHiddenInGame(false);
	CharMesh->SetVisibility(true);
	CharMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::None;
}

void AGIEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Movement and engagement handled by GIPatrolComponent when present
	// Fall back to direct hunt only if no patrol component is attached
	if (FindComponentByClass<UGIPatrolComponent>()) return;

	if (bIsDead) return;

	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player || !IsValid(Player)) return;

	// Stop shooting if player is dead or being destroyed
	AXiDongCharacter* PlayerChar = Cast<AXiDongCharacter>(Player);
	if (PlayerChar && PlayerChar->IsDead())
	{
		StopShooting();
		return;
	}

	const FVector MyLoc = GetActorLocation();
	const FVector PlayerLoc = Player->GetActorLocation();
	const float Distance = FVector::Dist(MyLoc, PlayerLoc);

	// Always face the player
	const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(MyLoc, PlayerLoc);
	SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));

	// Move toward player if too far — use AddMovementInput for proper gravity/physics
	if (Distance > StopDistance)
	{
		const FVector Dir = (PlayerLoc - MyLoc).GetSafeNormal2D();
		AddMovementInput(Dir, 1.0f);
	}
	else
	{
		// In range — stop moving
		GetCharacterMovement()->StopActiveMovement();
	}

	// Shoot at player when in range
	if (Distance <= ShootRange && Weapon)
	{
		ShootTimer += DeltaTime;
		if (ShootTimer >= ShootInterval)
		{
			ShootTimer = 0.0f;
			StartShooting(Player);
			GetWorld()->GetTimerManager().SetTimer(
				StopShootingTimer,
				[this]() { StopShooting(); },
				0.3f, false
			);
		}
	}
}

void AGIEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	USkeletalMeshComponent* EnemyMesh = GetMesh();
	if (EnemyMesh && !EnemyMesh->GetSkeletalMeshAsset())
	{
		if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
		{
			if (ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerPawn))
			{
				if (USkeletalMesh* PlayerMeshAsset = PlayerCharacter->GetMesh() ? PlayerCharacter->GetMesh()->GetSkeletalMeshAsset() : nullptr)
				{
					EnemyMesh->SetSkeletalMesh(PlayerMeshAsset);
				}
			}
		}
	}

    if (EnemyMesh && !EnemyMesh->GetAnimInstance())
	{
		EnemyMesh->SetAnimInstanceClass(UGIEnemyAnimInstance::StaticClass());
	}

	// Re-enforce visibility — Blueprint or parent BeginPlay may override constructor values
	ForceMeshVisible();

	// Officers start at maximum morale
	if (Archetype == EEnemyArchetype::Officer)
	{
		MoraleLevel = 1.0f;
		RoutMoraleThreshold = 0.05f;
	}

	// Attach and activate the weapon
	if (Weapon)
	{
		AttachWeaponMeshes(Weapon);
		Weapon->ActivateWeapon();
	}
}

float AGIEnemyCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float Result = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (bIsDead)
	{
		// Add "Dead" tag for body dragging system
		Tags.AddUnique(FName("Dead"));

		// Drop weapon pickup for scavenging (if WeaponPickup class exists)
		if (Weapon && WeaponPickupClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			
			FVector DropLocation = GetActorLocation() + FVector(0.f, 0.f, 30.f);
			AWeaponPickup* DroppedWeapon = GetWorld()->SpawnActor<AWeaponPickup>(WeaponPickupClass, DropLocation, FRotator::ZeroRotator, SpawnParams);
			if (DroppedWeapon)
			{
				DroppedWeapon->SetWeaponClass(Weapon->GetClass(), Weapon->GetBulletCount());
			}
		}
		
		// Per GDD: Eliminating officers causes squads to lose cohesion and retreat
		if (Archetype == EEnemyArchetype::Officer)
		{
			if (AGIGoHomeSliceGameMode* GM = Cast<AGIGoHomeSliceGameMode>(GetWorld()->GetAuthGameMode()))
			{
				GM->NotifyOfficerEliminated();
			}
		}

		// Notify nearby enemies to reduce their morale
		TArray<AActor*> NearbyActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGIEnemyCharacter::StaticClass(), NearbyActors);

		for (AActor* Actor : NearbyActors)
		{
			AGIEnemyCharacter* NearbyEnemy = Cast<AGIEnemyCharacter>(Actor);
			if (NearbyEnemy && NearbyEnemy != this && !NearbyEnemy->bIsDead)
			{
				const float Distance = FVector::Dist(GetActorLocation(), NearbyEnemy->GetActorLocation());
				if (Distance <= MoraleAwarenessRadius)
				{
					NearbyEnemy->NotifyNearbyAllyDied();
				}
			}
		}

		// Auto-destroy after 30 seconds if not dragged
		SetLifeSpan(30.0f);
	}

	return Result;
}

void AGIEnemyCharacter::AddFear(float Amount)
{
	const float OldFear = FearLevel;
	FearLevel = FMath::Clamp(FearLevel + Amount, 0.0f, 1.0f);

	if (!FMath::IsNearlyEqual(FearLevel, OldFear, 0.05f))
	{
		OnFearChanged(FearLevel);
	}
}

void AGIEnemyCharacter::SetCalledReinforcements()
{
	bHasCalledReinforcements = true;
}

void AGIEnemyCharacter::NotifyNearbyAllyDied()
{
	MoraleLevel = FMath::Clamp(MoraleLevel - MoralePerDeathLoss, 0.0f, 1.0f);
	OnMoraleChanged.Broadcast(MoraleLevel);
	EvaluateMorale();
}

void AGIEnemyCharacter::EvaluateMorale()
{
	if (!bIsRouting && MoraleLevel <= RoutMoraleThreshold)
	{
		bIsRouting = true;
		OnStartRouting();
	}
}

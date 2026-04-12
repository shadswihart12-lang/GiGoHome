// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/AI/GITestEnemy.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"

AGITestEnemy::AGITestEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// Ensure mesh is visible
	GetMesh()->SetOwnerNoSee(false);
	GetMesh()->SetOnlyOwnerSee(false);
	GetMesh()->SetHiddenInGame(false);
	GetMesh()->SetVisibility(true);

	// Setup movement
	GetCharacterMovement()->GravityScale = 1.0f;
	GetCharacterMovement()->MaxWalkSpeed = 250.0f;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	
	// Make sure capsule has collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);

	CurrentHP = MaxHP;
}

void AGITestEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHP = MaxHP;
	SpawnWeapon();

	UE_LOG(LogTemp, Warning, TEXT("GITestEnemy spawned at %s"), *GetActorLocation().ToString());
}

void AGITestEnemy::SpawnWeapon()
{
	if (!WeaponClass) 
	{
		UE_LOG(LogTemp, Warning, TEXT("GITestEnemy: No WeaponClass assigned!"));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Weapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);
	
	if (Weapon)
	{
		// Attach to hand
		const FAttachmentTransformRules AttachRule(EAttachmentRule::SnapToTarget, false);
		Weapon->AttachToComponent(GetMesh(), AttachRule, FName("hand_r"));
		Weapon->ActivateWeapon();
		UE_LOG(LogTemp, Warning, TEXT("GITestEnemy: Weapon spawned and attached"));
	}
}

void AGITestEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead) return;

	// Find player
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player || !IsValid(Player)) return;

	const FVector MyLoc = GetActorLocation();
	const FVector PlayerLoc = Player->GetActorLocation();
	const float Distance = FVector::Dist(MyLoc, PlayerLoc);

	// Face player
	const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(MyLoc, PlayerLoc);
	SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));

	// Move toward player
	if (Distance > StopDistance)
	{
		const FVector Dir = (PlayerLoc - MyLoc).GetSafeNormal2D();
		AddMovementInput(Dir, 1.0f);
	}

	// Shoot at player when in range
	if (Distance <= ShootRange && Weapon)
	{
		ShootTimer += DeltaTime;
		if (ShootTimer >= ShootInterval)
		{
			ShootTimer = 0.0f;
			
			// Fire at player
			Weapon->StartFiring();
			
			// Stop firing after short burst
			GetWorld()->GetTimerManager().SetTimer(
				StopShootTimer,
				[this]() { if (Weapon) Weapon->StopFiring(); },
				0.3f, false
			);
		}
	}
}

float AGITestEnemy::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.0f;

	CurrentHP -= Damage;
	UE_LOG(LogTemp, Warning, TEXT("GITestEnemy took %f damage, HP now %f"), Damage, CurrentHP);

	if (CurrentHP <= 0.0f)
	{
		Die();
	}

	return Damage;
}

void AGITestEnemy::Die()
{
	bIsDead = true;
	
	UE_LOG(LogTemp, Warning, TEXT("GITestEnemy died!"));

	// Ragdoll
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	GetMesh()->SetSimulatePhysics(true);

	// Destroy after delay
	SetLifeSpan(5.0f);
}

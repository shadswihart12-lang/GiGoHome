// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/AI/ShooterNPC.h"
#include "ShooterWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "ShooterGameMode.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/SkeletalMesh.h"

AShooterNPC::AShooterNPC()
{
	// NPCs are AI-controlled and seen in third person by the player
	// Base class sets OwnerNoSee=true for player FP logic — undo that for NPCs
	GetMesh()->SetOwnerNoSee(false);
	GetMesh()->SetOnlyOwnerSee(false);
	GetMesh()->bOwnerNoSee = false;
	GetMesh()->bOnlyOwnerSee = false;
	GetMesh()->SetHiddenInGame(false);
	GetMesh()->SetVisibility(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::None;

	// Hide first person arms — NPCs don't need them
	GetFirstPersonMesh()->SetHiddenInGame(true);
	GetFirstPersonMesh()->SetVisibility(false);

	// Ensure NPCs walk on the ground and respond to AI move requests
	GetCharacterMovement()->GravityScale = 1.0f;
	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	bUseControllerRotationYaw = false;
}

void AShooterNPC::BeginPlay()
{
	Super::BeginPlay();

	// spawn the weapon
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Weapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);

	// activate the weapon so it is visible and ready to fire
	if (Weapon)
	{
		Weapon->ActivateWeapon();
	}
}

void AShooterNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the death timer
	GetWorld()->GetTimerManager().ClearTimer(DeathTimer);
}

float AShooterNPC::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// ignore if already dead
	if (bIsDead)
	{
		return 0.0f;
	}

	// Reduce HP
	CurrentHP -= Damage;

	// Have we depleted HP?
	if (CurrentHP <= 0.0f)
	{
		Die();
	}

	return Damage;
}

void AShooterNPC::AttachWeaponMeshes(AShooterWeapon* WeaponToAttach)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

	// Attach the weapon actor to the character
	WeaponToAttach->AttachToActor(this, AttachmentRule);

	// Attach FP mesh to FP arms socket
	WeaponToAttach->GetFirstPersonMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);

	// Attach TP mesh to body hand socket — try ThirdPersonWeaponSocket first, fall back to hand_r
	USkeletalMeshComponent* BodyMesh = GetMesh();
	FName AttachSocket = ThirdPersonWeaponSocket;
	if (!BodyMesh->DoesSocketExist(AttachSocket))
	{
		AttachSocket = FName("hand_r");
	}
	if (!BodyMesh->DoesSocketExist(AttachSocket))
	{
		AttachSocket = FName("RightHandSocket");
	}
	WeaponToAttach->GetThirdPersonMesh()->AttachToComponent(BodyMesh, AttachmentRule, AttachSocket);

	// If ThirdPersonMesh has no skeletal mesh assigned, load SK_AR4 directly
	if (!WeaponToAttach->GetThirdPersonMesh()->GetSkeletalMeshAsset())
	{
		USkeletalMesh* FallbackMesh = LoadObject<USkeletalMesh>(
			nullptr,
			TEXT("/Game/FPS_Weapon_Bundle/Weapons/Meshes/Ka47/SK_KA47.SK_KA47")
		);
		if (FallbackMesh)
		{
			WeaponToAttach->GetThirdPersonMesh()->SetSkeletalMesh(FallbackMesh);
		}
	}

	// Force weapon mesh visible after attachment
	WeaponToAttach->GetThirdPersonMesh()->SetHiddenInGame(false);
	WeaponToAttach->GetThirdPersonMesh()->SetVisibility(true);
	WeaponToAttach->GetThirdPersonMesh()->bOwnerNoSee = false;
	WeaponToAttach->SetActorHiddenInGame(false);
}

void AShooterNPC::PlayFiringMontage(UAnimMontage* Montage)
{
	// unused
}

void AShooterNPC::AddWeaponRecoil(float Recoil)
{
	// unused
}

void AShooterNPC::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	// unused
}

FVector AShooterNPC::GetWeaponTargetLocation()
{
	// Guard against invalid aim target
	if (!CurrentAimTarget || !IsValid(CurrentAimTarget))
	{
		return GetActorLocation() + GetActorForwardVector() * AimRange;
	}

	// start aiming from the camera location
	const FVector AimSource = GetFirstPersonCameraComponent()->GetComponentLocation();

	FVector AimDir, AimTarget = FVector::ZeroVector;

	// do we have an aim target?
	if (CurrentAimTarget)
	{
		// target the actor location
		AimTarget = CurrentAimTarget->GetActorLocation();

		// apply a vertical offset to target head/feet
		AimTarget.Z += FMath::RandRange(MinAimOffsetZ, MaxAimOffsetZ);

		// get the aim direction and apply randomness in a cone
		AimDir = (AimTarget - AimSource).GetSafeNormal();
		AimDir = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(AimDir, AimVarianceHalfAngle);

		
	} else {

		// no aim target, so just use the camera facing
		AimDir = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(GetFirstPersonCameraComponent()->GetForwardVector(), AimVarianceHalfAngle);

	}

	// calculate the unobstructed aim target location
	AimTarget = AimSource + (AimDir * AimRange);

	// run a visibility trace to see if there's obstructions
	FHitResult OutHit;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(OutHit, AimSource, AimTarget, ECC_Visibility, QueryParams);

	// return either the impact point or the trace end
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}

void AShooterNPC::AddWeaponClass(const TSubclassOf<AShooterWeapon>& InWeaponClass)
{
	// unused
}

void AShooterNPC::OnWeaponActivated(AShooterWeapon* InWeapon)
{
	// unused
}

void AShooterNPC::OnWeaponDeactivated(AShooterWeapon* InWeapon)
{
	// unused
}

void AShooterNPC::OnSemiWeaponRefire()
{
	// are we still shooting?
	if (bIsShooting)
	{
		// fire the weapon
		Weapon->StartFiring();
	}
}

void AShooterNPC::Die()
{
	// ignore if already dead
	if (bIsDead)
	{
		return;
	}

	// raise the dead flag
	bIsDead = true;

	// grant the death tag to the character
	Tags.Add(DeathTag);

	// call the delegate
	OnPawnDeath.Broadcast();

	// increment the team score
	if (AShooterGameMode* GM = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->IncrementTeamScore(TeamByte);
	}

	// disable capsule collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// stop movement
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->StopActiveMovement();

	// enable ragdoll physics on the third person mesh
	GetMesh()->SetCollisionProfileName(RagdollCollisionProfile);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetPhysicsBlendWeight(1.0f);

	// schedule actor destruction
	GetWorld()->GetTimerManager().SetTimer(DeathTimer, this, &AShooterNPC::DeferredDestruction, DeferredDestructionTime, false);
}

void AShooterNPC::DeferredDestruction()
{
	Destroy();
}

void AShooterNPC::StartShooting(AActor* ActorToShoot)
{
	// save the aim target
	CurrentAimTarget = ActorToShoot;

	// raise the flag
	bIsShooting = true;

	// signal the weapon
	Weapon->StartFiring();
}

void AShooterNPC::StopShooting()
{
	// lower the flag
	bIsShooting = false;

	// signal the weapon
	Weapon->StopFiring();
}

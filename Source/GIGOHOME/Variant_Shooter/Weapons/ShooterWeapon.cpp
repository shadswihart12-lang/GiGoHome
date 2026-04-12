// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterWeapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "ShooterProjectile.h"
#include "ShooterWeaponHolder.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AShooterWeapon::AShooterWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(RootComponent);
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	// Use standard visibility - owner only sees this mesh
	FirstPersonMesh->bOnlyOwnerSee = true;
	FirstPersonMesh->bOwnerNoSee = false;
	FirstPersonMesh->SetHiddenInGame(false);

	ThirdPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Third Person Mesh"));
	ThirdPersonMesh->SetupAttachment(RootComponent);
	ThirdPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	// Visible to everyone including the owning enemy
	ThirdPersonMesh->bOwnerNoSee = false;
	ThirdPersonMesh->bOnlyOwnerSee = false;
	ThirdPersonMesh->SetHiddenInGame(false);
}

void AShooterWeapon::BeginPlay()
{
	Super::BeginPlay();

	GetOwner()->OnDestroyed.AddDynamic(this, &AShooterWeapon::OnOwnerDestroyed);

	WeaponOwner = Cast<IShooterWeaponHolder>(GetOwner());
	PawnOwner = Cast<APawn>(GetOwner());

	// start with a full magazine
	CurrentBullets = MagazineSize;

	WeaponOwner->AttachWeaponMeshes(this);
}

void AShooterWeapon::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(RefireTimer);
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
}

void AShooterWeapon::OnOwnerDestroyed(AActor* DestroyedActor)
{
	Destroy();
}

void AShooterWeapon::ActivateWeapon()
{
	SetActorHiddenInGame(false);
	WeaponOwner->OnWeaponActivated(this);
}

void AShooterWeapon::DeactivateWeapon()
{
	StopFiring();
	SetActorHiddenInGame(true);
	WeaponOwner->OnWeaponDeactivated(this);
}

void AShooterWeapon::StartFiring()
{
	if (bIsReloading) return;

	if (CurrentBullets <= 0)
	{
		// Play dry fire click
		if (DryFireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, DryFireSound, GetActorLocation(), FireSoundVolume);
		}
		Reload();
		return;
	}

	bIsFiring = true;

	const float TimeSinceLastShot = GetWorld()->GetTimeSeconds() - TimeOfLastShot;

	if (TimeSinceLastShot > RefireRate)
	{
		Fire();
	}
	else if (bFullAuto)
	{
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::Fire, TimeSinceLastShot, false);
	}
}

void AShooterWeapon::StopFiring()
{
	bIsFiring = false;
	GetWorld()->GetTimerManager().ClearTimer(RefireTimer);
}

void AShooterWeapon::Reload()
{
	if (bIsReloading || CurrentBullets >= MagazineSize || ReserveAmmo <= 0) return;

	bIsReloading = true;
	StopFiring();

	// Play reload sound
	if (ReloadSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation(), FireSoundVolume);
	}

	// Play reload montage on owner
	PlayReloadMontage();

	WeaponOwner->UpdateWeaponHUD(0, MagazineSize);

	// Use montage length if available, otherwise use ReloadTime
	float ActualReloadTime = ReloadTime;
	if (ReloadMontage)
	{
		ActualReloadTime = ReloadMontage->GetPlayLength();
	}

	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimer,
		this,
		&AShooterWeapon::FinishReload,
		ActualReloadTime,
		false
	);
}

void AShooterWeapon::PlayReloadMontage()
{
	ACharacter* CharOwner = Cast<ACharacter>(GetOwner());
	if (!CharOwner) return;

	// Play on first person mesh (arms)
	if (ReloadMontage)
	{
		// Get the first person mesh from the weapon holder interface
		// This is typically the arms mesh attached to the camera
		USkeletalMeshComponent* FPMesh = nullptr;
		
		// Try to get from character if it has GetFirstPersonMesh
		// For now we'll use the character's mesh animation system
		if (UAnimInstance* AnimInstance = CharOwner->GetMesh()->GetAnimInstance())
		{
			// Play on third person mesh first - this is what the world sees
			if (ReloadMontage3P)
			{
				AnimInstance->Montage_Play(ReloadMontage3P);
			}
			else if (ReloadMontage)
			{
				AnimInstance->Montage_Play(ReloadMontage);
			}
		}
	}

	// Notify the weapon holder that we're reloading (for any additional visual effects)
	if (WeaponOwner)
	{
		WeaponOwner->PlayFiringMontage(ReloadMontage); // Reuse the montage callback
	}
}

void AShooterWeapon::FinishReload()
{
	bIsReloading = false;

	const int32 BulletsNeeded = MagazineSize - CurrentBullets;
	const int32 BulletsToAdd = FMath::Min(BulletsNeeded, ReserveAmmo);

	CurrentBullets += BulletsToAdd;
	ReserveAmmo -= BulletsToAdd;

	WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
}

void AShooterWeapon::AddReserveAmmo(int32 Amount)
{
	ReserveAmmo += Amount;
}

void AShooterWeapon::Fire()
{
	if (!bIsFiring) return;

	if (CurrentBullets <= 0)
	{
		Reload();
		return;
	}

	FireProjectile(WeaponOwner->GetWeaponTargetLocation());

	TimeOfLastShot = GetWorld()->GetTimeSeconds();

	MakeNoise(ShotLoudness, PawnOwner, PawnOwner->GetActorLocation(), ShotNoiseRange, ShotNoiseTag);

	if (bFullAuto)
	{
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::Fire, RefireRate, false);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::FireCooldownExpired, RefireRate, false);
	}
}

void AShooterWeapon::FireCooldownExpired()
{
	WeaponOwner->OnSemiWeaponRefire();
}

void AShooterWeapon::FireProjectile(const FVector& TargetLocation)
{
	FTransform ProjectileTransform = CalculateProjectileSpawnTransform(TargetLocation);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::OverrideRootScale;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = PawnOwner;

	GetWorld()->SpawnActor<AShooterProjectile>(ProjectileClass, ProjectileTransform, SpawnParams);

	// Play gunfire sound
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, ProjectileTransform.GetLocation(), FireSoundVolume);
	}

	WeaponOwner->PlayFiringMontage(FiringMontage);
	WeaponOwner->AddWeaponRecoil(FiringRecoil);

	--CurrentBullets;

	WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
}

FTransform AShooterWeapon::CalculateProjectileSpawnTransform(const FVector& TargetLocation) const
{
	const FVector MuzzleLoc = FirstPersonMesh->GetSocketLocation(MuzzleSocketName);
	const FVector SpawnLoc = MuzzleLoc + ((TargetLocation - MuzzleLoc).GetSafeNormal() * MuzzleOffset);
	const FRotator AimRot = UKismetMathLibrary::FindLookAtRotation(SpawnLoc, TargetLocation + (UKismetMathLibrary::RandomUnitVector() * AimVariance));
	return FTransform(AimRot, SpawnLoc, FVector::OneVector);
}

const TSubclassOf<UAnimInstance>& AShooterWeapon::GetFirstPersonAnimInstanceClass() const
{
	return FirstPersonAnimInstanceClass;
}

const TSubclassOf<UAnimInstance>& AShooterWeapon::GetThirdPersonAnimInstanceClass() const
{
	return ThirdPersonAnimInstanceClass;
}

// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Concrete C++ Weapon Implementations

#include "GIGoHome/Weapons/GIWeaponClasses.h"
#include "Variant_Shooter/Weapons/ShooterProjectile.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/ConstructorHelpers.h"

// ============================================================================
// BULLET PROJECTILE
// ============================================================================

AGIBulletProjectile::AGIBulletProjectile()
{
	HitDamage = 25.0f;
	NoiseLoudness = 2.0f;
	NoiseRange = 2000.0f;
	bExplodeOnHit = false;
	DeferredDestructionTime = 3.0f;

	// Configure projectile movement for a fast bullet
	if (UProjectileMovementComponent* ProjMove = FindComponentByClass<UProjectileMovementComponent>())
	{
		ProjMove->InitialSpeed = 15000.0f;
		ProjMove->MaxSpeed = 15000.0f;
		ProjMove->bRotationFollowsVelocity = true;
		ProjMove->ProjectileGravityScale = 0.05f;
	}
}

// ============================================================================
// RIFLE - AK-47 Configuration
// ============================================================================

AGIRifleWeapon::AGIRifleWeapon()
{
	// Magazine
	MagazineSize = 30;
	ReserveAmmo = 120;
	ReloadTime = 2.5f;

	// Fire behavior - full auto
	bFullAuto = true;
	RefireRate = 0.1f;
	AimVariance = 2.0f;
	FiringRecoil = 1.5f;

	// Muzzle
	MuzzleSocketName = FName("Muzzle");
	MuzzleOffset = 10.0f;

	// AI perception
	ShotLoudness = 1.0f;
	ShotNoiseRange = 5000.0f;

	// Audio
	FireSoundVolume = 1.0f;

	// Projectile class
	ProjectileClass = AGIBulletProjectile::StaticClass();

	// Load weapon mesh (KA47)
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> RifleMeshFinder(
		TEXT("/Game/FPS_Weapon_Bundle/Weapons/Meshes/Ka47/SK_KA47.SK_KA47")
	);
	if (RifleMeshFinder.Succeeded())
	{
		GetFirstPersonMesh()->SetSkeletalMesh(RifleMeshFinder.Object);
		GetThirdPersonMesh()->SetSkeletalMesh(RifleMeshFinder.Object);
	}

	// Load fire sound
	static ConstructorHelpers::FObjectFinder<USoundBase> FireSoundFinder(
		TEXT("/Game/Weapons/GrenadeLauncher/Audio/FirstPersonTemplateWeaponFire02.FirstPersonTemplateWeaponFire02")
	);
	if (FireSoundFinder.Succeeded())
	{
		FireSound = FireSoundFinder.Object;
	}
}

// ============================================================================
// PISTOL - Officer sidearm
// ============================================================================

AGIPistolWeapon::AGIPistolWeapon()
{
	// Magazine
	MagazineSize = 12;
	ReserveAmmo = 48;
	ReloadTime = 1.8f;

	// Fire behavior - semi-auto
	bFullAuto = false;
	RefireRate = 0.3f;
	AimVariance = 1.5f;
	FiringRecoil = 2.5f;

	// Muzzle
	MuzzleSocketName = FName("Muzzle");
	MuzzleOffset = 8.0f;

	// AI perception
	ShotLoudness = 0.8f;
	ShotNoiseRange = 4000.0f;

	// Audio
	FireSoundVolume = 0.9f;

	// Projectile class
	ProjectileClass = AGIBulletProjectile::StaticClass();

	// Load weapon mesh (KA47 as fallback - ideally replace with pistol mesh)
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> PistolMeshFinder(
		TEXT("/Game/FPS_Weapon_Bundle/Weapons/Meshes/Ka47/SK_KA47.SK_KA47")
	);
	if (PistolMeshFinder.Succeeded())
	{
		GetFirstPersonMesh()->SetSkeletalMesh(PistolMeshFinder.Object);
		GetThirdPersonMesh()->SetSkeletalMesh(PistolMeshFinder.Object);
		GetThirdPersonMesh()->SetRelativeScale3D(FVector(0.7f));
	}

	// Load fire sound
	static ConstructorHelpers::FObjectFinder<USoundBase> FireSoundFinder(
		TEXT("/Game/Weapons/GrenadeLauncher/Audio/FirstPersonTemplateWeaponFire02.FirstPersonTemplateWeaponFire02")
	);
	if (FireSoundFinder.Succeeded())
	{
		FireSound = FireSoundFinder.Object;
	}
}

// ============================================================================
// SMG - Close-quarters weapon
// ============================================================================

AGISMGWeapon::AGISMGWeapon()
{
	// Magazine
	MagazineSize = 25;
	ReserveAmmo = 100;
	ReloadTime = 2.0f;

	// Fire behavior - full auto, high fire rate
	bFullAuto = true;
	RefireRate = 0.07f;
	AimVariance = 3.0f;
	FiringRecoil = 1.0f;

	// Muzzle
	MuzzleSocketName = FName("Muzzle");
	MuzzleOffset = 8.0f;

	// AI perception
	ShotLoudness = 0.7f;
	ShotNoiseRange = 3500.0f;

	// Audio
	FireSoundVolume = 0.85f;

	// Projectile class
	ProjectileClass = AGIBulletProjectile::StaticClass();

	// Load weapon mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SMGMeshFinder(
		TEXT("/Game/FPS_Weapon_Bundle/Weapons/Meshes/Ka47/SK_KA47.SK_KA47")
	);
	if (SMGMeshFinder.Succeeded())
	{
		GetFirstPersonMesh()->SetSkeletalMesh(SMGMeshFinder.Object);
		GetThirdPersonMesh()->SetSkeletalMesh(SMGMeshFinder.Object);
		GetThirdPersonMesh()->SetRelativeScale3D(FVector(0.85f));
	}

	// Load fire sound
	static ConstructorHelpers::FObjectFinder<USoundBase> FireSoundFinder(
		TEXT("/Game/Weapons/GrenadeLauncher/Audio/FirstPersonTemplateWeaponFire02.FirstPersonTemplateWeaponFire02")
	);
	if (FireSoundFinder.Succeeded())
	{
		FireSound = FireSoundFinder.Object;
	}
}

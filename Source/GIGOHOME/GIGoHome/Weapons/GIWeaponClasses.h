// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Concrete C++ Weapon Classes
// No Blueprint required. Production-ready.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "Variant_Shooter/Weapons/ShooterProjectile.h"
#include "GIWeaponClasses.generated.h"

class AGIBulletProjectile;

// ============================================================================
// BULLET PROJECTILE - Concrete hitscan-style bullet
// ============================================================================

UCLASS()
class GIGOHOME_API AGIBulletProjectile : public AShooterProjectile
{
	GENERATED_BODY()

public:
	AGIBulletProjectile();
};

// ============================================================================
// RIFLE - AK-47 style assault rifle
// Per GDD: Primary weapon for Xi Dong and most enemy soldiers
// ============================================================================

UCLASS()
class GIGOHOME_API AGIRifleWeapon : public AShooterWeapon
{
	GENERATED_BODY()

public:
	AGIRifleWeapon();
};

// ============================================================================
// PISTOL - Sidearm for officers
// ============================================================================

UCLASS()
class GIGOHOME_API AGIPistolWeapon : public AShooterWeapon
{
	GENERATED_BODY()

public:
	AGIPistolWeapon();
};

// ============================================================================
// SMG - Close-quarters weapon
// ============================================================================

UCLASS()
class GIGOHOME_API AGISMGWeapon : public AShooterWeapon
{
	GENERATED_BODY()

public:
	AGISMGWeapon();
};

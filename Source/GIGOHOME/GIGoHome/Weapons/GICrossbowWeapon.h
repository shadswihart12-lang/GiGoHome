// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Silent Crossbow Weapon
// Mission 1 starting weapon: 4 bolts, low noise signature

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "Variant_Shooter/Weapons/ShooterProjectile.h"
#include "GICrossbowWeapon.generated.h"

/**
 * Bolt projectile tuned for stealth use.
 */
UCLASS()
class GIGOHOME_API AGICrossbowBoltProjectile : public AShooterProjectile
{
	GENERATED_BODY()

public:
	AGICrossbowBoltProjectile();
};

/**
 * Xi Dong's crossbow for Act 1 Mission 1.
 * - Single shot (no auto)
 * - 4 bolts total
 * - very low firing noise
 * - quick handling, slower projectile
 */
UCLASS()
class GIGOHOME_API AGICrossbowWeapon : public AShooterWeapon
{
	GENERATED_BODY()

public:
	AGICrossbowWeapon();
};

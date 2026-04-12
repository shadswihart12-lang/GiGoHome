// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Silent Crossbow Weapon Implementation

#include "GIGoHome/Weapons/GICrossbowWeapon.h"

AGICrossbowBoltProjectile::AGICrossbowBoltProjectile()
{
	// Stealth projectile: very quiet impact and moderate damage.
	NoiseLoudness = 0.2f;
	NoiseRange = 500.0f;
	NoiseTag = FName("BoltImpact");
	HitDamage = 70.0f;
	PhysicsForce = 80.0f;
	bExplodeOnHit = false;
}

AGICrossbowWeapon::AGICrossbowWeapon()
{
	ProjectileClass = AGICrossbowBoltProjectile::StaticClass();

	MagazineSize = 1;
	ReserveAmmo = 3; // total 4 bolts at mission start
	ReloadTime = 1.4f;

	bFullAuto = false;
	RefireRate = 0.9f;
	AimVariance = 0.4f;
	FiringRecoil = 2.0f;

	ShotLoudness = 0.08f;
	ShotNoiseRange = 300.0f;
	ShotNoiseTag = FName("CrossbowShot");

	MuzzleSocketName = FName("Muzzle");
	MuzzleOffset = 6.0f;

}

// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Concrete Enemy Archetype Implementations

#include "GIGoHome/AI/GIEnemyArchetypes.h"
#include "GIGoHome/Weapons/GIWeaponClasses.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/ConstructorHelpers.h"

// ============================================================================
// GREEN - Nervous Rookies
// ============================================================================

AGIEnemyGreen::AGIEnemyGreen()
{
	// Set archetype
	Archetype = EEnemyArchetype::Green;

	// Per GDD: Feel fear quickly, low morale
	FearLevel = 0.1f;
	MoraleLevel = 0.7f;
	RoutMoraleThreshold = 0.35f;
	MoralePerDeathLoss = 0.3f;

	// Slower, less accurate
	ShootInterval = 2.0f;
	ShootRange = 1500.0f;
	StopDistance = 400.0f;

	GetCharacterMovement()->MaxWalkSpeed = 250.0f;

	// Load default skeletal mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple")
	);
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}

	// Use C++ rifle weapon class directly
	WeaponClass = AGIRifleWeapon::StaticClass();
}

AGIEnemyVeteran::AGIEnemyVeteran()
{
	// Set archetype
	Archetype = EEnemyArchetype::Veteran;

	// Per GDD: Most dangerous, high morale, low fear
	FearLevel = 0.0f;
	MoraleLevel = 0.9f;
	RoutMoraleThreshold = 0.15f;
	MoralePerDeathLoss = 0.2f;

	// Faster, more accurate, shoots more often
	ShootInterval = 1.2f;
	ShootRange = 2500.0f;
	StopDistance = 350.0f;

	GetCharacterMovement()->MaxWalkSpeed = 320.0f;

	// Load default skeletal mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple")
	);
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}

	// Use C++ rifle weapon class directly
	WeaponClass = AGIRifleWeapon::StaticClass();
}

// ============================================================================
// BROKEN - Crossed the Line
// ============================================================================

AGIEnemyBroken::AGIEnemyBroken()
{
	// Set archetype
	Archetype = EEnemyArchetype::Broken;

	// Per GDD: No mercy, no ambiguity - very aggressive
	FearLevel = 0.0f;
	MoraleLevel = 1.0f;
	RoutMoraleThreshold = 0.1f;
	MoralePerDeathLoss = 0.15f;

	// Very aggressive - fast, high fire rate
	ShootInterval = 0.8f;
	ShootRange = 2000.0f;
	StopDistance = 250.0f;

	GetCharacterMovement()->MaxWalkSpeed = 380.0f;

	// Load default skeletal mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple")
	);
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}

	// Use C++ rifle weapon class directly
	WeaponClass = AGIRifleWeapon::StaticClass();
}

// ============================================================================
// OFFICER - Commands from Rear
// ============================================================================

AGIEnemyOfficer::AGIEnemyOfficer()
{
	// Set archetype
	Archetype = EEnemyArchetype::Officer;

	// Per GDD: Priority target - eliminating causes squad cohesion loss
	FearLevel = 0.0f;
	MoraleLevel = 1.0f;
	RoutMoraleThreshold = 0.05f;
	MoralePerDeathLoss = 0.1f;

	// Stays back, commands others
	ShootInterval = 2.5f;
	ShootRange = 3000.0f;
	StopDistance = 600.0f;

	// Slower - directs rather than charges
	GetCharacterMovement()->MaxWalkSpeed = 200.0f;

	// Officers have radio
	bHasRadio = true;

	// Load default skeletal mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple")
	);
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}

	// Officers use pistol - C++ class directly
	WeaponClass = AGIPistolWeapon::StaticClass();
}

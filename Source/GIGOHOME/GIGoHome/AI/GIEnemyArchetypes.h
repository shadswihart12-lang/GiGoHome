// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Concrete Enemy Archetypes
// Production-ready with default meshes and animations

#pragma once

#include "CoreMinimal.h"
#include "GIGoHome/AI/GIEnemyCharacter.h"
#include "GIEnemyArchetypes.generated.h"

/**
 * GREEN ARCHETYPE
 * Per GDD: "Young, nervous rookies - unpredictable, make mistakes, feel fear quickly"
 * Lower morale, higher fear sensitivity, slower reaction time
 */
UCLASS()
class GIGOHOME_API AGIEnemyGreen : public AGIEnemyCharacter
{
	GENERATED_BODY()

public:
	AGIEnemyGreen();
};

/**
 * VETERAN ARCHETYPE
 * Per GDD: "Battle-hardened veterans - check corners, communicate, most dangerous"
 * High morale, low fear, fast reactions, accurate shots
 */
UCLASS()
class GIGOHOME_API AGIEnemyVeteran : public AGIEnemyCharacter
{
	GENERATED_BODY()

public:
	AGIEnemyVeteran();
};

/**
 * BROKEN ARCHETYPE
 * Per GDD: "Soldiers who crossed the line - burn villages, no mercy, no ambiguity"
 * Very high morale, almost no fear, aggressive behavior
 */
UCLASS()
class GIGOHOME_API AGIEnemyBroken : public AGIEnemyCharacter
{
	GENERATED_BODY()

public:
	AGIEnemyBroken();
};

/**
 * OFFICER ARCHETYPE
 * Per GDD: "Commands from the rear - eliminating causes squad cohesion loss"
 * Maximum morale resistance, stays back, eliminating causes cascade rout
 */
UCLASS()
class GIGOHOME_API AGIEnemyOfficer : public AGIEnemyCharacter
{
	GENERATED_BODY()

public:
	AGIEnemyOfficer();
};

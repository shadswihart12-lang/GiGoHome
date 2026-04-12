// Copyright Epic Games, Inc. All Rights Reserved.
// Procedural Foliage - Data types for spawn rules

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "FoliageSpawnerTypes.generated.h"

/**
 * Defines one type of foliage (e.g. a tree, bush, or grass clump)
 * and the rules governing how it gets scattered across the landscape.
 */
USTRUCT(BlueprintType)
struct FFoliageEntry
{
	GENERATED_BODY()

	/** The mesh to spawn (tree, bush, grass, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh")
	TSoftObjectPtr<UStaticMesh> Mesh;

	/** Optional material override (leave empty to use mesh default) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh")
	TSoftObjectPtr<UMaterialInterface> MaterialOverride;

	/** Instances per 10 000 cm^2 (100 m^2). Higher = denser. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Density", meta=(ClampMin=0.0f, ClampMax=500.0f))
	float DensityPerHectare = 20.0f;

	/** Minimum random scale applied to each instance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scale", meta=(ClampMin=0.01f))
	float MinScale = 0.8f;

	/** Maximum random scale applied to each instance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scale", meta=(ClampMin=0.01f))
	float MaxScale = 1.2f;

	/** Maximum slope angle (degrees) this foliage can grow on. 0 = flat only, 90 = any slope. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Placement", meta=(ClampMin=0.0f, ClampMax=90.0f, Units="deg"))
	float MaxSlopeAngle = 45.0f;

	/** Minimum altitude (Z) this foliage can appear at (world units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Placement")
	float MinAltitude = -100000.0f;

	/** Maximum altitude (Z) this foliage can appear at (world units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Placement")
	float MaxAltitude = 100000.0f;

	/** Minimum distance between instances of this type (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spacing", meta=(ClampMin=0.0f))
	float MinSpacing = 50.0f;

	/** Align instance rotation to surface normal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rotation")
	bool bAlignToSurface = false;

	/** Add random yaw rotation (0-360) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rotation")
	bool bRandomYaw = true;

	/** Maximum random pitch/roll tilt in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rotation", meta=(ClampMin=0.0f, ClampMax=45.0f, Units="deg"))
	float MaxTilt = 5.0f;

	/** Sink instances this far into the ground (cm). Useful for trees/bushes with visible root bases. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Placement", meta=(ClampMin=0.0f))
	float GroundOffset = 0.0f;

	/** Collision channel to trace against when placing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Placement")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_WorldStatic;

	/** Cast shadows */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rendering")
	bool bCastShadow = true;
};

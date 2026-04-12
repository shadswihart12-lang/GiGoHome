// Copyright Epic Games, Inc. All Rights Reserved.
// Vietnam Jungle Foliage Spawner - Pre-configured for Quang Tri Province jungle

#pragma once

#include "CoreMinimal.h"
#include "GIGoHome/Foliage/ProceduralFoliageSpawnerActor.h"
#include "VietnamJungleFoliageSpawner.generated.h"

/**
 * Pre-configured procedural foliage spawner for Vietnam jungle environments.
 * 
 * Features authentic Quang Tri Province vegetation:
 * - Dense bamboo groves along rivers and trails
 * - Tropical hardwood trees (teak, mahogany analogs)
 * - Ground cover ferns and elephant grass
 * - Banana plants near villages
 * - Rice paddies vegetation (if placed in water areas)
 * 
 * Usage:
 *   1. Place in level covering your jungle area
 *   2. Assign meshes to the exposed properties (or use defaults)
 *   3. Call Regenerate() or let it auto-spawn on BeginPlay
 * 
 * The spawner uses altitude and slope to place appropriate vegetation:
 *   - River banks: dense bamboo
 *   - Flat areas: elephant grass
 *   - Hillsides: hardwood trees
 *   - Slopes >45°: exposed rock (no foliage)
 */
UCLASS(Blueprintable, meta=(DisplayName="Vietnam Jungle Foliage Spawner"))
class GIGOHOME_API AVietnamJungleFoliageSpawner : public AProceduralFoliageSpawnerActor
{
	GENERATED_BODY()

public:

	AVietnamJungleFoliageSpawner();

	// --- Quick-assign mesh properties ---
	// Set these in Blueprint defaults or per-instance

	/** Main jungle tree mesh (e.g., tropical hardwood) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vietnam|Trees")
	TSoftObjectPtr<UStaticMesh> JungleTreeMesh;

	/** Bamboo cluster mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vietnam|Trees")
	TSoftObjectPtr<UStaticMesh> BambooMesh;

	/** Banana plant mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vietnam|Plants")
	TSoftObjectPtr<UStaticMesh> BananaPlantMesh;

	/** Fern mesh for ground cover */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vietnam|Ground Cover")
	TSoftObjectPtr<UStaticMesh> FernMesh;

	/** Elephant grass / tall grass mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vietnam|Ground Cover")
	TSoftObjectPtr<UStaticMesh> TallGrassMesh;

	/** Bush/shrub mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vietnam|Ground Cover")
	TSoftObjectPtr<UStaticMesh> BushMesh;

	// --- Density multipliers ---

	/** Overall density multiplier (1.0 = default, 2.0 = double density) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vietnam|Density", meta=(ClampMin=0.1f, ClampMax=5.0f))
	float DensityMultiplier = 1.0f;

	/** Extra bamboo density near water (rivers, ponds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vietnam|Density", meta=(ClampMin=0.0f, ClampMax=3.0f))
	float BambooDensityBoost = 1.5f;

	/** Ground cover density in clearings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vietnam|Density", meta=(ClampMin=0.0f, ClampMax=5.0f))
	float GroundCoverDensity = 2.0f;

	// --- Configuration ---

	/** Rebuild foliage entries from the quick-assign meshes */
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Vietnam")
	void RebuildFoliageEntries();

protected:

	virtual void BeginPlay() override;

private:

	/** Creates default foliage entries if none are assigned */
	void SetupDefaultFoliageEntries();
};

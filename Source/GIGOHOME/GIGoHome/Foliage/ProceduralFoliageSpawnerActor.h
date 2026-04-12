// Copyright Epic Games, Inc. All Rights Reserved.
// Procedural Foliage Spawner - scatters foliage instances across the landscape

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FoliageSpawnerTypes.h"
#include "ProceduralFoliageSpawnerActor.generated.h"

class UBoxComponent;
class UHierarchicalInstancedStaticMeshComponent;
class UBillboardComponent;

/**
 * Drop this actor into a level to procedurally scatter foliage
 * (trees, grass, bushes) within its bounds.
 *
 * Usage:
 *  1. Place in level and scale/move the box to cover the target area.
 *  2. Add entries to the FoliageTypes array (assign meshes, tweak density/scale).
 *  3. Hit "Regenerate" in the Details panel (or call Regenerate at runtime).
 *
 * Instances are spawned via HierarchicalInstancedStaticMeshComponents
 * for efficient GPU instancing and LOD.
 */
UCLASS(Blueprintable, meta=(DisplayName="Procedural Foliage Spawner"))
class GIGOHOME_API AProceduralFoliageSpawnerActor : public AActor
{
	GENERATED_BODY()

public:

	AProceduralFoliageSpawnerActor();

	// --- Configuration ---

	/** The types of foliage to scatter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Foliage")
	TArray<FFoliageEntry> FoliageTypes;

	/** Random seed - same seed produces the same layout every time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Foliage")
	int32 Seed = 42;

	/** Automatically regenerate foliage when the game starts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Foliage")
	bool bSpawnOnBeginPlay = true;

	// --- Spawn Area ---

	/** Half-extent of the spawn area (X) in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Area", meta=(ClampMin=100.0f, Units="cm"))
	float AreaHalfExtentX = 5000.0f;

	/** Half-extent of the spawn area (Y) in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Area", meta=(ClampMin=100.0f, Units="cm"))
	float AreaHalfExtentY = 5000.0f;

	/** How high above the actor to start the ground trace (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Area", meta=(ClampMin=0.0f))
	float TraceHeightAbove = 5000.0f;

	/** How far below the actor to end the ground trace (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Area", meta=(ClampMin=0.0f))
	float TraceHeightBelow = 5000.0f;

	// --- Actions ---

	/** Clears existing foliage and re-scatters everything */
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Foliage")
	void Regenerate();

	/** Removes all spawned foliage instances */
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Foliage")
	void ClearFoliage();

	/** Returns total number of foliage instances currently spawned */
	UFUNCTION(BlueprintPure, Category="Foliage")
	int32 GetTotalInstanceCount() const;

protected:

	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:

	/** Visual bounds in editor */
	UPROPERTY(VisibleAnywhere, Category="Components")
	UBoxComponent* SpawnBounds;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UBillboardComponent* SpriteComponent;
#endif

	/** One HISM component per foliage type (created at runtime) */
	UPROPERTY()
	TArray<UHierarchicalInstancedStaticMeshComponent*> MeshComponents;

	/** Scatter instances for a single foliage entry */
	void SpawnFoliageType(int32 EntryIndex, FRandomStream& RNG);

	/** Line-trace to find the ground at an XY position */
	bool TraceGround(const FVector& XYPosition, FVector& OutLocation, FVector& OutNormal) const;
};

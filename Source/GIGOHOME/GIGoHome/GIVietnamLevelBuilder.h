// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Vietnam Level Builder
// Creates a playable jungle level with terrain, collision, and foliage
// NO CESIUM - Just works

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GIVietnamLevelBuilder.generated.h"

class UStaticMeshComponent;
class UHierarchicalInstancedStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;

UCLASS()
class GIGOHOME_API AGIVietnamLevelBuilder : public AActor
{
	GENERATED_BODY()

public:
	AGIVietnamLevelBuilder();

	virtual void BeginPlay() override;

	/** Rebuild the level in editor */
	UFUNCTION(CallInEditor, Category="Level Builder")
	void RebuildLevel();

protected:

	// ============== TERRAIN ==============

	/** Size of the terrain (units) */
	UPROPERTY(EditAnywhere, Category="Terrain")
	float TerrainSize = 50000.0f;

	/** Number of ground tiles per axis */
	UPROPERTY(EditAnywhere, Category="Terrain")
	int32 TerrainTiles = 5;

	// ============== FOLIAGE ==============

	/** Tree mesh - assign in Blueprint */
	UPROPERTY(EditAnywhere, Category="Foliage")
	UStaticMesh* TreeMesh;

	/** Number of trees */
	UPROPERTY(EditAnywhere, Category="Foliage")
	int32 TreeCount = 300;

	/** Bush/shrub mesh */
	UPROPERTY(EditAnywhere, Category="Foliage")
	UStaticMesh* BushMesh;

	/** Number of bushes */
	UPROPERTY(EditAnywhere, Category="Foliage")
	int32 BushCount = 800;

	/** Grass mesh */
	UPROPERTY(EditAnywhere, Category="Foliage")
	UStaticMesh* GrassMesh;

	/** Number of grass clusters */
	UPROPERTY(EditAnywhere, Category="Foliage")
	int32 GrassCount = 2000;

	/** Rock mesh */
	UPROPERTY(EditAnywhere, Category="Foliage")
	UStaticMesh* RockMesh;

	/** Number of rocks */
	UPROPERTY(EditAnywhere, Category="Foliage")
	int32 RockCount = 100;

	// ============== COMPONENTS ==============

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere)
	UHierarchicalInstancedStaticMeshComponent* TreeInstances;

	UPROPERTY(VisibleAnywhere)
	UHierarchicalInstancedStaticMeshComponent* BushInstances;

	UPROPERTY(VisibleAnywhere)
	UHierarchicalInstancedStaticMeshComponent* GrassInstances;

	UPROPERTY(VisibleAnywhere)
	UHierarchicalInstancedStaticMeshComponent* RockInstances;

	/** Generated floor tiles - stored so we can clean them up */
	UPROPERTY()
	TArray<UStaticMeshComponent*> FloorTiles;

	// ============== INTERNAL ==============

	void BuildTerrain();
	void BuildFoliage();
	void ClearAll();
};

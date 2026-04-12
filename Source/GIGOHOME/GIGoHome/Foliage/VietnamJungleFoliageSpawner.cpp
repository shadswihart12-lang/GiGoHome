// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Foliage/VietnamJungleFoliageSpawner.h"
#include "Engine/StaticMesh.h"

AVietnamJungleFoliageSpawner::AVietnamJungleFoliageSpawner()
{
	// Default area covers a large jungle section
	AreaHalfExtentX = 10000.0f;
	AreaHalfExtentY = 10000.0f;
	TraceHeightAbove = 10000.0f;
	TraceHeightBelow = 5000.0f;
}

void AVietnamJungleFoliageSpawner::BeginPlay()
{
	// Build entries from quick-assign meshes before spawning
	if (FoliageTypes.Num() == 0)
	{
		SetupDefaultFoliageEntries();
	}

	Super::BeginPlay();
}

void AVietnamJungleFoliageSpawner::RebuildFoliageEntries()
{
	SetupDefaultFoliageEntries();
	Regenerate();
}

void AVietnamJungleFoliageSpawner::SetupDefaultFoliageEntries()
{
	FoliageTypes.Empty();

	// ============================================================
	// LAYER 1: CANOPY TREES — Tropical hardwoods
	// ============================================================
	if (!JungleTreeMesh.IsNull())
	{
		FFoliageEntry TreeEntry;
		TreeEntry.Mesh = JungleTreeMesh;
		TreeEntry.DensityPerHectare = 8.0f * DensityMultiplier;
		TreeEntry.MinScale = 0.8f;
		TreeEntry.MaxScale = 1.4f;
		TreeEntry.MaxSlopeAngle = 35.0f;
		TreeEntry.MinSpacing = 800.0f;
		TreeEntry.bRandomYaw = true;
		TreeEntry.MaxTilt = 3.0f;
		TreeEntry.GroundOffset = 20.0f;
		TreeEntry.bCastShadow = true;
		FoliageTypes.Add(TreeEntry);
	}

	// ============================================================
	// LAYER 2: BAMBOO CLUSTERS — Dense along trails and water
	// ============================================================
	if (!BambooMesh.IsNull())
	{
		FFoliageEntry BambooEntry;
		BambooEntry.Mesh = BambooMesh;
		BambooEntry.DensityPerHectare = 25.0f * DensityMultiplier * BambooDensityBoost;
		BambooEntry.MinScale = 0.7f;
		BambooEntry.MaxScale = 1.2f;
		BambooEntry.MaxSlopeAngle = 40.0f;
		BambooEntry.MinSpacing = 200.0f;
		BambooEntry.bRandomYaw = true;
		BambooEntry.MaxTilt = 5.0f;
		BambooEntry.GroundOffset = 5.0f;
		BambooEntry.bCastShadow = true;
		FoliageTypes.Add(BambooEntry);
	}

	// ============================================================
	// LAYER 3: BANANA PLANTS — Near clearings and villages
	// ============================================================
	if (!BananaPlantMesh.IsNull())
	{
		FFoliageEntry BananaEntry;
		BananaEntry.Mesh = BananaPlantMesh;
		BananaEntry.DensityPerHectare = 5.0f * DensityMultiplier;
		BananaEntry.MinScale = 0.8f;
		BananaEntry.MaxScale = 1.1f;
		BananaEntry.MaxSlopeAngle = 25.0f;
		BananaEntry.MinSpacing = 300.0f;
		BananaEntry.bRandomYaw = true;
		BananaEntry.MaxTilt = 2.0f;
		BananaEntry.GroundOffset = 0.0f;
		BananaEntry.bCastShadow = true;
		FoliageTypes.Add(BananaEntry);
	}

	// ============================================================
	// LAYER 4: BUSHES/SHRUBS — Mid-layer vegetation
	// ============================================================
	if (!BushMesh.IsNull())
	{
		FFoliageEntry BushEntry;
		BushEntry.Mesh = BushMesh;
		BushEntry.DensityPerHectare = 40.0f * DensityMultiplier;
		BushEntry.MinScale = 0.6f;
		BushEntry.MaxScale = 1.0f;
		BushEntry.MaxSlopeAngle = 45.0f;
		BushEntry.MinSpacing = 150.0f;
		BushEntry.bRandomYaw = true;
		BushEntry.MaxTilt = 5.0f;
		BushEntry.GroundOffset = 0.0f;
		BushEntry.bCastShadow = false;
		FoliageTypes.Add(BushEntry);
	}

	// ============================================================
	// LAYER 5: FERNS — Dense ground cover
	// ============================================================
	if (!FernMesh.IsNull())
	{
		FFoliageEntry FernEntry;
		FernEntry.Mesh = FernMesh;
		FernEntry.DensityPerHectare = 80.0f * DensityMultiplier * GroundCoverDensity;
		FernEntry.MinScale = 0.5f;
		FernEntry.MaxScale = 1.0f;
		FernEntry.MaxSlopeAngle = 50.0f;
		FernEntry.MinSpacing = 60.0f;
		FernEntry.bRandomYaw = true;
		FernEntry.MaxTilt = 8.0f;
		FernEntry.GroundOffset = 0.0f;
		FernEntry.bCastShadow = false;
		FoliageTypes.Add(FernEntry);
	}

	// ============================================================
	// LAYER 6: ELEPHANT GRASS / TALL GRASS — Clearings and edges
	// ============================================================
	if (!TallGrassMesh.IsNull())
	{
		FFoliageEntry GrassEntry;
		GrassEntry.Mesh = TallGrassMesh;
		GrassEntry.DensityPerHectare = 120.0f * DensityMultiplier * GroundCoverDensity;
		GrassEntry.MinScale = 0.4f;
		GrassEntry.MaxScale = 0.9f;
		GrassEntry.MaxSlopeAngle = 30.0f;
		GrassEntry.MinSpacing = 40.0f;
		GrassEntry.bRandomYaw = true;
		GrassEntry.MaxTilt = 10.0f;
		GrassEntry.GroundOffset = 0.0f;
		GrassEntry.bCastShadow = false;
		FoliageTypes.Add(GrassEntry);
	}

	UE_LOG(LogTemp, Log, TEXT("VietnamJungleFoliageSpawner: Configured %d foliage types"), FoliageTypes.Num());
}

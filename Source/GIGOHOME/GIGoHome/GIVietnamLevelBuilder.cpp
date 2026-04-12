// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/GIVietnamLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"

AGIVietnamLevelBuilder::AGIVietnamLevelBuilder()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Create HISM components for foliage
	TreeInstances = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Trees"));
	TreeInstances->SetupAttachment(Root);
	TreeInstances->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TreeInstances->SetCollisionResponseToAllChannels(ECR_Block);

	BushInstances = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Bushes"));
	BushInstances->SetupAttachment(Root);
	BushInstances->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BushInstances->SetCollisionResponseToAllChannels(ECR_Overlap);

	GrassInstances = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Grass"));
	GrassInstances->SetupAttachment(Root);
	GrassInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RockInstances = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Rocks"));
	RockInstances->SetupAttachment(Root);
	RockInstances->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RockInstances->SetCollisionResponseToAllChannels(ECR_Block);

	// Try to load default plane mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneFinder(
		TEXT("/Engine/BasicShapes/Plane.Plane")
	);
}

void AGIVietnamLevelBuilder::BeginPlay()
{
	Super::BeginPlay();

	// Build level on play if not already built
	if (FloorTiles.Num() == 0)
	{
		RebuildLevel();
	}
}

void AGIVietnamLevelBuilder::RebuildLevel()
{
	ClearAll();
	BuildTerrain();
	BuildFoliage();

	UE_LOG(LogTemp, Warning, TEXT("GIVietnamLevelBuilder: Level built! %d trees, %d bushes, %d grass, %d rocks"),
		TreeInstances->GetInstanceCount(),
		BushInstances->GetInstanceCount(),
		GrassInstances->GetInstanceCount(),
		RockInstances->GetInstanceCount()
	);
}

void AGIVietnamLevelBuilder::ClearAll()
{
	// Destroy floor tiles
	for (UStaticMeshComponent* Tile : FloorTiles)
	{
		if (Tile)
		{
			Tile->DestroyComponent();
		}
	}
	FloorTiles.Empty();

	// Clear foliage instances
	TreeInstances->ClearInstances();
	BushInstances->ClearInstances();
	GrassInstances->ClearInstances();
	RockInstances->ClearInstances();
}

void AGIVietnamLevelBuilder::BuildTerrain()
{
	// Load the plane mesh
	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (!PlaneMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("GIVietnamLevelBuilder: Could not load Plane mesh!"));
		return;
	}

	// Load a basic ground material
	UMaterialInterface* GroundMat = LoadObject<UMaterialInterface>(nullptr, 
		TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial"));

	const float TileSize = TerrainSize / TerrainTiles;
	const float HalfTerrain = TerrainSize / 2.0f;
	const float PlaneScale = TileSize / 100.0f; // Plane is 100x100 by default

	for (int32 X = 0; X < TerrainTiles; X++)
	{
		for (int32 Y = 0; Y < TerrainTiles; Y++)
		{
			// Create a floor tile
			UStaticMeshComponent* Tile = NewObject<UStaticMeshComponent>(this);
			Tile->SetupAttachment(Root);
			Tile->SetStaticMesh(PlaneMesh);
			Tile->RegisterComponent();

			// Position tile
			float PosX = (X * TileSize) - HalfTerrain + (TileSize / 2.0f);
			float PosY = (Y * TileSize) - HalfTerrain + (TileSize / 2.0f);
			
			// Add some height variation for hills
			float HeightNoise = FMath::PerlinNoise2D(FVector2D(X * 0.3f, Y * 0.3f)) * 500.0f;
			
			Tile->SetRelativeLocation(FVector(PosX, PosY, HeightNoise));
			Tile->SetRelativeScale3D(FVector(PlaneScale, PlaneScale, 1.0f));

			// Set material if available
			if (GroundMat)
			{
				Tile->SetMaterial(0, GroundMat);
			}

			// CRITICAL: Enable collision!
			Tile->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Tile->SetCollisionResponseToAllChannels(ECR_Block);
			Tile->SetCollisionProfileName(TEXT("BlockAll"));

			FloorTiles.Add(Tile);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("GIVietnamLevelBuilder: Created %d floor tiles with collision"), FloorTiles.Num());
}

void AGIVietnamLevelBuilder::BuildFoliage()
{
	const float HalfTerrain = TerrainSize / 2.0f;

	// Helper lambda to get ground height at position
	auto GetGroundZ = [this, HalfTerrain](float X, float Y) -> float
	{
		// Approximate height based on perlin noise (matches terrain generation)
		float GridX = ((X + HalfTerrain) / TerrainSize) * TerrainTiles * 0.3f;
		float GridY = ((Y + HalfTerrain) / TerrainSize) * TerrainTiles * 0.3f;
		return FMath::PerlinNoise2D(FVector2D(GridX, GridY)) * 500.0f;
	};

	// Spawn Trees
	if (TreeMesh)
	{
		TreeInstances->SetStaticMesh(TreeMesh);
		for (int32 i = 0; i < TreeCount; i++)
		{
			float X = FMath::RandRange(-HalfTerrain * 0.9f, HalfTerrain * 0.9f);
			float Y = FMath::RandRange(-HalfTerrain * 0.9f, HalfTerrain * 0.9f);
			float Z = GetGroundZ(X, Y);
			float Scale = FMath::RandRange(0.8f, 1.5f);
			float Yaw = FMath::RandRange(0.0f, 360.0f);

			FTransform TreeTransform;
			TreeTransform.SetLocation(FVector(X, Y, Z));
			TreeTransform.SetRotation(FQuat(FRotator(0.f, Yaw, 0.f)));
			TreeTransform.SetScale3D(FVector(Scale));

			TreeInstances->AddInstance(TreeTransform);
		}
	}

	// Spawn Bushes
	if (BushMesh)
	{
		BushInstances->SetStaticMesh(BushMesh);
		for (int32 i = 0; i < BushCount; i++)
		{
			float X = FMath::RandRange(-HalfTerrain * 0.95f, HalfTerrain * 0.95f);
			float Y = FMath::RandRange(-HalfTerrain * 0.95f, HalfTerrain * 0.95f);
			float Z = GetGroundZ(X, Y);
			float Scale = FMath::RandRange(0.5f, 1.2f);
			float Yaw = FMath::RandRange(0.0f, 360.0f);

			FTransform BushTransform;
			BushTransform.SetLocation(FVector(X, Y, Z));
			BushTransform.SetRotation(FQuat(FRotator(0.f, Yaw, 0.f)));
			BushTransform.SetScale3D(FVector(Scale));

			BushInstances->AddInstance(BushTransform);
		}
	}

	// Spawn Grass
	if (GrassMesh)
	{
		GrassInstances->SetStaticMesh(GrassMesh);
		for (int32 i = 0; i < GrassCount; i++)
		{
			float X = FMath::RandRange(-HalfTerrain * 0.95f, HalfTerrain * 0.95f);
			float Y = FMath::RandRange(-HalfTerrain * 0.95f, HalfTerrain * 0.95f);
			float Z = GetGroundZ(X, Y);
			float Scale = FMath::RandRange(0.3f, 0.8f);
			float Yaw = FMath::RandRange(0.0f, 360.0f);

			FTransform GrassTransform;
			GrassTransform.SetLocation(FVector(X, Y, Z));
			GrassTransform.SetRotation(FQuat(FRotator(0.f, Yaw, 0.f)));
			GrassTransform.SetScale3D(FVector(Scale));

			GrassInstances->AddInstance(GrassTransform);
		}
	}

	// Spawn Rocks
	if (RockMesh)
	{
		RockInstances->SetStaticMesh(RockMesh);
		for (int32 i = 0; i < RockCount; i++)
		{
			float X = FMath::RandRange(-HalfTerrain * 0.9f, HalfTerrain * 0.9f);
			float Y = FMath::RandRange(-HalfTerrain * 0.9f, HalfTerrain * 0.9f);
			float Z = GetGroundZ(X, Y);
			float Scale = FMath::RandRange(0.5f, 2.0f);
			float Yaw = FMath::RandRange(0.0f, 360.0f);
			float Pitch = FMath::RandRange(-15.0f, 15.0f);

			FTransform RockTransform;
			RockTransform.SetLocation(FVector(X, Y, Z - 20.0f)); // Sink slightly
			RockTransform.SetRotation(FQuat(FRotator(Pitch, Yaw, 0.f)));
			RockTransform.SetScale3D(FVector(Scale));

			RockInstances->AddInstance(RockTransform);
		}
	}
}

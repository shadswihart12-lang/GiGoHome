// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Foliage/ProceduralFoliageSpawnerActor.h"
#include "Components/BoxComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

#if WITH_EDITOR
#include "Components/BillboardComponent.h"
#endif

AProceduralFoliageSpawnerActor::AProceduralFoliageSpawnerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// root scene component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// visible bounds box (editor only, no collision)
	SpawnBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBounds"));
	SpawnBounds->SetupAttachment(Root);
	SpawnBounds->SetBoxExtent(FVector(5000.0f, 5000.0f, 500.0f));
	SpawnBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnBounds->SetHiddenInGame(true);
	SpawnBounds->ShapeColor = FColor::Green;

#if WITH_EDITORONLY_DATA
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (SpriteComponent)
	{
		SpriteComponent->SetupAttachment(Root);
	}
#endif
}

void AProceduralFoliageSpawnerActor::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnBeginPlay)
	{
		Regenerate();
	}
}

#if WITH_EDITOR
void AProceduralFoliageSpawnerActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// keep the visual box in sync with the area settings
	if (SpawnBounds)
	{
		SpawnBounds->SetBoxExtent(FVector(AreaHalfExtentX, AreaHalfExtentY, 500.0f));
	}
}
#endif

void AProceduralFoliageSpawnerActor::Regenerate()
{
	ClearFoliage();

	FRandomStream RNG(Seed);

	for (int32 i = 0; i < FoliageTypes.Num(); ++i)
	{
		const FFoliageEntry& Entry = FoliageTypes[i];

		// load the mesh (soft reference)
		UStaticMesh* LoadedMesh = Entry.Mesh.LoadSynchronous();
		if (!LoadedMesh)
		{
			continue;
		}

		// create an HISM component for this foliage type
		UHierarchicalInstancedStaticMeshComponent* HISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
		HISM->SetStaticMesh(LoadedMesh);
		HISM->SetCastShadow(Entry.bCastShadow);
		HISM->SetMobility(EComponentMobility::Static);
		HISM->NumCustomDataFloats = 0;

		// material override
		if (UMaterialInterface* Mat = Entry.MaterialOverride.LoadSynchronous())
		{
			for (int32 Section = 0; Section < LoadedMesh->GetStaticMaterials().Num(); ++Section)
			{
				HISM->SetMaterial(Section, Mat);
			}
		}

		HISM->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		HISM->RegisterComponent();

		MeshComponents.Add(HISM);

		// scatter instances
		SpawnFoliageType(i, RNG);
	}

	UE_LOG(LogTemp, Log, TEXT("ProceduralFoliageSpawner: Spawned %d total instances across %d types"), GetTotalInstanceCount(), FoliageTypes.Num());
}

void AProceduralFoliageSpawnerActor::ClearFoliage()
{
	for (UHierarchicalInstancedStaticMeshComponent* HISM : MeshComponents)
	{
		if (HISM)
		{
			HISM->ClearInstances();
			HISM->DestroyComponent();
		}
	}
	MeshComponents.Empty();
}

int32 AProceduralFoliageSpawnerActor::GetTotalInstanceCount() const
{
	int32 Total = 0;
	for (const UHierarchicalInstancedStaticMeshComponent* HISM : MeshComponents)
	{
		if (HISM)
		{
			Total += HISM->GetInstanceCount();
		}
	}
	return Total;
}

void AProceduralFoliageSpawnerActor::SpawnFoliageType(int32 EntryIndex, FRandomStream& RNG)
{
	if (!MeshComponents.IsValidIndex(EntryIndex))
	{
		return;
	}

	const FFoliageEntry& Entry = FoliageTypes[EntryIndex];
	UHierarchicalInstancedStaticMeshComponent* HISM = MeshComponents[EntryIndex];

	if (!HISM)
	{
		return;
	}

	const FVector Origin = GetActorLocation();

	// calculate how many instances to attempt based on density and area
	// DensityPerHectare = instances per 10,000 m^2 = instances per 100,000,000 cm^2
	const float AreaCm2 = (AreaHalfExtentX * 2.0f) * (AreaHalfExtentY * 2.0f);
	const float AreaHectares = AreaCm2 / 100000000.0f;
	const int32 TargetCount = FMath::RoundToInt32(Entry.DensityPerHectare * AreaHectares);

	if (TargetCount <= 0)
	{
		return;
	}

	const float MaxSlopeCos = FMath::Cos(FMath::DegreesToRadians(Entry.MaxSlopeAngle));
	const float MinSpacingSq = Entry.MinSpacing * Entry.MinSpacing;

	// track placed positions for spacing enforcement
	TArray<FVector2D> PlacedPositions;
	PlacedPositions.Reserve(TargetCount);

	// try placing instances (allow some extra attempts for rejected spots)
	const int32 MaxAttempts = TargetCount * 3;
	int32 Placed = 0;

	for (int32 Attempt = 0; Attempt < MaxAttempts && Placed < TargetCount; ++Attempt)
	{
		// random XY within area
		const float RandX = RNG.FRandRange(-AreaHalfExtentX, AreaHalfExtentX);
		const float RandY = RNG.FRandRange(-AreaHalfExtentY, AreaHalfExtentY);
		const FVector XYWorld = Origin + FVector(RandX, RandY, 0.0f);

		// trace to find ground
		FVector HitLocation, HitNormal;
		if (!TraceGround(XYWorld, HitLocation, HitNormal))
		{
			continue;
		}

		// altitude check
		if (HitLocation.Z < Entry.MinAltitude || HitLocation.Z > Entry.MaxAltitude)
		{
			continue;
		}

		// slope check (normal.Z = cos of slope angle from vertical)
		if (HitNormal.Z < MaxSlopeCos)
		{
			continue;
		}

		// minimum spacing check
		const FVector2D Pos2D(HitLocation.X, HitLocation.Y);
		bool bTooClose = false;
		for (const FVector2D& Existing : PlacedPositions)
		{
			if (FVector2D::DistSquared(Pos2D, Existing) < MinSpacingSq)
			{
				bTooClose = true;
				break;
			}
		}
		if (bTooClose)
		{
			continue;
		}

		// build transform
		const float Scale = RNG.FRandRange(Entry.MinScale, Entry.MaxScale);

		FRotator Rotation = FRotator::ZeroRotator;

		if (Entry.bAlignToSurface)
		{
			Rotation = HitNormal.Rotation();
			// adjust so Z-up becomes surface normal
			Rotation.Pitch -= 90.0f;
		}

		if (Entry.bRandomYaw)
		{
			Rotation.Yaw = RNG.FRandRange(0.0f, 360.0f);
		}

		if (Entry.MaxTilt > 0.0f)
		{
			Rotation.Pitch += RNG.FRandRange(-Entry.MaxTilt, Entry.MaxTilt);
			Rotation.Roll += RNG.FRandRange(-Entry.MaxTilt, Entry.MaxTilt);
		}

		FVector FinalLocation = HitLocation;
		FinalLocation.Z -= Entry.GroundOffset;

		const FTransform InstanceTransform(Rotation, FinalLocation, FVector(Scale));

		HISM->AddInstance(InstanceTransform, true);
		PlacedPositions.Add(Pos2D);
		++Placed;
	}
}

bool AProceduralFoliageSpawnerActor::TraceGround(const FVector& XYPosition, FVector& OutLocation, FVector& OutNormal) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector TraceStart = FVector(XYPosition.X, XYPosition.Y, GetActorLocation().Z + TraceHeightAbove);
	const FVector TraceEnd = FVector(XYPosition.X, XYPosition.Y, GetActorLocation().Z - TraceHeightBelow);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
	{
		OutLocation = Hit.ImpactPoint;
		OutNormal = Hit.ImpactNormal;
		return true;
	}

	return false;
}

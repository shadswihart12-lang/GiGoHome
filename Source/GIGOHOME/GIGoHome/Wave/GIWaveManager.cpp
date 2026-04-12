// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Wave/GIWaveManager.h"
#include "GIGoHome/AI/GIEnemyCharacter.h"
#include "GIGoHome/AI/GIEnemyArchetypes.h"
#include "GIGoHome/AI/GIPatrolComponent.h"
#include "GIGoHome/GIGoHomeSliceGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

AGIWaveManager::AGIWaveManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGIWaveManager::BeginPlay()
{
	Super::BeginPlay();

	CacheGameMode();
	BuildSpawnPoints();
	EnsureEnemyClasses();

	// Generate default wave config if none set in editor
	if (Waves.Num() == 0)
	{
		SetupDefaultWaves();
	}

	// Auto-start if enabled (default for placed-in-level managers)
	if (bAutoStart)
	{
		StartFirstWave();
	}
}

void AGIWaveManager::StartFirstWave()
{
	if (bHasStarted) return;
	bHasStarted = true;
	StartWave(0);
}

void AGIWaveManager::CacheGameMode()
{
	CachedGameMode = Cast<AGIGoHomeSliceGameMode>(GetWorld()->GetAuthGameMode());
}

void AGIWaveManager::EnsureEnemyClasses()
{
	// Auto-assign concrete C++ enemy classes if not set in editor
	if (!GreenClass)
	{
		GreenClass = AGIEnemyGreen::StaticClass();
	}
	if (!VeteranClass)
	{
		VeteranClass = AGIEnemyVeteran::StaticClass();
	}
	if (!BrokenClass)
	{
		BrokenClass = AGIEnemyBroken::StaticClass();
	}
	if (!OfficerClass)
	{
		OfficerClass = AGIEnemyOfficer::StaticClass();
	}
}

void AGIWaveManager::SetupDefaultWaves()
{
	// Per GDD: 5 escalating waves for the vertical slice
	// Total playtime target: 15-20 minutes

	// Wave 1: 3 Greens — rookie patrol, ~3 minutes
	FGIWaveConfig W1;
	W1.GreenCount = 3;
	W1.DelayBeforeSpawn = 0.0f; // Start immediately
	Waves.Add(W1);

	// Wave 2: 4 Greens + 1 Veteran — escalation, ~4 minutes
	FGIWaveConfig W2;
	W2.GreenCount = 4;
	W2.VeteranCount = 1;
	W2.DelayBeforeSpawn = 8.0f; // Breathing room between waves
	Waves.Add(W2);

	// Wave 3: 3 Greens + 2 Veterans + 1 Officer — officer is priority kill, ~4 minutes
	FGIWaveConfig W3;
	W3.GreenCount = 3;
	W3.VeteranCount = 2;
	W3.OfficerCount = 1;
	W3.DelayBeforeSpawn = 10.0f;
	Waves.Add(W3);

	// Wave 4: 2 Veterans + 2 Broken + 1 Officer — the Broken have crossed the line, ~4 minutes
	FGIWaveConfig W4;
	W4.VeteranCount = 2;
	W4.BrokenCount = 2;
	W4.OfficerCount = 1;
	W4.DelayBeforeSpawn = 12.0f;
	Waves.Add(W4);

	// Wave 5: Full force — the hardest push, ~5 minutes
	FGIWaveConfig W5;
	W5.GreenCount = 2;
	W5.VeteranCount = 3;
	W5.BrokenCount = 2;
	W5.OfficerCount = 1;
	W5.DelayBeforeSpawn = 15.0f;
	Waves.Add(W5);
}

void AGIWaveManager::BuildSpawnPoints()
{
	SpawnPoints.Empty();

	TArray<AActor*> SpawnActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), SpawnPointTag, SpawnActors);

	for (AActor* Actor : SpawnActors)
	{
		if (Actor)
		{
			// Trace down to find ground
			FHitResult Hit;
			FVector Start = Actor->GetActorLocation() + FVector(0.0f, 0.0f, 500.0f);
			FVector End = Actor->GetActorLocation() - FVector(0.0f, 0.0f, 1000.0f);
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(Actor);

			if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
			{
				SpawnPoints.Add(Hit.ImpactPoint + FVector(0.0f, 0.0f, 100.0f));
			}
			else
			{
				SpawnPoints.Add(Actor->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f));
			}
		}
	}

	// Fallback: Generate spawn points around the player start
	if (SpawnPoints.Num() == 0)
	{
		AActor* PlayerStart = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass());
		FVector Origin = PlayerStart ? PlayerStart->GetActorLocation() : FVector::ZeroVector;

		// Per GDD: Enemies spawn at distance, patrol toward player area
		const float MinRadius = 2000.0f;
		const float MaxRadius = 4000.0f;
		const int32 FallbackCount = 12;

		for (int32 i = 0; i < FallbackCount; i++)
		{
			const float Angle = (360.0f / FallbackCount) * i * (PI / 180.0f);
			const float Radius = FMath::RandRange(MinRadius, MaxRadius);
			FVector SpawnLoc = Origin + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);

			// Trace down to find ground
			FHitResult Hit;
			FVector Start = SpawnLoc + FVector(0.0f, 0.0f, 5000.0f);
			FVector End = SpawnLoc - FVector(0.0f, 0.0f, 5000.0f);
			FCollisionQueryParams Params;

			if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
			{
				SpawnPoints.Add(Hit.ImpactPoint + FVector(0.0f, 0.0f, 100.0f));
			}
			else
			{
				SpawnPoints.Add(SpawnLoc + FVector(0.0f, 0.0f, 100.0f));
			}
		}
	}
}

void AGIWaveManager::StartWave(int32 WaveIndex)
{
	if (!Waves.IsValidIndex(WaveIndex)) return;

	CurrentWaveIndex = WaveIndex;
	const FGIWaveConfig& Config = Waves[WaveIndex];

	// Notify delegates and game mode
	OnWaveStarted.Broadcast(WaveIndex);
	if (CachedGameMode)
	{
		CachedGameMode->NotifyWaveStarted(WaveIndex);
	}

	if (Config.DelayBeforeSpawn > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			WaveSpawnTimer,
			[this, WaveIndex]() { SpawnWaveEnemies(Waves[WaveIndex]); },
			Config.DelayBeforeSpawn,
			false
		);
	}
	else
	{
		SpawnWaveEnemies(Config);
	}
}

void AGIWaveManager::SpawnWaveEnemies(const FGIWaveConfig& Config)
{
	ActiveEnemies.Empty();

	SpawnEnemy(GreenClass, Config.GreenCount);
	SpawnEnemy(VeteranClass, Config.VeteranCount);
	SpawnEnemy(BrokenClass, Config.BrokenCount);
	SpawnEnemy(OfficerClass, Config.OfficerCount);

	bWaveActive = true;

	// Poll every 2 seconds to check if wave is cleared
	GetWorld()->GetTimerManager().SetTimer(
		WaveSpawnTimer,
		this,
		&AGIWaveManager::CheckWaveCleared,
		2.0f,
		true
	);
}

void AGIWaveManager::SpawnEnemy(TSubclassOf<AGIEnemyCharacter> EnemyClass, int32 Count)
{
	if (!EnemyClass || Count <= 0) return;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;

	for (int32 i = 0; i < Count; i++)
	{
		FVector SpawnLoc = GetRandomSpawnPoint();

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AGIEnemyCharacter* Enemy = GetWorld()->SpawnActor<AGIEnemyCharacter>(
			EnemyClass,
			SpawnLoc,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (Enemy)
		{
			ActiveEnemies.Add(Enemy);

			// Add dynamic patrol component with waypoints around spawn → player area
			UGIPatrolComponent* Patrol = NewObject<UGIPatrolComponent>(Enemy);
			if (Patrol)
			{
				Patrol->RegisterComponent();

				// Generate patrol waypoints: start at spawn, patrol toward player area
				TArray<FVector> Waypoints;
				
				// Waypoint 1: Near spawn
				Waypoints.Add(SpawnLoc);

				// Waypoint 2-3: Intermediate points toward player
				const FVector DirectionToPlayer = (PlayerLocation - SpawnLoc).GetSafeNormal2D();
				const float TotalDist = FVector::Dist2D(SpawnLoc, PlayerLocation);
				
				for (int32 w = 1; w <= 2; w++)
				{
					const float Fraction = 0.3f * w;
					FVector WpLoc = SpawnLoc + DirectionToPlayer * TotalDist * Fraction;
					
					// Add lateral offset for non-linear patrol
					const float LateralOffset = FMath::RandRange(-500.0f, 500.0f);
					WpLoc += FVector(-DirectionToPlayer.Y, DirectionToPlayer.X, 0.0f) * LateralOffset;

					// Snap to terrain
					FHitResult WpHit;
					FCollisionQueryParams WpParams;
					if (GetWorld()->LineTraceSingleByChannel(WpHit, WpLoc + FVector(0,0,3000), WpLoc - FVector(0,0,3000), ECC_WorldStatic, WpParams))
					{
						Waypoints.Add(WpHit.ImpactPoint + FVector(0,0,90));
					}
					else
					{
						Waypoints.Add(WpLoc);
					}
				}

				// Waypoint 4: Near player area (engagement zone)
				FVector FinalWp = PlayerLocation + FVector(
					FMath::RandRange(-800.0f, 800.0f),
					FMath::RandRange(-800.0f, 800.0f),
					0.0f
				);
				FHitResult FinalHit;
				if (GetWorld()->LineTraceSingleByChannel(FinalHit, FinalWp + FVector(0,0,3000), FinalWp - FVector(0,0,3000), ECC_WorldStatic, FCollisionQueryParams()))
				{
					Waypoints.Add(FinalHit.ImpactPoint + FVector(0,0,90));
				}
				else
				{
					Waypoints.Add(FinalWp);
				}

				Patrol->SetPatrolWaypoints(Waypoints);
			}
		}
	}
}

int32 AGIWaveManager::GetActiveEnemyCount() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<AGIEnemyCharacter>& Ptr : ActiveEnemies)
	{
		if (Ptr.IsValid() && !Ptr->GetIsDead())
		{
			++Count;
		}
	}
	return Count;
}

void AGIWaveManager::CheckWaveCleared()
{
	if (!bWaveActive) return;

	// Remove invalid/dead entries
	ActiveEnemies.RemoveAll([](const TWeakObjectPtr<AGIEnemyCharacter>& Ptr)
	{
		return !Ptr.IsValid() || Ptr->GetIsDead();
	});

	if (ActiveEnemies.Num() == 0)
	{
		OnWaveCleared();
	}
}

void AGIWaveManager::OnWaveCleared()
{
	bWaveActive = false;
	GetWorld()->GetTimerManager().ClearTimer(WaveSpawnTimer);

	QueueNextWave();
}

void AGIWaveManager::QueueNextWave()
{
	const int32 NextWave = CurrentWaveIndex + 1;

	if (Waves.IsValidIndex(NextWave))
	{
		StartWave(NextWave);
	}
	else if (bLoopWaves)
	{
		StartWave(0);
	}
	else
	{
		// All waves cleared — mission complete
		bAllWavesComplete = true;
		OnAllWavesCleared.Broadcast();
		if (CachedGameMode)
		{
			CachedGameMode->NotifyAllWavesCleared();
		}
	}
}

FVector AGIWaveManager::GetRandomSpawnPoint() const
{
	if (SpawnPoints.Num() == 0)
	{
		return FVector::ZeroVector;
	}

	const int32 Index = FMath::RandRange(0, SpawnPoints.Num() - 1);
	return SpawnPoints[Index];
}

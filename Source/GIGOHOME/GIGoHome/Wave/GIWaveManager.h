// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GIWaveManager.generated.h"

class AGIEnemyCharacter;
class AGIGoHomeSliceGameMode;

USTRUCT(BlueprintType)
struct FGIWaveConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Wave")
	int32 GreenCount = 2;

	UPROPERTY(EditAnywhere, Category="Wave")
	int32 VeteranCount = 0;

	UPROPERTY(EditAnywhere, Category="Wave")
	int32 BrokenCount = 0;

	UPROPERTY(EditAnywhere, Category="Wave")
	int32 OfficerCount = 0;

	UPROPERTY(EditAnywhere, Category="Wave")
	float DelayBeforeSpawn = 5.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStartedDelegate, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllWavesClearedDelegate);

/**
 * Manages enemy wave spawning for the vertical slice
 * Spawns escalating waves of enemies, tracks kills, notifies GameMode
 * 
 * USAGE:
 * - Place in level OR let SliceGameMode auto-spawn
 * - Assign enemy archetype classes (or let them default to C++ classes)
 * - Tag actors with "EnemySpawn" to define spawn points
 */
UCLASS()
class GIGOHOME_API AGIWaveManager : public AActor
{
	GENERATED_BODY()

public:

	/** Fired when a new wave starts (0-indexed) */
	UPROPERTY(BlueprintAssignable, Category="Waves")
	FOnWaveStartedDelegate OnWaveStarted;

	/** Fired when all waves have been cleared */
	UPROPERTY(BlueprintAssignable, Category="Waves")
	FOnAllWavesClearedDelegate OnAllWavesCleared;

	AGIWaveManager();

protected:

	virtual void BeginPlay() override;

public:

	/** Tag used to find spawn point actors in the level */
	UPROPERTY(EditAnywhere, Category="Spawning")
	FName SpawnPointTag = FName("EnemySpawn");

	/** Enemy archetype classes - auto-assigned to C++ classes if not set */
	UPROPERTY(EditAnywhere, Category="Enemies")
	TSubclassOf<AGIEnemyCharacter> GreenClass;

	UPROPERTY(EditAnywhere, Category="Enemies")
	TSubclassOf<AGIEnemyCharacter> VeteranClass;

	UPROPERTY(EditAnywhere, Category="Enemies")
	TSubclassOf<AGIEnemyCharacter> BrokenClass;

	UPROPERTY(EditAnywhere, Category="Enemies")
	TSubclassOf<AGIEnemyCharacter> OfficerClass;

	/** Wave configurations - if empty, defaults are generated */
	UPROPERTY(EditAnywhere, Category="Waves")
	TArray<FGIWaveConfig> Waves;

	/** If true, waves loop after the final wave */
	UPROPERTY(EditAnywhere, Category="Waves")
	bool bLoopWaves = false;

	/** If true, auto-start first wave in BeginPlay. If false, call StartFirstWave() manually */
	UPROPERTY(EditAnywhere, Category="Waves")
	bool bAutoStart = true;

	/** Manually start the first wave (used when auto-start is disabled) */
	UFUNCTION(BlueprintCallable, Category="Waves")
	void StartFirstWave();

	/** Returns current wave index (0-indexed) */
	UFUNCTION(BlueprintPure, Category="Waves")
	int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }

	/** Returns total number of waves */
	UFUNCTION(BlueprintPure, Category="Waves")
	int32 GetTotalWaveCount() const { return Waves.Num(); }

	/** Returns number of enemies still alive in current wave */
	UFUNCTION(BlueprintPure, Category="Waves")
	int32 GetActiveEnemyCount() const;

	/** Returns true if waves have started */
	UFUNCTION(BlueprintPure, Category="Waves")
	bool HasStarted() const { return bHasStarted; }

private:

	TArray<TWeakObjectPtr<AGIEnemyCharacter>> ActiveEnemies;
	TArray<FVector> SpawnPoints;
	int32 CurrentWaveIndex = 0;
	bool bWaveActive = false;
	bool bAllWavesComplete = false;
	bool bHasStarted = false;
	FTimerHandle WaveSpawnTimer;
	AGIGoHomeSliceGameMode* CachedGameMode = nullptr;

	void CacheGameMode();
	void BuildSpawnPoints();
	void SetupDefaultWaves();
	void EnsureEnemyClasses();
	void StartWave(int32 WaveIndex);
	void SpawnWaveEnemies(const FGIWaveConfig& Config);
	void SpawnEnemy(TSubclassOf<AGIEnemyCharacter> EnemyClass, int32 Count);
	void CheckWaveCleared();
	void OnWaveCleared();
	void QueueNextWave();
	FVector GetRandomSpawnPoint() const;
};

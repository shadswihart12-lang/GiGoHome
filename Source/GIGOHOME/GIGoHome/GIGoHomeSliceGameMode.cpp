// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Vertical Slice Game Mode
// PRODUCTION-READY: Complete auto-bootstrap implementation

#include "GIGoHome/GIGoHomeSliceGameMode.h"
#include "GIGoHome/GISlateHUD.h"
#include "GIGoHome/Wave/GIWaveManager.h"
#include "GIGoHome/AI/GIEnemyCharacter.h"
#include "GIGoHome/AI/GIEnemyArchetypes.h"
#include "GIGoHome/Player/GIXiDongConcrete.h"
#include "GIGoHome/Player/GIAct1PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

AGIGoHomeSliceGameMode::AGIGoHomeSliceGameMode()
{
	// Pure C++ - no Blueprint dependencies
	DefaultPawnClass = AGIXiDongConcrete::StaticClass();
   PlayerControllerClass = AGIAct1PlayerController::StaticClass();
}

void AGIGoHomeSliceGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Setup default objectives if none configured
	if (MissionObjectives.Num() == 0)
	{
		SetupDefaultObjectives();
	}

	// Create pure Slate HUD (no Blueprint required)
	CreateSlateHUD();

	// Ensure WaveManager exists (auto-spawn if missing)
	EnsureWaveManager();

	// Initial wave notification (internal tracking only)
	OnWaveChanged.Broadcast(1, TotalWaves);
}

void AGIGoHomeSliceGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up Slate HUD
	if (SlateHUD)
	{
		SlateHUD->DestroyHUD();
		SlateHUD = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AGIGoHomeSliceGameMode::CreateSlateHUD()
{
	SlateHUD = NewObject<UGISlateHUDManager>(this);
	if (SlateHUD)
	{
		SlateHUD->CreateHUD(GetWorld());
	}
}

void AGIGoHomeSliceGameMode::EnsureWaveManager()
{
	// First, look for existing WaveManager placed in level
	for (TActorIterator<AGIWaveManager> It(GetWorld()); It; ++It)
	{
		WaveManager = *It;
		
		// Bind events to existing manager
		WaveManager->OnWaveStarted.AddDynamic(this, &AGIGoHomeSliceGameMode::NotifyWaveStarted);
		WaveManager->OnAllWavesCleared.AddDynamic(this, &AGIGoHomeSliceGameMode::NotifyAllWavesCleared);
		
		// Ensure it has default classes if not set
		if (!WaveManager->GreenClass)
		{
			WaveManager->GreenClass = AGIEnemyGreen::StaticClass();
		}
		if (!WaveManager->VeteranClass)
		{
			WaveManager->VeteranClass = AGIEnemyVeteran::StaticClass();
		}
		if (!WaveManager->BrokenClass)
		{
			WaveManager->BrokenClass = AGIEnemyBroken::StaticClass();
		}
		if (!WaveManager->OfficerClass)
		{
			WaveManager->OfficerClass = AGIEnemyOfficer::StaticClass();
		}
		
		// If it hasn't started yet (manual start), start it now
		if (!WaveManager->HasStarted())
		{
			WaveManager->StartFirstWave();
		}
		
		return; // Found existing manager, done
	}

	// No WaveManager exists - spawn one using deferred spawn to configure before BeginPlay
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Use SpawnActorDeferred so we can set properties BEFORE BeginPlay runs
	WaveManager = GetWorld()->SpawnActorDeferred<AGIWaveManager>(
		AGIWaveManager::StaticClass(),
		FTransform::Identity,
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	if (WaveManager)
	{
		// Configure BEFORE BeginPlay runs
		WaveManager->bAutoStart = false; // We'll start manually after binding events
		WaveManager->GreenClass = AGIEnemyGreen::StaticClass();
		WaveManager->VeteranClass = AGIEnemyVeteran::StaticClass();
		WaveManager->BrokenClass = AGIEnemyBroken::StaticClass();
		WaveManager->OfficerClass = AGIEnemyOfficer::StaticClass();

		// Finish spawning - this triggers BeginPlay
		WaveManager->FinishSpawning(FTransform::Identity);

		// NOW bind events (after BeginPlay has set up waves)
		WaveManager->OnWaveStarted.AddDynamic(this, &AGIGoHomeSliceGameMode::NotifyWaveStarted);
		WaveManager->OnAllWavesCleared.AddDynamic(this, &AGIGoHomeSliceGameMode::NotifyAllWavesCleared);

		// NOW start the first wave
		WaveManager->StartFirstWave();
	}
}

void AGIGoHomeSliceGameMode::OnPlayerDeath()
{
	if (bMissionComplete) return;

	Super::OnPlayerDeath();

	// Show death screen via Slate HUD
	if (SlateHUD)
	{
		SlateHUD->ShowDeathScreen();
	}

	OnMissionFailed.Broadcast();
}

void AGIGoHomeSliceGameMode::SetupDefaultObjectives()
{
	// Per GDD: No HUD objectives displayed - these are internal tracking only
	// The player figures it out, like Xi Dong does

	// Primary: Survive all waves
	FMissionObjectiveData SurviveObj;
	SurviveObj.Type = EMissionObjective::SurviveAllWaves;
	SurviveObj.Description = FText::FromString(TEXT("Survive"));
	SurviveObj.RequiredProgress = TotalWaves;
	MissionObjectives.Add(SurviveObj);

	// Secondary: Eliminate officers (causes squad cohesion loss per GDD)
	FMissionObjectiveData OfficerObj;
	OfficerObj.Type = EMissionObjective::EliminateOfficer;
	OfficerObj.Description = FText::FromString(TEXT("Eliminate officers"));
	OfficerObj.RequiredProgress = 2;
	MissionObjectives.Add(OfficerObj);

	// Optional: Set traps (punji pits, tripwire grenades per GDD)
	FMissionObjectiveData TrapObj;
	TrapObj.Type = EMissionObjective::PlaceTraps;
	TrapObj.Description = FText::FromString(TEXT("Set traps"));
	TrapObj.RequiredProgress = 3;
	MissionObjectives.Add(TrapObj);
}

void AGIGoHomeSliceGameMode::NotifyWaveStarted(int32 WaveIndex)
{
	CurrentWave = WaveIndex + 1;
	OnWaveChanged.Broadcast(CurrentWave, TotalWaves);
}

void AGIGoHomeSliceGameMode::NotifyAllWavesCleared()
{
	// Mark survive objective as completed
	for (FMissionObjectiveData& Obj : MissionObjectives)
	{
		if (Obj.Type == EMissionObjective::SurviveAllWaves)
		{
			Obj.CurrentProgress = Obj.RequiredProgress;
			Obj.bCompleted = true;
			OnObjectiveUpdated.Broadcast(Obj);
			break;
		}
	}

	CheckMissionComplete();
}

void AGIGoHomeSliceGameMode::NotifyOfficerEliminated()
{
	UpdateObjective(EMissionObjective::EliminateOfficer, 1);
}

void AGIGoHomeSliceGameMode::NotifyRadioDestroyed()
{
	UpdateObjective(EMissionObjective::DestroyRadio, 1);
}

void AGIGoHomeSliceGameMode::NotifyTrapPlaced()
{
	UpdateObjective(EMissionObjective::PlaceTraps, 1);
}

void AGIGoHomeSliceGameMode::ShowAmmoCheck(int32 CurrentAmmo, int32 MagazineSize)
{
	if (SlateHUD)
	{
		SlateHUD->ShowAmmoCheck(CurrentAmmo, MagazineSize);
	}
}

void AGIGoHomeSliceGameMode::UpdateObjective(EMissionObjective Type, int32 ProgressDelta)
{
	for (FMissionObjectiveData& Obj : MissionObjectives)
	{
		if (Obj.Type == Type && !Obj.bCompleted)
		{
			Obj.CurrentProgress = FMath::Clamp(Obj.CurrentProgress + ProgressDelta, 0, Obj.RequiredProgress);
			if (Obj.CurrentProgress >= Obj.RequiredProgress)
			{
				Obj.bCompleted = true;
			}
			OnObjectiveUpdated.Broadcast(Obj);
			break;
		}
	}
}

void AGIGoHomeSliceGameMode::CheckMissionComplete()
{
	bool bPrimaryComplete = false;
	for (const FMissionObjectiveData& Obj : MissionObjectives)
	{
		if (Obj.Type == EMissionObjective::SurviveAllWaves)
		{
			bPrimaryComplete = Obj.bCompleted;
			break;
		}
	}

	if (bPrimaryComplete && !bMissionComplete)
	{
		bMissionComplete = true;
		
		// Show victory screen
		if (SlateHUD)
		{
			SlateHUD->ShowVictoryScreen();
		}

		OnMissionComplete.Broadcast();
		CompleteMission();
	}
}

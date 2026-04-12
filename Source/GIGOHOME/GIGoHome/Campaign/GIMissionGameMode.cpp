// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Base Campaign Mission Game Mode Implementation

#include "GIGoHome/Campaign/GIMissionGameMode.h"
#include "GIGoHome/Campaign/GINarrativeDirector.h"
#include "GIGoHome/Campaign/Actors/GICampaignNPC.h"
#include "GIGoHome/GIGoHomeCampaignState.h"
#include "GIGoHome/GISlateHUD.h"
#include "GIGoHome/Player/GIXiDongConcrete.h"
#include "GIGoHome/Player/GIAct1PlayerController.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "TimerManager.h"

AGIMissionGameMode::AGIMissionGameMode()
{
	DefaultPawnClass = AGIXiDongConcrete::StaticClass();
   PlayerControllerClass = AGIAct1PlayerController::StaticClass();
}

void AGIMissionGameMode::BeginPlay()
{
	Super::BeginPlay();

	CreateNarrativeDirector();
	CreateMissionHUD();
	SetupMission();
	ApplyPriorChoiceConsequences();
	StartMission();
}

void AGIMissionGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopMissionTimer();
   StopDemoSessionTimer();
	CleanupMissionNPCs();

	if (SlateHUD)
	{
		SlateHUD->DestroyHUD();
		SlateHUD = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AGIMissionGameMode::OnPlayerDeath()
{
	if (bMissionComplete || bMissionFailed) return;

	Super::OnPlayerDeath();

	bMissionFailed = true;
	bMissionActive = false;
	StopMissionTimer();
	StopDemoSessionTimer();
	LogTelemetryEvent(TEXT("PlayerDeath"));

	if (AGIGoHomeCampaignState* CS = GetCampaignState())
	{
		const float MissionDuration = GetWorld() ? static_cast<float>(GetWorld()->GetTimeSeconds() - MissionStartTimeSeconds) : 0.0f;
		CS->RecordMissionEnded(MissionID, false, MissionDuration);
	}

	if (SlateHUD)
	{
		SlateHUD->ShowDeathScreen();
	}

	OnMissionEnded.Broadcast(false);
}

// ============================================================================
// MISSION FLOW
// ============================================================================

void AGIMissionGameMode::StartMission()
{
	if (bMissionActive) return;

	bMissionActive = true;
	bMissionComplete = false;
	bMissionFailed = false;

	SpawnMissionNPCs();

	if (bUseMissionTimer)
	{
		MissionTimerRemaining = MissionTimerDuration;
	}

	MissionStartTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	StartDemoSessionTimer();
	LogTelemetryEvent(TEXT("MissionStarted"));

	if (AGIGoHomeCampaignState* CS = GetCampaignState())
	{
		CS->RecordMissionStarted(MissionID);
	}

	OnMissionStarted.Broadcast();
	OnMissionBegin();
}

void AGIMissionGameMode::EndMission(bool bSuccess)
{
	if (!bMissionActive) return;

	bMissionActive = false;
	bMissionComplete = bSuccess;
	StopMissionTimer();
	StopDemoSessionTimer();

	OnMissionEnd(bSuccess);

	if (bSuccess)
	{
		// Record to campaign state
		CompleteMission();

		if (SlateHUD)
		{
			SlateHUD->ShowVictoryScreen();
		}
	}

	const float MissionDuration = GetWorld() ? static_cast<float>(GetWorld()->GetTimeSeconds() - MissionStartTimeSeconds) : 0.0f;
	LogTelemetryEvent(bSuccess ? TEXT("MissionCompleted") : TEXT("MissionFailed"));

	if (AGIGoHomeCampaignState* CS = GetCampaignState())
	{
		CS->RecordMissionEnded(MissionID, bSuccess, MissionDuration);
	}

	OnMissionEnded.Broadcast(bSuccess);
}

void AGIMissionGameMode::RestartCurrentMission()
{
	LogTelemetryEvent(TEXT("MissionRestartRequested"));

	if (AGIGoHomeCampaignState* CS = GetCampaignState())
	{
		CS->RecordMissionRestarted(MissionID);
	}

	if (UWorld* World = GetWorld())
	{
		const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(World, true);
		if (!CurrentLevelName.IsEmpty())
		{
			UGameplayStatics::OpenLevel(World, FName(*CurrentLevelName));
		}
	}
}

void AGIMissionGameMode::DemoRestartMission()
{
	RestartCurrentMission();
}

// ============================================================================
// OBJECTIVES
// ============================================================================

void AGIMissionGameMode::UpdateObjectiveProgress(FName ObjectiveTag, int32 ProgressDelta)
{
	FGIMissionObjective* Obj = FindObjective(ObjectiveTag);
	if (!Obj || Obj->bCompleted) return;

	Obj->CurrentProgress = FMath::Clamp(
		Obj->CurrentProgress + ProgressDelta, 0, Obj->RequiredProgress);

	if (Obj->CurrentProgress >= Obj->RequiredProgress)
	{
		Obj->bCompleted = true;
		OnObjectiveCompleted.Broadcast(*Obj);
		CheckMissionCompletion();
	}
	else
	{
		OnObjectiveUpdated.Broadcast(*Obj);
	}
}

void AGIMissionGameMode::CompleteObjective(FName ObjectiveTag)
{
	FGIMissionObjective* Obj = FindObjective(ObjectiveTag);
	if (!Obj || Obj->bCompleted) return;

	Obj->CurrentProgress = Obj->RequiredProgress;
	Obj->bCompleted = true;
	OnObjectiveCompleted.Broadcast(*Obj);
	CheckMissionCompletion();
}

void AGIMissionGameMode::FailMission()
{
	if (bMissionFailed || bMissionComplete) return;

	bMissionFailed = true;
	bMissionActive = false;
	StopMissionTimer();
    StopDemoSessionTimer();
	LogTelemetryEvent(TEXT("FailMission"));

	if (AGIGoHomeCampaignState* CS = GetCampaignState())
	{
		const float MissionDuration = GetWorld() ? static_cast<float>(GetWorld()->GetTimeSeconds() - MissionStartTimeSeconds) : 0.0f;
		CS->RecordMissionEnded(MissionID, false, MissionDuration);
	}
	OnMissionEnded.Broadcast(false);
}

bool AGIMissionGameMode::AreRequiredObjectivesComplete() const
{
	for (const FGIMissionObjective& Obj : Objectives)
	{
		if (!Obj.bOptional && !Obj.bCompleted)
		{
			return false;
		}
	}
	return true;
}

FGIMissionObjective* AGIMissionGameMode::FindObjective(FName ObjectiveTag)
{
	for (FGIMissionObjective& Obj : Objectives)
	{
		if (Obj.ObjectiveTag == ObjectiveTag)
		{
			return &Obj;
		}
	}
	return nullptr;
}

bool AGIMissionGameMode::IsObjectiveCompleted(FName ObjectiveTag) const
{
	for (const FGIMissionObjective& Obj : Objectives)
	{
		if (Obj.ObjectiveTag == ObjectiveTag)
		{
			return Obj.bCompleted;
		}
	}

	return false;
}

void AGIMissionGameMode::CheckMissionCompletion()
{
	if (AreRequiredObjectivesComplete() && bMissionActive)
	{
		EndMission(true);
	}
}

// ============================================================================
// CHOICES
// ============================================================================

void AGIMissionGameMode::MakeChoice(FName ChoiceTag, EAct1Choice ChosenOption)
{
  EPlayerChoice CampaignChoice = EPlayerChoice::None;
	switch (ChosenOption)
	{
	case EAct1Choice::M1_KilledSleeping: CampaignChoice = EPlayerChoice::M1_KilledSleeping; break;
	case EAct1Choice::M1_WaitedForDawn: CampaignChoice = EPlayerChoice::M1_WaitedForDawn; break;
	case EAct1Choice::M2_AttackedListeningPost: CampaignChoice = EPlayerChoice::M2_AttackedListeningPost; break;
	case EAct1Choice::M2_CompletedObservation: CampaignChoice = EPlayerChoice::M2_CompletedObservation; break;
	case EAct1Choice::M3_ChasedRunner: CampaignChoice = EPlayerChoice::M3_ChasedRunner; break;
	case EAct1Choice::M3_DestroyedBackupRadio: CampaignChoice = EPlayerChoice::M3_DestroyedBackupRadio; break;
	case EAct1Choice::M4_KilledTylerGreg: CampaignChoice = EPlayerChoice::M4_KilledTylerGreg; break;
	case EAct1Choice::M4_SparedTylerGreg: CampaignChoice = EPlayerChoice::M4_SparedTylerGreg; break;
	case EAct1Choice::M5_FirebaseComplete: CampaignChoice = EPlayerChoice::M5_FirebaseComplete; break;
	default: break;
	}

	for (FMissionChoice& Choice : Choices)
	{
		if (Choice.ChoiceTag == ChoiceTag && !Choice.bPresented)
		{
			Choice.bPresented = true;
			Choice.ChosenOption = ChosenOption;

			// Record to campaign state
			if (AGIGoHomeCampaignState* CS = GetCampaignState())
			{
                if (CampaignChoice != EPlayerChoice::None)
				{
					CS->RecordChoice(CampaignChoice);
				}
			}

			OnChoiceMade.Broadcast(ChosenOption);
			return;
		}
	}
}

bool AGIMissionGameMode::HasChoiceBeenMade(FName ChoiceTag) const
{
	for (const FMissionChoice& Choice : Choices)
	{
		if (Choice.ChoiceTag == ChoiceTag)
		{
			return Choice.bPresented;
		}
	}
	return false;
}

EAct1Choice AGIMissionGameMode::GetChoiceResult(FName ChoiceTag) const
{
	for (const FMissionChoice& Choice : Choices)
	{
		if (Choice.ChoiceTag == ChoiceTag)
		{
			return Choice.ChosenOption;
		}
	}
	return EAct1Choice::None;
}

// ============================================================================
// TIMER
// ============================================================================

void AGIMissionGameMode::StartMissionTimer()
{
	if (!bUseMissionTimer) return;

	MissionTimerRemaining = MissionTimerDuration;

	GetWorldTimerManager().SetTimer(
		MissionTimerHandle,
		this,
		&AGIMissionGameMode::TickMissionTimer,
		1.0f,
		true
	);
}

void AGIMissionGameMode::StopMissionTimer()
{
	GetWorldTimerManager().ClearTimer(MissionTimerHandle);
}

void AGIMissionGameMode::TickMissionTimer()
{
	MissionTimerRemaining -= 1.0f;
	OnTimerTick.Broadcast(MissionTimerRemaining);

	if (MissionTimerRemaining <= 0.0f)
	{
		MissionTimerRemaining = 0.0f;
		StopMissionTimer();
		OnMissionTimerExpired();
	}
}

void AGIMissionGameMode::OnMissionTimerExpired()
{
	// Default: mission success when timer expires (survived the duration)
	CompleteObjective(FName("SurviveTimer"));
}

// ============================================================================
// NARRATIVE
// ============================================================================

void AGIMissionGameMode::PlayNarrativeMoment(FName MomentTag)
{
	if (!NarrativeDirector) return;

	for (const FNarrativeMoment& Moment : NarrativeMoments)
	{
		if (Moment.MomentTag == MomentTag)
		{
			NarrativeDirector->PlayMoment(Moment);
			return;
		}
	}
}

void AGIMissionGameMode::FadeToBlack(float Duration)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, Duration, FLinearColor::Black, false, true);
	}
}

void AGIMissionGameMode::FadeFromBlack(float Duration)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, Duration, FLinearColor::Black, false, false);
	}
}

void AGIMissionGameMode::ShowAmmoCheck(int32 CurrentAmmo, int32 MagazineSize)
{
	if (SlateHUD)
	{
		SlateHUD->ShowAmmoCheck(CurrentAmmo, MagazineSize);
	}
}

// ============================================================================
// NPC MANAGEMENT
// ============================================================================

AGICampaignNPC* AGIMissionGameMode::GetNPCByTag(FName NPCTag) const
{
 AGICampaignNPC* const* Found = SpawnedNPCs.Find(NPCTag);
	return Found ? *Found : nullptr;
}

AGICampaignNPC* AGIMissionGameMode::ResolveMissionNPC(FName NPCTag) const
{
	if (AGICampaignNPC* CachedNPC = GetNPCByTag(NPCTag))
	{
		return CachedNPC;
	}

	if (AActor* TaggedActor = FindWorldActorByTag(NPCTag, AGICampaignNPC::StaticClass()))
	{
		return Cast<AGICampaignNPC>(TaggedActor);
	}

	return nullptr;
}

AActor* AGIMissionGameMode::FindWorldActorByTag(FName Tag, TSubclassOf<AActor> RequiredClass) const
{
	if (Tag.IsNone() || !GetWorld())
	{
		return nullptr;
	}

	TArray<AActor*> TaggedActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag, TaggedActors);

	if (!RequiredClass)
	{
		return TaggedActors.Num() > 0 ? TaggedActors[0] : nullptr;
	}

	for (AActor* Actor : TaggedActors)
	{
		if (IsValid(Actor) && Actor->IsA(RequiredClass))
		{
			return Actor;
		}
	}

	return nullptr;
}

void AGIMissionGameMode::SpawnMissionNPCs()
{
	UWorld* World = GetWorld();
	if (!World) return;

	for (const FMissionNPCSpawn& Spawn : NPCSpawns)
	{
		if (!Spawn.NPCClass) continue;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AGICampaignNPC* NPC = World->SpawnActor<AGICampaignNPC>(
			Spawn.NPCClass, Spawn.SpawnTransform, SpawnParams);

		if (NPC)
		{
			NPC->SetNPCState(Spawn.InitialState);
			NPC->Tags.Add(Spawn.NPCTag);
			SpawnedNPCs.Add(Spawn.NPCTag, NPC);
		}
	}
}

void AGIMissionGameMode::CleanupMissionNPCs()
{
	for (auto& Pair : SpawnedNPCs)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->Destroy();
		}
	}
	SpawnedNPCs.Empty();
}

// ============================================================================
// SETUP
// ============================================================================

void AGIMissionGameMode::SetupMission()
{
	// Subclasses override to populate Objectives, Choices, NPCSpawns, NarrativeMoments
}

void AGIMissionGameMode::CreateNarrativeDirector()
{
	NarrativeDirector = NewObject<UGINarrativeDirector>(this);
	if (NarrativeDirector)
	{
		NarrativeDirector->Initialize(GetWorld());
	}
}

void AGIMissionGameMode::CreateMissionHUD()
{
	SlateHUD = NewObject<UGISlateHUDManager>(this);
	if (SlateHUD)
	{
		SlateHUD->CreateHUD(GetWorld());
	}
}

void AGIMissionGameMode::ApplyPriorChoiceConsequences()
{
	// Subclasses override to check campaign state and adjust difficulty/spawns
}

void AGIMissionGameMode::StartDemoSessionTimer()
{
	if (!bEnablePublisherDemoMode || DemoSessionTimeLimitSeconds <= 0.0f)
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		DemoSessionTimerHandle,
		this,
		&AGIMissionGameMode::OnDemoSessionTimeout,
		DemoSessionTimeLimitSeconds,
		false
	);
}

void AGIMissionGameMode::StopDemoSessionTimer()
{
	GetWorldTimerManager().ClearTimer(DemoSessionTimerHandle);
}

void AGIMissionGameMode::OnDemoSessionTimeout()
{
	if (!bMissionActive)
	{
		return;
	}

	LogTelemetryEvent(TEXT("DemoSessionTimeout"));
	FailMission();
}

void AGIMissionGameMode::LogTelemetryEvent(const FString& EventName) const
{
	UE_LOG(LogTemp, Display, TEXT("[PublisherTelemetry] Mission=%d Event=%s Time=%.2f"),
		static_cast<int32>(MissionID),
		*EventName,
		GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f);
}

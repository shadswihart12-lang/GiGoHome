// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Act 1 Mission Game Modes Implementation

#include "GIGoHome/Campaign/GIAct1Missions.h"
#include "GIGoHome/Campaign/Actors/GIRadioActor.h"
#include "GIGoHome/Campaign/Actors/GICampaignNPC.h"
#include "GIGoHome/Campaign/Actors/GIObservationPoint.h"
#include "GIGoHome/Campaign/Components/GIEscortComponent.h"
#include "GIGoHome/AI/GIEnemyArchetypes.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

namespace GIAct1MissionHelpers
{
	static APawn* GetPlayerPawn(UWorld* World)
	{
		return World ? UGameplayStatics::GetPlayerPawn(World, 0) : nullptr;
	}

	static bool IsPlayerNearActor(UWorld* World, const AActor* Target, float Distance)
	{
		const APawn* PlayerPawn = GetPlayerPawn(World);
		if (!PlayerPawn || !Target)
		{
			return false;
		}

		return FVector::DistSquared(PlayerPawn->GetActorLocation(), Target->GetActorLocation()) <= FMath::Square(Distance);
	}
}

// ============================================================================
// MISSION 1
// ============================================================================

void AGIAct1Mission1GameMode::SetupMission()
{
	MissionID = ECampaignMission::Act1_Mission1_TheProving;

	Objectives.Empty();
	Choices.Empty();

	FGIMissionObjective NeutralizeOperator;
	NeutralizeOperator.Type = EGIMissionObjectiveType::Eliminate;
	NeutralizeOperator.Description = FText::FromString(TEXT("Neutralize radio operator"));
	NeutralizeOperator.ObjectiveTag = FName("M1_NeutralizeOperator");
	Objectives.Add(NeutralizeOperator);

	FGIMissionObjective DestroyRadio;
	DestroyRadio.Type = EGIMissionObjectiveType::Destroy;
	DestroyRadio.Description = FText::FromString(TEXT("Destroy radio"));
	DestroyRadio.ObjectiveTag = FName("M1_DestroyRadio");
	Objectives.Add(DestroyRadio);

	FGIMissionObjective ReturnToCell;
	ReturnToCell.Type = EGIMissionObjectiveType::Extract;
	ReturnToCell.Description = FText::FromString(TEXT("Return to cell"));
	ReturnToCell.ObjectiveTag = FName("M1_Extract");
	Objectives.Add(ReturnToCell);

	FMissionChoice DawnChoice;
	DawnChoice.ChoiceTag = FName("M1_DawnChoice");
	DawnChoice.ChoiceA = EAct1Choice::M1_KilledSleeping;
	DawnChoice.ChoiceB = EAct1Choice::M1_WaitedForDawn;
	Choices.Add(DawnChoice);
}

void AGIAct1Mission1GameMode::OnMissionBegin()
{
	RadioOperator = ResolveMissionNPC(RadioOperatorTag);
	MissionRadio = Cast<AGIRadioActor>(FindWorldActorByTag(RadioActorTag, AGIRadioActor::StaticClass()));

	if (RadioOperator)
	{
		RadioOperator->OnNPCKilled.AddDynamic(this, &AGIAct1Mission1GameMode::OnRadioOperatorKilled);
	}

	if (MissionRadio)
	{
		MissionRadio->OnRadioDestroyed.AddDynamic(this, &AGIAct1Mission1GameMode::OnMissionRadioDestroyed);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Mission1: Radio actor with tag '%s' not found."), *RadioActorTag.ToString());
		CompleteObjective(FName("M1_DestroyRadio"));
	}

	if (!RadioOperator)
	{
		UE_LOG(LogTemp, Warning, TEXT("Mission1: Radio operator with tag '%s' not found."), *RadioOperatorTag.ToString());
		CompleteObjective(FName("M1_NeutralizeOperator"));
	}

	// Test-map fallback: guarantee at least one combat contact for playtest validation.
	if (!RadioOperator && !MissionRadio)
	{
		if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
		{
			const FVector SpawnLocation = PlayerPawn->GetActorLocation() + (PlayerPawn->GetActorForwardVector() * 900.0f);
			const FRotator SpawnRotation = PlayerPawn->GetActorRotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			GetWorld()->SpawnActor<AGIEnemyGreen>(AGIEnemyGreen::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
		}
	}

	GetWorldTimerManager().SetTimer(ExtractionCheckTimer, this, &AGIAct1Mission1GameMode::CheckExtraction, 0.25f, true);
}

void AGIAct1Mission1GameMode::OnMissionEnd(bool bSuccess)
{
	GetWorldTimerManager().ClearTimer(ExtractionCheckTimer);
}

void AGIAct1Mission1GameMode::OnMissionRadioDestroyed(AGIRadioActor* DestroyedRadio)
{
	CompleteObjective(FName("M1_DestroyRadio"));
}

void AGIAct1Mission1GameMode::OnRadioOperatorKilled(AGICampaignNPC* DeadNPC)
{
	CompleteObjective(FName("M1_NeutralizeOperator"));

	if (!HasChoiceBeenMade(FName("M1_DawnChoice")))
	{
		MakeChoice(FName("M1_DawnChoice"), EAct1Choice::M1_KilledSleeping);
	}
}

void AGIAct1Mission1GameMode::CheckExtraction()
{
	const FGIMissionObjective* NeutralizeObj = FindObjective(FName("M1_NeutralizeOperator"));
	const FGIMissionObjective* DestroyObj = FindObjective(FName("M1_DestroyRadio"));
	if (!NeutralizeObj || !DestroyObj || !NeutralizeObj->bCompleted || !DestroyObj->bCompleted)
	{
		return;
	}

	if (GIAct1MissionHelpers::IsPlayerNearActor(GetWorld(), FindWorldActorByTag(ExtractionTag), ExtractionDistance))
	{
		CompleteObjective(FName("M1_Extract"));
	}
}

// ============================================================================
// MISSION 2
// ============================================================================

void AGIAct1Mission2GameMode::SetupMission()
{
	MissionID = ECampaignMission::Act1_Mission2_TheObservation;

	Objectives.Empty();
	Choices.Empty();

	for (int32 Index = 1; Index <= 3; ++Index)
	{
		FGIMissionObjective ObservePoint;
		ObservePoint.Type = EGIMissionObjectiveType::Observe;
		ObservePoint.Description = FText::FromString(FString::Printf(TEXT("Observe point %d"), Index));
		ObservePoint.ObjectiveTag = FName(*FString::Printf(TEXT("M2_Observe%d"), Index));
		Objectives.Add(ObservePoint);
	}

	FGIMissionObjective IdentifyRadio;
	IdentifyRadio.Type = EGIMissionObjectiveType::Observe;
	IdentifyRadio.Description = FText::FromString(TEXT("Identify radio equipment"));
	IdentifyRadio.ObjectiveTag = FName("M2_IdentifyRadio");
	Objectives.Add(IdentifyRadio);

	FGIMissionObjective Extract;
	Extract.Type = EGIMissionObjectiveType::Extract;
	Extract.Description = FText::FromString(TEXT("Extract undetected"));
	Extract.ObjectiveTag = FName("M2_Extract");
	Objectives.Add(Extract);

	FMissionChoice AssaultChoice;
	AssaultChoice.ChoiceTag = FName("M2_AssaultChoice");
	AssaultChoice.ChoiceA = EAct1Choice::M2_AttackedListeningPost;
	AssaultChoice.ChoiceB = EAct1Choice::M2_CompletedObservation;
	Choices.Add(AssaultChoice);
}

void AGIAct1Mission2GameMode::OnMissionBegin()
{
	TArray<AActor*> ObservationActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGIObservationPoint::StaticClass(), ObservationActors);

	ObservationPoints.Empty();
	for (AActor* Actor : ObservationActors)
	{
		if (AGIObservationPoint* Point = Cast<AGIObservationPoint>(Actor))
		{
			ObservationPoints.Add(Point);
			Point->OnObservationComplete.AddDynamic(this, &AGIAct1Mission2GameMode::OnObservationPointComplete);
		}
	}

	if (ObservationPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Mission2: No observation points found in level. Auto-completing observation objectives."));
		for (int32 Index = 1; Index <= 3; ++Index)
		{
			CompleteObjective(FName(*FString::Printf(TEXT("M2_Observe%d"), Index)));
		}
		CompleteObjective(FName("M2_IdentifyRadio"));

		if (!HasChoiceBeenMade(FName("M2_AssaultChoice")))
		{
			MakeChoice(FName("M2_AssaultChoice"), EAct1Choice::M2_CompletedObservation);
		}
	}

	GetWorldTimerManager().SetTimer(ExtractionCheckTimer, this, &AGIAct1Mission2GameMode::CheckExtraction, 0.25f, true);
}

void AGIAct1Mission2GameMode::OnMissionEnd(bool bSuccess)
{
	GetWorldTimerManager().ClearTimer(ExtractionCheckTimer);
}

void AGIAct1Mission2GameMode::OnObservationPointComplete(AGIObservationPoint* Point)
{
	if (!Point)
	{
		return;
	}

	const FName PointTag = Point->GetPointTag();
	if (!PointTag.IsNone())
	{
		CompleteObjective(PointTag);
	}

	// Once all points are done, radio identification is complete.
	bool bAllObserved = true;
	for (int32 Index = 1; Index <= 3; ++Index)
	{
		const FGIMissionObjective* ObserveObj = FindObjective(FName(*FString::Printf(TEXT("M2_Observe%d"), Index)));
		if (!ObserveObj || !ObserveObj->bCompleted)
		{
			bAllObserved = false;
			break;
		}
	}

	if (bAllObserved)
	{
		CompleteObjective(FName("M2_IdentifyRadio"));

		if (!HasChoiceBeenMade(FName("M2_AssaultChoice")))
		{
			MakeChoice(FName("M2_AssaultChoice"), EAct1Choice::M2_CompletedObservation);
		}
	}
}

void AGIAct1Mission2GameMode::CheckExtraction()
{
	const FGIMissionObjective* IdentifyObj = FindObjective(FName("M2_IdentifyRadio"));
	if (!IdentifyObj || !IdentifyObj->bCompleted)
	{
		return;
	}

	if (GIAct1MissionHelpers::IsPlayerNearActor(GetWorld(), FindWorldActorByTag(ExtractionTag), ExtractionDistance))
	{
		CompleteObjective(FName("M2_Extract"));
	}
}

// ============================================================================
// MISSION 3
// ============================================================================

void AGIAct1Mission3GameMode::SetupMission()
{
	MissionID = ECampaignMission::Act1_Mission3_TheDistraction;

	bUseMissionTimer = true;
	MissionTimerDuration = 12.0f * 60.0f;

	Objectives.Empty();
	Choices.Empty();

	FGIMissionObjective Survive;
	Survive.Type = EGIMissionObjectiveType::SurviveTimer;
	Survive.Description = FText::FromString(TEXT("Hold for 12 minutes"));
	Survive.ObjectiveTag = FName("M3_SurviveTimer");
	Objectives.Add(Survive);

	FGIMissionObjective StopComms;
	StopComms.Type = EGIMissionObjectiveType::PreventAction;
	StopComms.Description = FText::FromString(TEXT("Prevent backup radio call"));
	StopComms.ObjectiveTag = FName("M3_StopBackupRadio");
	Objectives.Add(StopComms);

	FGIMissionObjective Extract;
	Extract.Type = EGIMissionObjectiveType::Extract;
	Extract.Description = FText::FromString(TEXT("Extract on signal"));
	Extract.ObjectiveTag = FName("M3_Extract");
	Objectives.Add(Extract);

	FMissionChoice RunnerChoice;
	RunnerChoice.ChoiceTag = FName("M3_RunnerChoice");
	RunnerChoice.ChoiceA = EAct1Choice::M3_ChasedRunner;
	RunnerChoice.ChoiceB = EAct1Choice::M3_DestroyedBackupRadio;
	Choices.Add(RunnerChoice);
}

void AGIAct1Mission3GameMode::OnMissionBegin()
{
	PrimaryRadio = Cast<AGIRadioActor>(FindWorldActorByTag(PrimaryRadioTag, AGIRadioActor::StaticClass()));
	BackupRadio = Cast<AGIRadioActor>(FindWorldActorByTag(BackupRadioTag, AGIRadioActor::StaticClass()));
	RunnerNPC = ResolveMissionNPC(RunnerTag);

	if (PrimaryRadio)
	{
		PrimaryRadio->OnRadioUsed.AddDynamic(this, &AGIAct1Mission3GameMode::OnPrimaryRadioUsed);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Mission3: Primary radio with tag '%s' not found; call-in failure path disabled."), *PrimaryRadioTag.ToString());
	}

	if (BackupRadio)
	{
		BackupRadio->OnRadioDestroyed.AddDynamic(this, &AGIAct1Mission3GameMode::OnBackupRadioDestroyed);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Mission3: Backup radio with tag '%s' not found; auto-completing anti-comm objective."), *BackupRadioTag.ToString());
		CompleteObjective(FName("M3_StopBackupRadio"));
	}

	if (RunnerNPC)
	{
		RunnerNPC->OnNPCKilled.AddDynamic(this, &AGIAct1Mission3GameMode::OnRunnerKilled);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Mission3: Runner NPC with tag '%s' not found."), *RunnerTag.ToString());
	}

	StartMissionTimer();
	GetWorldTimerManager().SetTimer(ExtractionCheckTimer, this, &AGIAct1Mission3GameMode::CheckExtraction, 0.25f, true);
}

void AGIAct1Mission3GameMode::OnMissionTimerExpired()
{
	CompleteObjective(FName("M3_SurviveTimer"));
}

void AGIAct1Mission3GameMode::OnMissionEnd(bool bSuccess)
{
	GetWorldTimerManager().ClearTimer(ExtractionCheckTimer);
}

void AGIAct1Mission3GameMode::OnPrimaryRadioUsed(AActor* User)
{
	FailMission();
}

void AGIAct1Mission3GameMode::OnBackupRadioDestroyed(AGIRadioActor* DestroyedRadio)
{
	CompleteObjective(FName("M3_StopBackupRadio"));

	if (!HasChoiceBeenMade(FName("M3_RunnerChoice")))
	{
		MakeChoice(FName("M3_RunnerChoice"), EAct1Choice::M3_DestroyedBackupRadio);
	}
}

void AGIAct1Mission3GameMode::OnRunnerKilled(AGICampaignNPC* DeadNPC)
{
	if (!HasChoiceBeenMade(FName("M3_RunnerChoice")))
	{
		MakeChoice(FName("M3_RunnerChoice"), EAct1Choice::M3_ChasedRunner);
	}
}

void AGIAct1Mission3GameMode::CheckExtraction()
{
	const FGIMissionObjective* SurviveObj = FindObjective(FName("M3_SurviveTimer"));
	const FGIMissionObjective* BackupObj = FindObjective(FName("M3_StopBackupRadio"));
	if (!SurviveObj || !BackupObj || !SurviveObj->bCompleted || !BackupObj->bCompleted)
	{
		return;
	}

	if (GIAct1MissionHelpers::IsPlayerNearActor(GetWorld(), FindWorldActorByTag(ExtractionTag), ExtractionDistance))
	{
		CompleteObjective(FName("M3_Extract"));
	}
}

// ============================================================================
// MISSION 4
// ============================================================================

void AGIAct1Mission4GameMode::SetupMission()
{
	MissionID = ECampaignMission::Act1_Mission4_TylerGreg;

	Objectives.Empty();
	Choices.Empty();

	FGIMissionObjective ReachPerimeter;
	ReachPerimeter.Type = EGIMissionObjectiveType::ReachLocation;
	ReachPerimeter.Description = FText::FromString(TEXT("Reach firebase perimeter"));
	ReachPerimeter.ObjectiveTag = FName("M4_ReachPerimeter");
	Objectives.Add(ReachPerimeter);

	FGIMissionObjective ResolveTyler;
	ResolveTyler.Type = EGIMissionObjectiveType::Custom;
	ResolveTyler.Description = FText::FromString(TEXT("Resolve Tyler Greg encounter"));
	ResolveTyler.ObjectiveTag = FName("M4_ResolveTyler");
	Objectives.Add(ResolveTyler);

	FGIMissionObjective Extract;
	Extract.Type = EGIMissionObjectiveType::Extract;
	Extract.Description = FText::FromString(TEXT("Return to cell with intel"));
	Extract.ObjectiveTag = FName("M4_Extract");
	Objectives.Add(Extract);

	FMissionChoice TylerChoice;
	TylerChoice.ChoiceTag = FName("M4_TylerChoice");
	TylerChoice.ChoiceA = EAct1Choice::M4_KilledTylerGreg;
	TylerChoice.ChoiceB = EAct1Choice::M4_SparedTylerGreg;
	Choices.Add(TylerChoice);
}

void AGIAct1Mission4GameMode::OnMissionBegin()
{
	TylerGreg = ResolveMissionNPC(TylerGregTag);
	if (TylerGreg)
	{
		TylerGreg->OnNPCKilled.AddDynamic(this, &AGIAct1Mission4GameMode::OnTylerKilled);
		TylerGreg->OnNPCInteracted.AddDynamic(this, &AGIAct1Mission4GameMode::OnTylerInteracted);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Mission4: Tyler Greg NPC with tag '%s' not found; auto-resolving encounter as spared."), *TylerGregTag.ToString());
		bTylerResolved = true;
		CompleteObjective(FName("M4_ResolveTyler"));
		if (!HasChoiceBeenMade(FName("M4_TylerChoice")))
		{
			MakeChoice(FName("M4_TylerChoice"), EAct1Choice::M4_SparedTylerGreg);
		}
	}

	GetWorldTimerManager().SetTimer(ObjectiveCheckTimer, this, &AGIAct1Mission4GameMode::CheckMissionObjectives, 0.25f, true);
}

void AGIAct1Mission4GameMode::OnMissionEnd(bool bSuccess)
{
	GetWorldTimerManager().ClearTimer(ObjectiveCheckTimer);
}

void AGIAct1Mission4GameMode::OnTylerKilled(AGICampaignNPC* DeadNPC)
{
	bTylerResolved = true;
	CompleteObjective(FName("M4_ResolveTyler"));

	if (!HasChoiceBeenMade(FName("M4_TylerChoice")))
	{
		MakeChoice(FName("M4_TylerChoice"), EAct1Choice::M4_KilledTylerGreg);
	}
}

void AGIAct1Mission4GameMode::OnTylerInteracted(AGICampaignNPC* NPC, AActor* Interactor)
{
	bTylerResolved = true;
	CompleteObjective(FName("M4_ResolveTyler"));

	if (!HasChoiceBeenMade(FName("M4_TylerChoice")))
	{
		MakeChoice(FName("M4_TylerChoice"), EAct1Choice::M4_SparedTylerGreg);
	}
}

void AGIAct1Mission4GameMode::CheckMissionObjectives()
{
	if (!bReachedPerimeter && GIAct1MissionHelpers::IsPlayerNearActor(GetWorld(), FindWorldActorByTag(PerimeterTag), ObjectiveDistance))
	{
		bReachedPerimeter = true;
		CompleteObjective(FName("M4_ReachPerimeter"));
	}

	if (!bTylerResolved)
	{
		return;
	}

	if (GIAct1MissionHelpers::IsPlayerNearActor(GetWorld(), FindWorldActorByTag(ExtractionTag), ObjectiveDistance))
	{
		CompleteObjective(FName("M4_Extract"));
	}
}

// ============================================================================
// MISSION 5
// ============================================================================

void AGIAct1Mission5GameMode::SetupMission()
{
	MissionID = ECampaignMission::Act1_Mission5_FirebaseKestrel;

	Objectives.Empty();
	Choices.Empty();

	FGIMissionObjective Infiltrate;
	Infiltrate.Type = EGIMissionObjectiveType::ReachLocation;
	Infiltrate.Description = FText::FromString(TEXT("Infiltrate Firebase Kestrel"));
	Infiltrate.ObjectiveTag = FName("M5_Infiltrate");
	Objectives.Add(Infiltrate);

	FGIMissionObjective SecureBriggs;
	SecureBriggs.Type = EGIMissionObjectiveType::Escort;
	SecureBriggs.Description = FText::FromString(TEXT("Secure Lieutenant Briggs"));
	SecureBriggs.ObjectiveTag = FName("M5_SecureBriggs");
	Objectives.Add(SecureBriggs);

	FGIMissionObjective EscortToTunnel;
	EscortToTunnel.Type = EGIMissionObjectiveType::Escort;
	EscortToTunnel.Description = FText::FromString(TEXT("Escort Briggs to tunnel"));
	EscortToTunnel.ObjectiveTag = FName("M5_EscortTunnel");
	Objectives.Add(EscortToTunnel);
}

void AGIAct1Mission5GameMode::OnMissionBegin()
{
	Briggs = ResolveMissionNPC(BriggsTag);
	if (Briggs)
	{
		Briggs->OnNPCInteracted.AddDynamic(this, &AGIAct1Mission5GameMode::OnBriggsInteracted);
		Briggs->SetNPCState(ECampaignNPCState::Escorted);

		BriggsEscort = NewObject<UGIEscortComponent>(Briggs);
		if (BriggsEscort)
		{
			BriggsEscort->RegisterComponent();
			BriggsEscort->OnEscortReachedDestination.AddDynamic(this, &AGIAct1Mission5GameMode::OnBriggsReachedTunnel);
			BriggsEscort->OnEscorteeStumbled.AddDynamic(this, &AGIAct1Mission5GameMode::OnBriggsStumbled);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Mission5: Briggs NPC with tag '%s' not found; auto-completing escort chain."), *BriggsTag.ToString());
		CompleteObjective(FName("M5_SecureBriggs"));
		CompleteObjective(FName("M5_EscortTunnel"));
	}

	GetWorldTimerManager().SetTimer(ObjectiveCheckTimer, this, &AGIAct1Mission5GameMode::CheckMissionObjectives, 0.25f, true);
}

void AGIAct1Mission5GameMode::OnMissionEnd(bool bSuccess)
{
	GetWorldTimerManager().ClearTimer(ObjectiveCheckTimer);
}

void AGIAct1Mission5GameMode::OnBriggsInteracted(AGICampaignNPC* NPC, AActor* Interactor)
{
	if (bBriggsSecured)
	{
		return;
	}

	bBriggsSecured = true;
	CompleteObjective(FName("M5_SecureBriggs"));

	const AActor* TunnelExit = FindWorldActorByTag(TunnelExitTag);
	if (BriggsEscort && TunnelExit)
	{
		BriggsEscort->StartEscort(Interactor, TunnelExit->GetActorLocation());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Mission5: Tunnel exit with tag '%s' not found; completing escort objective."), *TunnelExitTag.ToString());
		CompleteObjective(FName("M5_EscortTunnel"));
	}
}

void AGIAct1Mission5GameMode::OnBriggsReachedTunnel()
{
	CompleteObjective(FName("M5_EscortTunnel"));
}

void AGIAct1Mission5GameMode::OnBriggsStumbled()
{
	// Stumble is currently a gameplay pressure event only.
}

void AGIAct1Mission5GameMode::CheckMissionObjectives()
{
	if (!bInfiltrationComplete && GIAct1MissionHelpers::IsPlayerNearActor(GetWorld(), FindWorldActorByTag(KestrelCoreTag), ObjectiveDistance))
	{
		bInfiltrationComplete = true;
		CompleteObjective(FName("M5_Infiltrate"));
	}
}

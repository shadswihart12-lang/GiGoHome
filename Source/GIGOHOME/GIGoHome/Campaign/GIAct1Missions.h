// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Act 1 Mission Game Modes
// Five concrete mission implementations for Act 1: The Ash

#pragma once

#include "CoreMinimal.h"
#include "GIGoHome/Campaign/GIMissionGameMode.h"
#include "GIAct1Missions.generated.h"

class AGIRadioActor;
class AGICampaignNPC;
class AGIObservationPoint;
class UGIEscortComponent;

/**
 * Mission 1: The Proving
 * - Neutralize radio operator
 * - Destroy radio
 * - Return to cell undetected
 */
UCLASS()
class GIGOHOME_API AGIAct1Mission1GameMode : public AGIMissionGameMode
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category="Mission1|Tags")
	FName RadioOperatorTag = FName("M1_RadioOperator");

	UPROPERTY(EditAnywhere, Category="Mission1|Tags")
	FName RadioActorTag = FName("M1_Radio");

	UPROPERTY(EditAnywhere, Category="Mission1|Tags")
	FName ExtractionTag = FName("M1_Extraction");

	UPROPERTY(EditAnywhere, Category="Mission1")
	float ExtractionDistance = 250.0f;

	FTimerHandle ExtractionCheckTimer;

	UPROPERTY()
	AGICampaignNPC* RadioOperator;

	UPROPERTY()
	AGIRadioActor* MissionRadio;

	virtual void SetupMission() override;
	virtual void OnMissionBegin() override;
	virtual void OnMissionEnd(bool bSuccess) override;

	UFUNCTION()
	void OnMissionRadioDestroyed(AGIRadioActor* DestroyedRadio);

	UFUNCTION()
	void OnRadioOperatorKilled(AGICampaignNPC* DeadNPC);

	void CheckExtraction();
};

/**
 * Mission 2: The Observation
 * - Reach three observation points
 * - Document shift rotations and radio equipment
 * - Extract undetected
 */
UCLASS()
class GIGOHOME_API AGIAct1Mission2GameMode : public AGIMissionGameMode
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category="Mission2|Tags")
	FName ExtractionTag = FName("M2_Extraction");

	UPROPERTY(EditAnywhere, Category="Mission2")
	float ExtractionDistance = 250.0f;

	UPROPERTY()
	TArray<AGIObservationPoint*> ObservationPoints;

	FTimerHandle ExtractionCheckTimer;

	virtual void SetupMission() override;
	virtual void OnMissionBegin() override;
	virtual void OnMissionEnd(bool bSuccess) override;

	UFUNCTION()
	void OnObservationPointComplete(AGIObservationPoint* Point);

	void CheckExtraction();
};

/**
 * Mission 3: The Distraction
 * - Hold listening post attention for 12 minutes
 * - Prevent radio contact
 * - Extract on signal
 */
UCLASS()
class GIGOHOME_API AGIAct1Mission3GameMode : public AGIMissionGameMode
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category="Mission3|Tags")
	FName PrimaryRadioTag = FName("M3_PrimaryRadio");

	UPROPERTY(EditAnywhere, Category="Mission3|Tags")
	FName BackupRadioTag = FName("M3_BackupRadio");

	UPROPERTY(EditAnywhere, Category="Mission3|Tags")
	FName RunnerTag = FName("M3_Runner");

	UPROPERTY(EditAnywhere, Category="Mission3|Tags")
	FName ExtractionTag = FName("M3_Extraction");

	UPROPERTY(EditAnywhere, Category="Mission3")
	float ExtractionDistance = 300.0f;

	UPROPERTY()
	AGIRadioActor* PrimaryRadio;

	UPROPERTY()
	AGIRadioActor* BackupRadio;

	UPROPERTY()
	AGICampaignNPC* RunnerNPC;

	FTimerHandle ExtractionCheckTimer;

	virtual void SetupMission() override;
	virtual void OnMissionBegin() override;
	virtual void OnMissionTimerExpired() override;
	virtual void OnMissionEnd(bool bSuccess) override;

	UFUNCTION()
	void OnPrimaryRadioUsed(AActor* User);

	UFUNCTION()
	void OnBackupRadioDestroyed(AGIRadioActor* DestroyedRadio);

	UFUNCTION()
	void OnRunnerKilled(AGICampaignNPC* DeadNPC);

	void CheckExtraction();
};

/**
 * Mission 4: Tyler Greg
 * - Reach firebase perimeter
 * - Locate radio operator Tyler Greg
 * - Extract officer intel
 */
UCLASS()
class GIGOHOME_API AGIAct1Mission4GameMode : public AGIMissionGameMode
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category="Mission4|Tags")
	FName TylerGregTag = FName("M4_TylerGreg");

	UPROPERTY(EditAnywhere, Category="Mission4|Tags")
	FName PerimeterTag = FName("M4_Perimeter");

	UPROPERTY(EditAnywhere, Category="Mission4|Tags")
	FName ExtractionTag = FName("M4_Extraction");

	UPROPERTY(EditAnywhere, Category="Mission4")
	float ObjectiveDistance = 300.0f;

	UPROPERTY()
	AGICampaignNPC* TylerGreg;

	FTimerHandle ObjectiveCheckTimer;

	bool bReachedPerimeter = false;
	bool bTylerResolved = false;

	virtual void SetupMission() override;
	virtual void OnMissionBegin() override;
	virtual void OnMissionEnd(bool bSuccess) override;

	UFUNCTION()
	void OnTylerKilled(AGICampaignNPC* DeadNPC);

	UFUNCTION()
	void OnTylerInteracted(AGICampaignNPC* NPC, AActor* Interactor);

	void CheckMissionObjectives();
};

/**
 * Mission 5: Firebase Kestrel
 * - Infiltrate Kestrel
 * - Extract Lieutenant Briggs
 * - Escort to tunnel network
 */
UCLASS()
class GIGOHOME_API AGIAct1Mission5GameMode : public AGIMissionGameMode
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category="Mission5|Tags")
	FName BriggsTag = FName("M5_Briggs");

	UPROPERTY(EditAnywhere, Category="Mission5|Tags")
	FName KestrelCoreTag = FName("M5_KestrelCore");

	UPROPERTY(EditAnywhere, Category="Mission5|Tags")
	FName TunnelExitTag = FName("M5_TunnelExit");

	UPROPERTY(EditAnywhere, Category="Mission5")
	float ObjectiveDistance = 300.0f;

	UPROPERTY()
	AGICampaignNPC* Briggs;

	UPROPERTY()
	UGIEscortComponent* BriggsEscort;

	FTimerHandle ObjectiveCheckTimer;

	bool bInfiltrationComplete = false;
	bool bBriggsSecured = false;

	virtual void SetupMission() override;
	virtual void OnMissionBegin() override;
	virtual void OnMissionEnd(bool bSuccess) override;

	UFUNCTION()
	void OnBriggsInteracted(AGICampaignNPC* NPC, AActor* Interactor);

	UFUNCTION()
	void OnBriggsReachedTunnel();

	UFUNCTION()
	void OnBriggsStumbled();

	void CheckMissionObjectives();
};

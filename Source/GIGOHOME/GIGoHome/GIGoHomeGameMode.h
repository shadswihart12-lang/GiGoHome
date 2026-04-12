// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GIGoHomeGameMode.generated.h"

class AGIGoHomeCampaignState;

/**
 * Game mode for GI Go Home
 * Single-player narrative campaign - no respawn, no score
 * Xi Dong's death ends the current mission attempt
 * Manages campaign act progression and mission completion
 */
UCLASS(abstract)
class GIGOHOME_API AGIGoHomeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	AGIGoHomeGameMode();

public:

	/** Called by XiDongCharacter when the player dies - no respawn */
	virtual void OnPlayerDeath();

	/** Called to complete current mission and advance campaign state */
	UFUNCTION(BlueprintCallable, Category="GIGoHome")
	void CompleteMission();

	/** Returns the campaign state cast from GameState */
	UFUNCTION(BlueprintPure, Category="GIGoHome")
	AGIGoHomeCampaignState* GetCampaignState() const;

protected:

	/** Called when Xi Dong dies - override to show death screen, allow retry */
	virtual void OnPlayerDeathEffect() {}

	/** Called when mission completed - override to trigger transition */
	virtual void OnMissionCompleteEffect() {};
};

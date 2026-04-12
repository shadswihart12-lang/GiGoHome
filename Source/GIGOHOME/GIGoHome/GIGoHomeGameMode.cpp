// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/GIGoHomeGameMode.h"
#include "GIGoHome/GIGoHomeCampaignState.h"

AGIGoHomeGameMode::AGIGoHomeGameMode()
{
	GameStateClass = AGIGoHomeCampaignState::StaticClass();
}

void AGIGoHomeGameMode::OnPlayerDeath()
{
	OnPlayerDeathEffect();
}

void AGIGoHomeGameMode::CompleteMission()
{
	if (AGIGoHomeCampaignState* CS = GetCampaignState())
	{
		CS->AdvanceAct();
	}

	OnMissionCompleteEffect();
}

AGIGoHomeCampaignState* AGIGoHomeGameMode::GetCampaignState() const
{
	return Cast<AGIGoHomeCampaignState>(GameState);
}

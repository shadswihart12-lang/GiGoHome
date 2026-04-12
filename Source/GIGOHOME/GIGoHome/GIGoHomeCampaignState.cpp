// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/GIGoHomeCampaignState.h"

void AGIGoHomeCampaignState::AdvanceAct()
{
	switch (CurrentAct)
	{
		case ECampaignAct::Act1_TheAsh:
			CurrentAct = ECampaignAct::Act2_TheSquad;
			break;
		case ECampaignAct::Act2_TheSquad:
			CurrentAct = ECampaignAct::Act3_TheLongWar;
			break;
		default:
			// already in Act 3 - no further advancement
			break;
	}
}

void AGIGoHomeCampaignState::RecordChoice(EPlayerChoice Choice)
{
	PlayerChoices.Add(Choice);
}

void AGIGoHomeCampaignState::RecordMissionStarted(ECampaignMission MissionId)
{
	++TotalMissionAttempts;
	LastMissionId = MissionId;
}

void AGIGoHomeCampaignState::RecordMissionEnded(ECampaignMission MissionId, bool bSuccess, float DurationSeconds)
{
	LastMissionId = MissionId;
	LastMissionDurationSeconds = FMath::Max(0.0f, DurationSeconds);

	if (bSuccess)
	{
		++TotalMissionSuccesses;
	}
	else
	{
		++TotalMissionFailures;
	}
}

void AGIGoHomeCampaignState::RecordMissionRestarted(ECampaignMission MissionId)
{
	++TotalMissionRestarts;
	LastMissionId = MissionId;
}

FString AGIGoHomeCampaignState::GetTelemetrySummary() const
{
	const float SuccessRate = (TotalMissionAttempts > 0)
		? (static_cast<float>(TotalMissionSuccesses) / static_cast<float>(TotalMissionAttempts)) * 100.0f
		: 0.0f;

	return FString::Printf(
		TEXT("Attempts=%d | Successes=%d | Failures=%d | Restarts=%d | SuccessRate=%.1f%% | LastMission=%d | LastDuration=%.1fs"),
		TotalMissionAttempts,
		TotalMissionSuccesses,
		TotalMissionFailures,
		TotalMissionRestarts,
		SuccessRate,
		static_cast<int32>(LastMissionId),
		LastMissionDurationSeconds
	);
}

void AGIGoHomeCampaignState::ResetTelemetry()
{
	TotalMissionAttempts = 0;
	TotalMissionSuccesses = 0;
	TotalMissionFailures = 0;
	TotalMissionRestarts = 0;
	LastMissionDurationSeconds = 0.0f;
	LastMissionId = ECampaignMission::None;
}

void AGIGoHomeCampaignState::MarkSquadMemberDead(FName MemberTag)
{
	// remove from wounded list if present
	WoundedSquadMembers.Remove(MemberTag);

	// add to permanent dead list
	PermanentlyDeadSquadMembers.AddUnique(MemberTag);
}

void AGIGoHomeCampaignState::MarkSquadMemberWounded(FName MemberTag)
{
	// only wound if not permanently dead
	if (!PermanentlyDeadSquadMembers.Contains(MemberTag))
	{
		WoundedSquadMembers.AddUnique(MemberTag);
	}
}

void AGIGoHomeCampaignState::ClearSquadMemberWound(FName MemberTag)
{
	WoundedSquadMembers.Remove(MemberTag);
}

bool AGIGoHomeCampaignState::HasMadeChoice(EPlayerChoice Choice) const
{
	return PlayerChoices.Contains(Choice);
}

bool AGIGoHomeCampaignState::IsSquadMemberDead(FName MemberTag) const
{
	return PermanentlyDeadSquadMembers.Contains(MemberTag);
}

int32 AGIGoHomeCampaignState::GetEndingIndex() const
{
	// Ending 0: Xi Dong walks away into the jungle
	if (HasMadeChoice(EPlayerChoice::ChoseToDisappearIntoJungle))
	{
		return 0;
	}

	// Ending 2: Ambiguous - spared and killed surrendered soldiers
	if (HasMadeChoice(EPlayerChoice::SparedSurrenderedSoldier) &&
		HasMadeChoice(EPlayerChoice::KilledSurrenderedSoldier))
	{
		return 2;
	}

	// Ending 1: Xi Dong keeps fighting (default)
	return 1;
}

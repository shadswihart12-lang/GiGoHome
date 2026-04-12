// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Player/Components/SquadCommandComponent.h"
#include "GIGoHome/Squad/SquadMemberCharacter.h"
#include "GIGoHome/Squad/SquadMemberAIController.h"

USquadCommandComponent::USquadCommandComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USquadCommandComponent::AddSquadMember(ASquadMemberCharacter* NewMember)
{
	if (!NewMember)
	{
		return;
	}

	if (SquadMembers.Num() < MaxSquadSize && !SquadMembers.Contains(NewMember))
	{
		SquadMembers.Add(NewMember);
	}
}

void USquadCommandComponent::RemoveSquadMember(ASquadMemberCharacter* Member)
{
	SquadMembers.Remove(Member);
}

void USquadCommandComponent::IssueOrder(ESquadOrder Order)
{
	for (ASquadMemberCharacter* Member : SquadMembers)
	{
		if (!IsValid(Member))
		{
			continue;
		}

		// skip wounded or dead members
		if (Member->IsWounded() || Member->ActorHasTag(FName("Dead")))
		{
			continue;
		}

		if (ASquadMemberAIController* AICtrl = Cast<ASquadMemberAIController>(Member->GetController()))
		{
			AICtrl->ReceiveOrder(Order);
		}
	}
}

int32 USquadCommandComponent::GetActiveSquadMemberCount() const
{
	int32 Count = 0;
	for (const ASquadMemberCharacter* Member : SquadMembers)
	{
		if (IsValid(Member) && !Member->IsWounded() && !Member->ActorHasTag(FName("Dead")))
		{
			++Count;
		}
	}
	return Count;
}

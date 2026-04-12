// Copyright Epic Games, Inc. All Rights Reserved.


#include "HorrorUI.h"
#include "HorrorCharacter.h"

void UHorrorUI::SetupCharacter(AHorrorCharacter* HorrorCharacter)
{
	HorrorCharacter->OnSprintMeterUpdated.AddDynamic(this, &UHorrorUI::OnSprintMeterUpdated);
	HorrorCharacter->OnSprintStateChanged.AddDynamic(this, &UHorrorUI::OnSprintStateChanged);
}

void UHorrorUI::OnSprintMeterUpdated(float Percent)
{
	SprintMeterUpdatedEffect(Percent);
}

void UHorrorUI::OnSprintStateChanged(bool bSprinting)
{
	SprintStateChangedEffect(bSprinting);
}

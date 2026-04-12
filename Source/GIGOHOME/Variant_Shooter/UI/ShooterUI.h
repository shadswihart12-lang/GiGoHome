// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterUI.generated.h"

/**
 *  Simple scoreboard UI for a first person shooter game
 */
UCLASS(abstract)
class GIGOHOME_API UShooterUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** Override to update score sub-widgets */
	virtual void UpdateScore(uint8 TeamByte, int32 Score) {}
};

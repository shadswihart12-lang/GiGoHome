// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterBulletCounterUI.generated.h"

/**
 *  Simple bullet counter UI widget for a first person shooter game
 */
UCLASS(abstract)
class GIGOHOME_API UShooterBulletCounterUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** Override to update sub-widgets with the new bullet count */
	virtual void UpdateBulletCounter(int32 MagazineSize, int32 BulletCount) {}

	/** Override to update sub-widgets with the new life total and play a damage effect on the HUD */
	virtual void OnDamaged(float LifePercent) {}
};

// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Vertical Slice UI Components
// Per GDD: Minimal HUD philosophy - these are full-screen state changes, not persistent UI

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GISliceHUDWidget.generated.h"

class UTextBlock;
class UImage;
class UButton;

/**
 * Minimal HUD widget for the vertical slice.
 * 
 * Per GDD Section 8.3: "No kill counters, no XP bars, no progression metrics"
 * This widget only shows:
 * - Ammo count (when explicitly checked by player)
 * - Death screen (on player death)
 * - Victory screen (on mission complete)
 * 
 * Health is NOT displayed — per GDD, health is shown through
 * screen effects and breathing audio.
 */
UCLASS()
class GIGOHOME_API UGISliceHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Show the ammo display briefly (called by manual ammo check) */
	UFUNCTION(BlueprintCallable, Category="HUD")
	void ShowAmmoCheck(int32 CurrentAmmo, int32 MagazineSize);

	/** Show death screen with fade to black */
	UFUNCTION(BlueprintCallable, Category="HUD")
	void ShowDeathScreen();

	/** Show victory screen */
	UFUNCTION(BlueprintCallable, Category="HUD")
	void ShowVictoryScreen();

	/** Hide all overlays */
	UFUNCTION(BlueprintCallable, Category="HUD")
	void HideAllOverlays();

protected:

	virtual void NativeConstruct() override;

	// --- Ammo Display ---
	UPROPERTY(meta=(BindWidget), BlueprintReadOnly)
	UTextBlock* AmmoText;

	UPROPERTY(meta=(BindWidgetOptional))
	UImage* AmmoBackground;

	// --- Death Screen ---
	UPROPERTY(meta=(BindWidgetOptional))
	UImage* DeathOverlay;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* DeathText;

	UPROPERTY(meta=(BindWidgetOptional))
	UButton* RetryButton;

	UPROPERTY(meta=(BindWidgetOptional))
	UButton* QuitButton;

	// --- Victory Screen ---
	UPROPERTY(meta=(BindWidgetOptional))
	UImage* VictoryOverlay;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* VictoryText;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* VictorySubtext;

	UPROPERTY(meta=(BindWidgetOptional))
	UButton* ContinueButton;

	// --- Timing ---
	UPROPERTY(EditAnywhere, Category="Timing")
	float AmmoDisplayDuration = 2.0f;

	FTimerHandle AmmoHideTimer;

	UFUNCTION()
	void HideAmmoDisplay();

	UFUNCTION()
	void OnRetryClicked();

	UFUNCTION()
	void OnQuitClicked();

	UFUNCTION()
	void OnContinueClicked();
};

// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Pure Slate HUD
// No Blueprint required. Production-ready.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "GISlateHUD.generated.h"

class STextBlock;
class SImage;
class SButton;
class SBorder;
class SOverlay;

/**
 * Pure C++ Slate HUD for the vertical slice.
 * Displays: Ammo Check, Death Screen, Victory Screen
 * Per GDD: Minimal HUD - no kill counters, no XP bars
 */
class GIGOHOME_API SGISlateHUD : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGISlateHUD) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Show ammo check display briefly */
	void ShowAmmoCheck(int32 CurrentAmmo, int32 MagazineSize);

	/** Show wave notification briefly (e.g. "WAVE 2 / 5") */
	void ShowWaveNotification(int32 Wave, int32 Total);

	/** Show mission countdown timer (Mission 3: 12-minute hold) */
	void ShowMissionTimer(float Seconds);

	/** Hide the mission countdown timer */
	void HideMissionTimer();

	/** Show death screen with retry/quit options */
	void ShowDeathScreen();

	/** Show victory screen */
	void ShowVictoryScreen();

	/** Hide all overlays */
	void HideAll();

	/** Set world context for timers */
	void SetWorld(UWorld* InWorld) { WorldContext = InWorld; }

private:
	// Crosshair
	TSharedPtr<SWidget> CrosshairWidget;

	// Ammo Display
	TSharedPtr<SBorder> AmmoContainer;
	TSharedPtr<STextBlock> AmmoText;

	// Wave Notification
	TSharedPtr<SBorder> WaveContainer;
	TSharedPtr<STextBlock> WaveText;

	// Mission Timer (Mission 3: 12-minute hold countdown)
	TSharedPtr<SBorder> TimerContainer;
	TSharedPtr<STextBlock> TimerText;

	// Death Screen
	TSharedPtr<SBorder> DeathContainer;
	TSharedPtr<STextBlock> DeathTitleText;
	TSharedPtr<SButton> RetryButton;
	TSharedPtr<SButton> QuitButton;

	// Victory Screen
	TSharedPtr<SBorder> VictoryContainer;
	TSharedPtr<STextBlock> VictoryTitleText;
	TSharedPtr<STextBlock> VictorySubText;
	TSharedPtr<SButton> ContinueButton;

	// Timer for hiding ammo
	FTimerHandle AmmoHideTimer;

	// Timer for hiding wave notification
	FTimerHandle WaveHideTimer;

	// World context for timers
	TWeakObjectPtr<UWorld> WorldContext;

	void HideAmmoDisplay();
	void HideWaveNotification();

	FReply OnRetryClicked();
	FReply OnQuitClicked();
	FReply OnContinueClicked();
};

/**
 * HUD wrapper class that manages the Slate widget
 * Added to viewport by GameMode
 */
UCLASS()
class GIGOHOME_API UGISlateHUDManager : public UObject
{
	GENERATED_BODY()

public:
	/** Creates and adds the HUD to viewport */
	void CreateHUD(UWorld* InWorld);

	/** Removes HUD from viewport */
	void DestroyHUD();

	/** Show ammo check */
	void ShowAmmoCheck(int32 CurrentAmmo, int32 MagazineSize);

	/** Show wave notification */
	void ShowWaveNotification(int32 Wave, int32 Total);

	/** Show mission countdown timer */
	void ShowMissionTimer(float Seconds);

	/** Hide mission countdown timer */
	void HideMissionTimer();

	/** Show death screen */
	void ShowDeathScreen();

	/** Show victory screen */
	void ShowVictoryScreen();

	/** Hide all overlays */
	void HideAll();

	/** Returns true if HUD exists */
	bool IsValid() const { return SlateHUD.IsValid(); }

private:
	TSharedPtr<SGISlateHUD> SlateHUD;
	TWeakObjectPtr<UWorld> WorldRef;
};

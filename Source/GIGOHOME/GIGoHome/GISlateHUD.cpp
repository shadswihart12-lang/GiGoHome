// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Pure Slate HUD Implementation

#include "GIGoHome/GISlateHUD.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"

// ============================================================================
// SGISlateHUD - Pure Slate Widget
// ============================================================================

void SGISlateHUD::Construct(const FArguments& InArgs)
{
	// Define colors per GDD aesthetic
	const FLinearColor BackgroundDark(0.02f, 0.02f, 0.02f, 0.85f);
	const FLinearColor AccentRed(0.8f, 0.15f, 0.1f, 1.0f);
	const FLinearColor AccentGold(0.85f, 0.75f, 0.3f, 1.0f);
	const FLinearColor TextWhite(0.95f, 0.95f, 0.95f, 1.0f);
	const FLinearColor TextGray(0.6f, 0.6f, 0.6f, 1.0f);

	// Font styles
	FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 48);
	FSlateFontInfo SubtitleFont = FCoreStyle::GetDefaultFontStyle("Regular", 24);
	FSlateFontInfo ButtonFont = FCoreStyle::GetDefaultFontStyle("Bold", 18);
	FSlateFontInfo AmmoFont = FCoreStyle::GetDefaultFontStyle("Bold", 32);

	ChildSlot
	[
		SNew(SOverlay)

		// ========== CROSSHAIR ==========
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(20.0f)
			.HeightOverride(20.0f)
			[
				SNew(SOverlay)
				// Horizontal bar
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				[
					SNew(SBorder)
					.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.8f))
					.Padding(FMargin(0.0f, 1.0f))
				]
				// Vertical bar
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Fill)
				[
					SNew(SBorder)
					.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.8f))
					.Padding(FMargin(1.0f, 0.0f))
				]
			]
		]

		// ========== WAVE NOTIFICATION ==========
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(0, 80, 0, 0)
		[
			SAssignNew(WaveContainer, SBorder)
			.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
			.Padding(FMargin(40, 16))
			.Visibility(EVisibility::Collapsed)
			[
				SAssignNew(WaveText, STextBlock)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
				.ColorAndOpacity(FLinearColor(0.85f, 0.75f, 0.3f, 1.0f))
				.Text(FText::FromString(TEXT("WAVE 1 / 5")))
			]
		]

		// ========== MISSION TIMER (Mission 3: 12-minute hold) ==========
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(0, 30, 0, 0)
		[
			SAssignNew(TimerContainer, SBorder)
			.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
			.Padding(FMargin(40.0f, 14.0f))
			.Visibility(EVisibility::Collapsed)
			[
				SAssignNew(TimerText, STextBlock)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 38))
				.ColorAndOpacity(FLinearColor(0.85f, 0.75f, 0.3f, 1.0f))  // AccentGold — matches wave notif
				.Text(FText::FromString(TEXT("12:00")))
				.Justification(ETextJustify::Center)
			]
		]

		// ========== AMMO CHECK DISPLAY ==========
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(0, 0, 0, 150)
		[
			SAssignNew(AmmoContainer, SBorder)
			.BorderBackgroundColor(BackgroundDark)
			.Padding(FMargin(30, 15))
			.Visibility(EVisibility::Collapsed)
			[
				SAssignNew(AmmoText, STextBlock)
				.Font(AmmoFont)
				.ColorAndOpacity(TextWhite)
				.Text(FText::FromString(TEXT("Full")))
			]
		]

		// ========== DEATH SCREEN ==========
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(DeathContainer, SBorder)
			.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f))
			.Visibility(EVisibility::Collapsed)
			[
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					
					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0, 0, 0, 50)
					[
						SAssignNew(DeathTitleText, STextBlock)
						.Font(TitleFont)
						.ColorAndOpacity(AccentRed)
						.Text(FText::FromString(TEXT("GI GO HOME")))
					]

					// Retry Button
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0, 10)
					[
						SAssignNew(RetryButton, SButton)
						.OnClicked(this, &SGISlateHUD::OnRetryClicked)
						.ContentPadding(FMargin(50, 15))
						[
							SNew(STextBlock)
							.Font(ButtonFont)
							.Text(FText::FromString(TEXT("RETRY")))
						]
					]

					// Quit Button
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0, 10)
					[
						SAssignNew(QuitButton, SButton)
						.OnClicked(this, &SGISlateHUD::OnQuitClicked)
						.ContentPadding(FMargin(50, 15))
						[
							SNew(STextBlock)
							.Font(ButtonFont)
							.Text(FText::FromString(TEXT("QUIT")))
						]
					]
				]
			]
		]

		// ========== VICTORY SCREEN ==========
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(VictoryContainer, SBorder)
			.BorderBackgroundColor(FLinearColor(0.0f, 0.02f, 0.0f, 0.9f))
			.Visibility(EVisibility::Collapsed)
			[
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					
					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0, 0, 0, 20)
					[
						SAssignNew(VictoryTitleText, STextBlock)
						.Font(TitleFont)
						.ColorAndOpacity(AccentGold)
						.Text(FText::FromString(TEXT("VICTORY")))
					]

					// Subtitle
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0, 0, 0, 50)
					[
						SAssignNew(VictorySubText, STextBlock)
						.Font(SubtitleFont)
						.ColorAndOpacity(TextGray)
						.Text(FText::FromString(TEXT("The patrol has been eliminated")))
					]

					// Continue Button
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0, 10)
					[
						SAssignNew(ContinueButton, SButton)
						.OnClicked(this, &SGISlateHUD::OnContinueClicked)
						.ContentPadding(FMargin(50, 15))
						[
							SNew(STextBlock)
							.Font(ButtonFont)
							.Text(FText::FromString(TEXT("CONTINUE")))
						]
					]
				]
			]
		]
	];
}

void SGISlateHUD::ShowAmmoCheck(int32 CurrentAmmo, int32 MagazineSize)
{
	if (!AmmoContainer.IsValid() || !AmmoText.IsValid()) return;

	AmmoText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentAmmo, MagazineSize)));
	AmmoContainer->SetVisibility(EVisibility::Visible);

	// Auto-hide after 3 seconds
	if (WorldContext.IsValid())
	{
		TWeakPtr<SGISlateHUD> WeakSelf = StaticCastSharedRef<SGISlateHUD>(AsShared());
		WorldContext->GetTimerManager().SetTimer(
			AmmoHideTimer,
			FTimerDelegate::CreateLambda([WeakSelf]()
			{
				if (TSharedPtr<SGISlateHUD> Self = WeakSelf.Pin())
				{
					Self->HideAmmoDisplay();
				}
			}),
			3.0f,
			false
		);
	}
}

void SGISlateHUD::HideAmmoDisplay()
{
	if (AmmoContainer.IsValid())
	{
		AmmoContainer->SetVisibility(EVisibility::Collapsed);
	}
}

void SGISlateHUD::ShowWaveNotification(int32 Wave, int32 Total)
{
	if (!WaveContainer.IsValid() || !WaveText.IsValid()) return;

	WaveText->SetText(FText::FromString(FString::Printf(TEXT("WAVE  %d / %d"), Wave, Total)));
	WaveContainer->SetVisibility(EVisibility::Visible);

	// Auto-hide after 4 seconds
	if (WorldContext.IsValid())
	{
		TWeakPtr<SGISlateHUD> WeakSelf = StaticCastSharedRef<SGISlateHUD>(AsShared());
		WorldContext->GetTimerManager().SetTimer(
			WaveHideTimer,
			FTimerDelegate::CreateLambda([WeakSelf]()
			{
				if (TSharedPtr<SGISlateHUD> Self = WeakSelf.Pin())
				{
					Self->HideWaveNotification();
				}
			}),
			4.0f,
			false
		);
	}
}

void SGISlateHUD::HideWaveNotification()
{
	if (WaveContainer.IsValid())
	{
		WaveContainer->SetVisibility(EVisibility::Collapsed);
	}
}

void SGISlateHUD::ShowMissionTimer(float Seconds)
{
	if (!TimerContainer.IsValid() || !TimerText.IsValid()) return;

	// Format as MM:SS countdown
	const int32 TotalSeconds = FMath::Max(0, FMath::FloorToInt(Seconds));
	const int32 Minutes      = TotalSeconds / 60;
	const int32 SecsLeft     = TotalSeconds % 60;

	// Pulse red in the final 60 seconds — gives the player a pressure signal without breaking the minimal HUD aesthetic
	const FLinearColor TimerColor = (TotalSeconds <= 60)
		? FLinearColor(0.9f, 0.15f, 0.1f, 1.0f)   // AccentRed — last minute
		: FLinearColor(0.85f, 0.75f, 0.3f, 1.0f);  // AccentGold — normal

	TimerText->SetText(FText::FromString(
		FString::Printf(TEXT("%02d:%02d"), Minutes, SecsLeft)));
	TimerText->SetColorAndOpacity(FSlateColor(TimerColor));
	TimerContainer->SetVisibility(EVisibility::Visible);
}

void SGISlateHUD::HideMissionTimer()
{
	if (TimerContainer.IsValid())
	{
		TimerContainer->SetVisibility(EVisibility::Collapsed);
	}
}

void SGISlateHUD::ShowDeathScreen()
{
	HideAll();

	if (DeathContainer.IsValid())
	{
		DeathContainer->SetVisibility(EVisibility::Visible);
	}

	// Enable mouse cursor
	if (WorldContext.IsValid())
	{
		APlayerController* PC = WorldContext->GetFirstPlayerController();
		if (PC)
		{
			PC->SetShowMouseCursor(true);
			PC->SetInputMode(FInputModeUIOnly());
			
			// Set focus to the widget
			FSlateApplication::Get().SetUserFocusToGameViewport(0);
		}
	}
}

void SGISlateHUD::ShowVictoryScreen()
{
	HideAll();

	if (VictoryContainer.IsValid())
	{
		VictoryContainer->SetVisibility(EVisibility::Visible);
	}

	// Enable mouse cursor
	if (WorldContext.IsValid())
	{
		APlayerController* PC = WorldContext->GetFirstPlayerController();
		if (PC)
		{
			PC->SetShowMouseCursor(true);
			PC->SetInputMode(FInputModeUIOnly());
		}
	}
}

void SGISlateHUD::HideAll()
{
	if (AmmoContainer.IsValid())    AmmoContainer->SetVisibility(EVisibility::Collapsed);
	if (WaveContainer.IsValid())    WaveContainer->SetVisibility(EVisibility::Collapsed);
	if (TimerContainer.IsValid())   TimerContainer->SetVisibility(EVisibility::Collapsed);
	if (DeathContainer.IsValid())   DeathContainer->SetVisibility(EVisibility::Collapsed);
	if (VictoryContainer.IsValid()) VictoryContainer->SetVisibility(EVisibility::Collapsed);

	// Clear timers
	if (WorldContext.IsValid())
	{
		WorldContext->GetTimerManager().ClearTimer(AmmoHideTimer);
		WorldContext->GetTimerManager().ClearTimer(WaveHideTimer);
	}
}

FReply SGISlateHUD::OnRetryClicked()
{
	if (WorldContext.IsValid())
	{
		APlayerController* PC = WorldContext->GetFirstPlayerController();
		if (PC)
		{
			PC->SetShowMouseCursor(false);
			PC->SetInputMode(FInputModeGameOnly());
		}

		// Restart level
		UGameplayStatics::OpenLevel(WorldContext.Get(), FName(*WorldContext->GetName()));
	}
	return FReply::Handled();
}

FReply SGISlateHUD::OnQuitClicked()
{
	if (WorldContext.IsValid())
	{
		UKismetSystemLibrary::QuitGame(WorldContext.Get(), nullptr, EQuitPreference::Quit, false);
	}
	return FReply::Handled();
}

FReply SGISlateHUD::OnContinueClicked()
{
	// For vertical slice, restart to demonstrate the loop
	return OnRetryClicked();
}

// ============================================================================
// UGISlateHUDManager - UObject Wrapper
// ============================================================================

void UGISlateHUDManager::CreateHUD(UWorld* InWorld)
{
	if (SlateHUD.IsValid()) return;

	WorldRef = InWorld;

	SlateHUD = SNew(SGISlateHUD);
	SlateHUD->SetWorld(InWorld);

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->AddViewportWidgetForPlayer(
			nullptr, // Local player (nullptr = first player)
			SlateHUD.ToSharedRef(),
			100 // Z-order
		);
	}
}

void UGISlateHUDManager::DestroyHUD()
{
	if (SlateHUD.IsValid())
	{
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveViewportWidgetForPlayer(nullptr, SlateHUD.ToSharedRef());
		}
		SlateHUD.Reset();
	}
}

void UGISlateHUDManager::ShowAmmoCheck(int32 CurrentAmmo, int32 MagazineSize)
{
	if (SlateHUD.IsValid())
	{
		SlateHUD->ShowAmmoCheck(CurrentAmmo, MagazineSize);
	}
}

void UGISlateHUDManager::ShowWaveNotification(int32 Wave, int32 Total)
{
	if (SlateHUD.IsValid())
	{
		SlateHUD->ShowWaveNotification(Wave, Total);
	}
}

void UGISlateHUDManager::ShowMissionTimer(float Seconds)
{
	if (SlateHUD.IsValid())
	{
		SlateHUD->ShowMissionTimer(Seconds);
	}
}

void UGISlateHUDManager::HideMissionTimer()
{
	if (SlateHUD.IsValid())
	{
		SlateHUD->HideMissionTimer();
	}
}

void UGISlateHUDManager::ShowDeathScreen()
{
	if (SlateHUD.IsValid())
	{
		SlateHUD->ShowDeathScreen();
	}
}

void UGISlateHUDManager::ShowVictoryScreen()
{
	if (SlateHUD.IsValid())
	{
		SlateHUD->ShowVictoryScreen();
	}
}

void UGISlateHUDManager::HideAll()
{
	if (SlateHUD.IsValid())
	{
		SlateHUD->HideAll();
	}
}

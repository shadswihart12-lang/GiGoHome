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
}

void SGISlateHUD::HideAmmoDisplay()
{
	if (AmmoContainer.IsValid())
	{
		AmmoContainer->SetVisibility(EVisibility::Collapsed);
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
	if (AmmoContainer.IsValid()) AmmoContainer->SetVisibility(EVisibility::Collapsed);
	if (DeathContainer.IsValid()) DeathContainer->SetVisibility(EVisibility::Collapsed);
	if (VictoryContainer.IsValid()) VictoryContainer->SetVisibility(EVisibility::Collapsed);

	// Clear timer
	if (WorldContext.IsValid())
	{
		WorldContext->GetTimerManager().ClearTimer(AmmoHideTimer);
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

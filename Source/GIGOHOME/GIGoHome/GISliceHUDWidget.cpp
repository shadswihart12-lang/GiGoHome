// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/GISliceHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UGISliceHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Hide everything initially
	HideAllOverlays();

	// Bind button callbacks
	if (RetryButton)
	{
		RetryButton->OnClicked.AddDynamic(this, &UGISliceHUDWidget::OnRetryClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UGISliceHUDWidget::OnQuitClicked);
	}
	if (ContinueButton)
	{
		ContinueButton->OnClicked.AddDynamic(this, &UGISliceHUDWidget::OnContinueClicked);
	}
}

void UGISliceHUDWidget::ShowAmmoCheck(int32 CurrentAmmo, int32 MagazineSize)
{
	// Per GDD: Manual ammo check shows current/max in a non-intrusive way
	// "Xi Dong glances at his magazine"
	
	if (AmmoText)
	{
		FString AmmoString;
		
		// Vague descriptions instead of exact numbers (per minimal HUD philosophy)
		// But for the vertical slice, we show numbers
		if (CurrentAmmo >= MagazineSize)
		{
			AmmoString = TEXT("Full");
		}
		else if (CurrentAmmo > MagazineSize * 0.66f)
		{
			AmmoString = TEXT("Most left");
		}
		else if (CurrentAmmo > MagazineSize * 0.33f)
		{
			AmmoString = TEXT("Half left");
		}
		else if (CurrentAmmo > 0)
		{
			AmmoString = TEXT("Almost empty");
		}
		else
		{
			AmmoString = TEXT("Empty");
		}

		AmmoText->SetText(FText::FromString(AmmoString));
		AmmoText->SetVisibility(ESlateVisibility::Visible);
	}

	if (AmmoBackground)
	{
		AmmoBackground->SetVisibility(ESlateVisibility::Visible);
	}

	// Hide after duration
	GetWorld()->GetTimerManager().SetTimer(
		AmmoHideTimer,
		this,
		&UGISliceHUDWidget::HideAmmoDisplay,
		AmmoDisplayDuration,
		false
	);
}

void UGISliceHUDWidget::HideAmmoDisplay()
{
	if (AmmoText)
	{
		AmmoText->SetVisibility(ESlateVisibility::Hidden);
	}
	if (AmmoBackground)
	{
		AmmoBackground->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UGISliceHUDWidget::ShowDeathScreen()
{
	// Per GDD: "Xi Dong's death ends the current mission attempt"
	
	if (DeathOverlay)
	{
		DeathOverlay->SetVisibility(ESlateVisibility::Visible);
	}
	if (DeathText)
	{
		// Vietnamese for "Fallen in battle" or simply the game title
		DeathText->SetText(FText::FromString(TEXT("GI GO HOME")));
		DeathText->SetVisibility(ESlateVisibility::Visible);
	}
	if (RetryButton)
	{
		RetryButton->SetVisibility(ESlateVisibility::Visible);
	}
	if (QuitButton)
	{
		QuitButton->SetVisibility(ESlateVisibility::Visible);
	}

	// Show mouse cursor for button interaction
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeUIOnly());
	}
}

void UGISliceHUDWidget::ShowVictoryScreen()
{
	if (VictoryOverlay)
	{
		VictoryOverlay->SetVisibility(ESlateVisibility::Visible);
	}
	if (VictoryText)
	{
		VictoryText->SetText(FText::FromString(TEXT("Victory")));
		VictoryText->SetVisibility(ESlateVisibility::Visible);
	}
	if (VictorySubtext)
	{
		VictorySubtext->SetText(FText::FromString(TEXT("The patrol has been eliminated")));
		VictorySubtext->SetVisibility(ESlateVisibility::Visible);
	}
	if (ContinueButton)
	{
		ContinueButton->SetVisibility(ESlateVisibility::Visible);
	}

	// Show mouse cursor for button interaction
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeUIOnly());
	}
}

void UGISliceHUDWidget::HideAllOverlays()
{
	// Ammo
	if (AmmoText) AmmoText->SetVisibility(ESlateVisibility::Hidden);
	if (AmmoBackground) AmmoBackground->SetVisibility(ESlateVisibility::Hidden);

	// Death
	if (DeathOverlay) DeathOverlay->SetVisibility(ESlateVisibility::Hidden);
	if (DeathText) DeathText->SetVisibility(ESlateVisibility::Hidden);
	if (RetryButton) RetryButton->SetVisibility(ESlateVisibility::Hidden);
	if (QuitButton) QuitButton->SetVisibility(ESlateVisibility::Hidden);

	// Victory
	if (VictoryOverlay) VictoryOverlay->SetVisibility(ESlateVisibility::Hidden);
	if (VictoryText) VictoryText->SetVisibility(ESlateVisibility::Hidden);
	if (VictorySubtext) VictorySubtext->SetVisibility(ESlateVisibility::Hidden);
	if (ContinueButton) ContinueButton->SetVisibility(ESlateVisibility::Hidden);
}

void UGISliceHUDWidget::OnRetryClicked()
{
	// Restart the current level
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}

	UGameplayStatics::OpenLevel(GetWorld(), FName(*GetWorld()->GetName()));
}

void UGISliceHUDWidget::OnQuitClicked()
{
	// Return to main menu or quit
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}

void UGISliceHUDWidget::OnContinueClicked()
{
	// For vertical slice, just restart to demonstrate the loop
	OnRetryClicked();
}

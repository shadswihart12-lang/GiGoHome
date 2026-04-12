// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Narrative Director Implementation

#include "GIGoHome/Campaign/GINarrativeDirector.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "LevelSequence.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/GameViewportClient.h"

void UGINarrativeDirector::Initialize(UWorld* InWorld)
{
	CachedWorld = InWorld;
}

void UGINarrativeDirector::PlayMoment(const FNarrativeMoment& Moment)
{
	if (bIsPlaying)
	{
		StopCurrentMoment();
	}

	ActiveMoment = Moment;
	bIsPlaying = true;

	// Disable player input during cutscene
	if (Moment.bDisablePlayerInput)
	{
		SetPlayerInputEnabled(false);
	}

	// Fade in if requested
	if (Moment.FadeInDuration > 0.0f)
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(CachedWorld.Get(), 0))
		{
			PC->PlayerCameraManager->StartCameraFade(
				1.0f, 0.0f, Moment.FadeInDuration, FLinearColor::Black, false, false);
		}
	}

	// Show subtitle
	if (!Moment.SubtitleText.IsEmpty())
	{
		ShowSubtitle(Moment.SubtitleText);
	}

	OnMomentStarted.Broadcast(Moment.MomentTag);

	// Play Level Sequence if available
	ULevelSequence* LoadedSequence = Moment.Sequence.LoadSynchronous();
	if (LoadedSequence && CachedWorld.IsValid())
	{
		FMovieSceneSequencePlaybackSettings PlaybackSettings;
		PlaybackSettings.bAutoPlay = true;
		PlaybackSettings.bPauseAtEnd = false;

		ActiveSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
			CachedWorld.Get(),
			LoadedSequence,
			PlaybackSettings,
			ActiveSequenceActor
		);

		if (ActiveSequencePlayer)
		{
			ActiveSequencePlayer->OnFinished.AddDynamic(this, &UGINarrativeDirector::OnSequenceFinished);
			ActiveSequencePlayer->Play();
		}
	}
	else if (Moment.Duration > 0.0f)
	{
		// Duration-based moment with no sequence (e.g., holding on a scene)
		CachedWorld->GetTimerManager().SetTimer(
			DurationTimer,
			this,
			&UGINarrativeDirector::OnDurationExpired,
			Moment.Duration,
			false
		);
	}
	else
	{
		// No sequence, no duration — finish immediately
		FinishMoment();
	}
}

void UGINarrativeDirector::StopCurrentMoment()
{
	if (!bIsPlaying) return;

	if (ActiveSequencePlayer)
	{
		ActiveSequencePlayer->Stop();
		ActiveSequencePlayer->OnFinished.RemoveAll(this);
		ActiveSequencePlayer = nullptr;
	}

	if (CachedWorld.IsValid())
	{
		CachedWorld->GetTimerManager().ClearTimer(DurationTimer);
	}

	FinishMoment();
}

void UGINarrativeDirector::OnSequenceFinished()
{
	if (ActiveSequencePlayer)
	{
		ActiveSequencePlayer->OnFinished.RemoveAll(this);
		ActiveSequencePlayer = nullptr;
	}
	ActiveSequenceActor = nullptr;

	FinishMoment();
}

void UGINarrativeDirector::OnDurationExpired()
{
	FinishMoment();
}

void UGINarrativeDirector::FinishMoment()
{
	// Fade out if requested
	if (ActiveMoment.FadeOutDuration > 0.0f)
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(CachedWorld.Get(), 0))
		{
			PC->PlayerCameraManager->StartCameraFade(
				0.0f, 1.0f, ActiveMoment.FadeOutDuration, FLinearColor::Black, false, true);
		}
	}

	// Hide subtitle
	HideSubtitle();

	// Re-enable player input
	if (ActiveMoment.bDisablePlayerInput)
	{
		SetPlayerInputEnabled(true);
	}

	FName EndedTag = ActiveMoment.MomentTag;
	bIsPlaying = false;
	OnMomentEnded.Broadcast(EndedTag);
}

// ============================================================================
// CAMERA
// ============================================================================

void UGINarrativeDirector::BlendToCamera(ACameraActor* TargetCamera, float BlendTime)
{
	if (!TargetCamera || !CachedWorld.IsValid()) return;

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(CachedWorld.Get(), 0))
	{
		PC->SetViewTargetWithBlend(TargetCamera, BlendTime);
	}
}

void UGINarrativeDirector::ReturnCameraToPlayer(float BlendTime)
{
	if (!CachedWorld.IsValid()) return;

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(CachedWorld.Get(), 0))
	{
		if (APawn* PlayerPawn = PC->GetPawn())
		{
			PC->SetViewTargetWithBlend(PlayerPawn, BlendTime);
		}
	}
}

// ============================================================================
// SUBTITLES
// ============================================================================

void UGINarrativeDirector::ShowSubtitle(const FText& Text)
{
	if (!CachedWorld.IsValid()) return;

	RemoveSubtitleWidget();
	CreateSubtitleWidget(Text);
}

void UGINarrativeDirector::HideSubtitle()
{
	RemoveSubtitleWidget();
}

void UGINarrativeDirector::CreateSubtitleWidget(const FText& Text)
{
	if (!CachedWorld.IsValid()) return;

	UGameViewportClient* ViewportClient = CachedWorld->GetGameViewport();
	if (!ViewportClient) return;

	SubtitleWidget =
		SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(40.0f, 0.0f, 40.0f, 80.0f))
		[
			SNew(STextBlock)
			.Text(Text)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FSlateColor(FLinearColor::White))
			.ShadowOffset(FVector2D(1.0f, 1.0f))
			.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f))
		];

	ViewportClient->AddViewportWidgetContent(SubtitleWidget.ToSharedRef());
}

void UGINarrativeDirector::RemoveSubtitleWidget()
{
	if (!SubtitleWidget.IsValid()) return;

	if (CachedWorld.IsValid())
	{
		if (UGameViewportClient* ViewportClient = CachedWorld->GetGameViewport())
		{
			ViewportClient->RemoveViewportWidgetContent(SubtitleWidget.ToSharedRef());
		}
	}

	SubtitleWidget.Reset();
}

// ============================================================================
// INPUT CONTROL
// ============================================================================

void UGINarrativeDirector::SetPlayerInputEnabled(bool bEnabled)
{
	if (!CachedWorld.IsValid()) return;

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(CachedWorld.Get(), 0))
	{
		if (bEnabled)
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->SetIgnoreMoveInput(false);
			PC->SetIgnoreLookInput(false);
		}
		else
		{
			PC->SetIgnoreMoveInput(true);
			PC->SetIgnoreLookInput(true);
		}
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Player/XiDongPlayerController.h"
#include "GIGoHome/Player/XiDongCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

void AXiDongPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// add input mapping contexts
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* Context : DefaultMappingContexts)
		{
			if (Context)
			{
				Subsystem->AddMappingContext(Context, 0);
			}
		}
	}
}

void AXiDongPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void AXiDongPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!InPawn)
	{
		return;
	}

	// grant the player tag so tunnels and supply caches recognise Xi Dong
	InPawn->Tags.AddUnique(PlayerPawnTag);

	// listen for pawn destruction (death) so we can notify game mode
	InPawn->OnDestroyed.AddDynamic(this, &AXiDongPlayerController::OnPawnDestroyed);
}

void AXiDongPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// Xi Dong is dead - disable input until game mode shows retry screen
	DisableInput(this);
}

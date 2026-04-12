// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Concrete C++ Player Controller Implementation

#include "GIGoHome/Player/GIAct1PlayerController.h"
#include "GIGoHome/Player/XiDongCharacter.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "InputModifiers.h"

AGIAct1PlayerController::AGIAct1PlayerController()
{
	GIGoHomeIMC = nullptr;
}

void AGIAct1PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!InPawn) return;

	// Ensure owner-chain visibility checks work for first-person-only meshes
	InPawn->SetOwner(this);

	// Tag the pawn as Player so traps, supply caches, and pickups recognise it
	InPawn->Tags.AddUnique(FName("Player"));

	// Build the IMC from the character's input actions
	BuildInputMappingContext(InPawn);
}

void AGIAct1PlayerController::BuildInputMappingContext(APawn* InPawn)
{
	if (!InPawn) return;

	// Cast up the hierarchy to read input action pointers
	AGIGOHOMECharacter* BaseChar = Cast<AGIGOHOMECharacter>(InPawn);
	AShooterCharacter* ShooterChar = Cast<AShooterCharacter>(InPawn);
	AXiDongCharacter* XiDong = Cast<AXiDongCharacter>(InPawn);

	if (!BaseChar) return;

	// Create the IMC
	GIGoHomeIMC = NewObject<UInputMappingContext>(this, TEXT("IMC_GIGoHome"));

	// --- WASD Movement ---
	if (BaseChar->MoveAction)
	{
		FEnhancedActionKeyMapping& W = GIGoHomeIMC->MapKey(BaseChar->MoveAction, EKeys::W);
		UInputModifierSwizzleAxis* SwizzleW = NewObject<UInputModifierSwizzleAxis>(GIGoHomeIMC);
		SwizzleW->Order = EInputAxisSwizzle::YXZ;
		W.Modifiers.Add(SwizzleW);

		FEnhancedActionKeyMapping& S = GIGoHomeIMC->MapKey(BaseChar->MoveAction, EKeys::S);
		UInputModifierNegate* NegS = NewObject<UInputModifierNegate>(GIGoHomeIMC);
		UInputModifierSwizzleAxis* SwizzleS = NewObject<UInputModifierSwizzleAxis>(GIGoHomeIMC);
		SwizzleS->Order = EInputAxisSwizzle::YXZ;
		S.Modifiers.Add(NegS);
		S.Modifiers.Add(SwizzleS);

		GIGoHomeIMC->MapKey(BaseChar->MoveAction, EKeys::D);

		FEnhancedActionKeyMapping& A = GIGoHomeIMC->MapKey(BaseChar->MoveAction, EKeys::A);
		UInputModifierNegate* NegA = NewObject<UInputModifierNegate>(GIGoHomeIMC);
		A.Modifiers.Add(NegA);
	}

	// --- Mouse Look ---
	if (BaseChar->MouseLookAction)
	{
		GIGoHomeIMC->MapKey(BaseChar->MouseLookAction, EKeys::Mouse2D);
	}

	// --- Gamepad Look ---
	if (BaseChar->LookAction)
	{
		GIGoHomeIMC->MapKey(BaseChar->LookAction, EKeys::Gamepad_RightX);
	}

	// --- Jump ---
	if (BaseChar->JumpAction)
	{
		GIGoHomeIMC->MapKey(BaseChar->JumpAction, EKeys::SpaceBar);
	}

	// --- Shooter actions ---
	if (ShooterChar)
	{
		if (ShooterChar->FireAction)
		{
			GIGoHomeIMC->MapKey(ShooterChar->FireAction, EKeys::LeftMouseButton);
		}
		if (ShooterChar->SwitchWeaponAction)
		{
			GIGoHomeIMC->MapKey(ShooterChar->SwitchWeaponAction, EKeys::MouseScrollUp);
		}
	}

	// --- XiDong-specific actions ---
	if (XiDong)
	{
		if (XiDong->CrouchAction)
		{
			GIGoHomeIMC->MapKey(XiDong->CrouchAction, EKeys::LeftControl);
			GIGoHomeIMC->MapKey(XiDong->CrouchAction, EKeys::C);
		}
		if (XiDong->SprintAction)
		{
			GIGoHomeIMC->MapKey(XiDong->SprintAction, EKeys::LeftShift);
		}
		if (XiDong->HoldBreathAction)
		{
			GIGoHomeIMC->MapKey(XiDong->HoldBreathAction, EKeys::LeftAlt);
		}
		if (XiDong->BodyDragAction)
		{
			GIGoHomeIMC->MapKey(XiDong->BodyDragAction, EKeys::G);
		}
		if (XiDong->ReloadAction)
		{
			GIGoHomeIMC->MapKey(XiDong->ReloadAction, EKeys::R);
		}
		if (XiDong->AmmoCheckAction)
		{
			GIGoHomeIMC->MapKey(XiDong->AmmoCheckAction, EKeys::T);
		}
		if (XiDong->PlaceTrapAction)
		{
			GIGoHomeIMC->MapKey(XiDong->PlaceTrapAction, EKeys::V);
		}
		if (XiDong->InteractAction)
		{
			GIGoHomeIMC->MapKey(XiDong->InteractAction, EKeys::E);
		}
		if (XiDong->OrderHoldAction)
		{
			GIGoHomeIMC->MapKey(XiDong->OrderHoldAction, EKeys::One);
		}
		if (XiDong->OrderAdvanceAction)
		{
			GIGoHomeIMC->MapKey(XiDong->OrderAdvanceAction, EKeys::Two);
		}
		if (XiDong->OrderFlankAction)
		{
			GIGoHomeIMC->MapKey(XiDong->OrderFlankAction, EKeys::Three);
		}
		if (XiDong->OrderFallBackAction)
		{
			GIGoHomeIMC->MapKey(XiDong->OrderFallBackAction, EKeys::Four);
		}
	}

	// Add the IMC to the Enhanced Input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(GIGoHomeIMC, 0);
	}
}

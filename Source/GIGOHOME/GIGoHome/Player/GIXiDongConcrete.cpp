// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Concrete Player Character Implementation

#include "GIGoHome/Player/GIXiDongConcrete.h"
#include "GIGoHome/Weapons/GIWeaponClasses.h"
#include "GIGoHome/Traps/PunjiPitTrap.h"
#include "GIGoHome/GIGoHomeSliceGameMode.h"
#include "GIGoHome/Campaign/GIMissionGameMode.h"
#include "GIGoHome/AI/GIEnemyAnimInstance.h"
#include "GIGoHome/Player/GIPlayerAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "InputAction.h"
#include "Animation/AnimInstance.h"

AGIXiDongConcrete::AGIXiDongConcrete()
{
	// Set the starting weapon to our concrete C++ rifle
	StartingWeaponClass = AGIRifleWeapon::StaticClass();

	// Starting trap inventory
	SelectedTrapClass = APunjiPitTrap::StaticClass();
	TrapCount = 3;

	// Movement speeds per GDD
	WalkSpeed = 400.0f;
	SprintSpeed = 700.0f;

	// Health - lower to make combat feel dangerous (per GDD)
	MaxHP = 100.0f;
	CurrentHP = 100.0f;

	// Team
	TeamByte = 0;

	// Capsule
	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Enable crouching
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->bCanWalkOffLedges = true;

	// Load third person mesh - SKM_Manny_Simple is the correct asset in this project
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> BodyMeshFinder(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple")
	);
	if (BodyMeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(BodyMeshFinder.Object);
		GetMesh()->SetRelativeLocationAndRotation(
			FVector(0.0f, 0.0f, -96.0f),
			FRotator(0.0f, -90.0f, 0.0f)
		);
	}
	// Load first person arms mesh - same mesh, owneronly visibility handled by base class
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> ArmsMeshFinder(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple")
	);
	if (ArmsMeshFinder.Succeeded())
	{
		GetFirstPersonMesh()->SetSkeletalMesh(ArmsMeshFinder.Object);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> MannyAnimBP(
		TEXT("/Game/UltraComponents/DemoScene/ThirdPerson/Characters/Mannequins/Animations/ABP_Manny")
	);
	if (MannyAnimBP.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(MannyAnimBP.Class);
		GetFirstPersonMesh()->SetAnimInstanceClass(MannyAnimBP.Class);
	}

	// --- Create Input Actions in C++ ---
	// Base character actions (AGIGOHOMECharacter)
	JumpAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Jump"));
	JumpAction->ValueType = EInputActionValueType::Boolean;

	MoveAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Move"));
	MoveAction->ValueType = EInputActionValueType::Axis2D;

	LookAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Look"));
	LookAction->ValueType = EInputActionValueType::Axis2D;

	MouseLookAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_MouseLook"));
	MouseLookAction->ValueType = EInputActionValueType::Axis2D;

	// Shooter character actions (AShooterCharacter)
	FireAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Fire"));
	FireAction->ValueType = EInputActionValueType::Boolean;

	SwitchWeaponAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_SwitchWeapon"));
	SwitchWeaponAction->ValueType = EInputActionValueType::Boolean;

	// XiDong character actions (AXiDongCharacter)
	CrouchAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Crouch"));
	CrouchAction->ValueType = EInputActionValueType::Boolean;

	SprintAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Sprint"));
	SprintAction->ValueType = EInputActionValueType::Boolean;

	HoldBreathAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_HoldBreath"));
	HoldBreathAction->ValueType = EInputActionValueType::Boolean;

	BodyDragAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_BodyDrag"));
	BodyDragAction->ValueType = EInputActionValueType::Boolean;

	ReloadAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Reload"));
	ReloadAction->ValueType = EInputActionValueType::Boolean;

	AmmoCheckAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_AmmoCheck"));
	AmmoCheckAction->ValueType = EInputActionValueType::Boolean;

	PlaceTrapAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_PlaceTrap"));
	PlaceTrapAction->ValueType = EInputActionValueType::Boolean;

	InteractAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Interact"));
	InteractAction->ValueType = EInputActionValueType::Boolean;

	OrderHoldAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_OrderHold"));
	OrderHoldAction->ValueType = EInputActionValueType::Boolean;

	OrderAdvanceAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_OrderAdvance"));
	OrderAdvanceAction->ValueType = EInputActionValueType::Boolean;

	OrderFlankAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_OrderFlank"));
	OrderFlankAction->ValueType = EInputActionValueType::Boolean;

	OrderFallBackAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_OrderFallBack"));
	OrderFallBackAction->ValueType = EInputActionValueType::Boolean;
}

void AGIXiDongConcrete::BeginPlay()
{
	Super::BeginPlay();

	USkeletalMeshComponent* BodyMesh = GetMesh();
	USkeletalMeshComponent* FirstPersonArmsMesh = GetFirstPersonMesh();

	if (!BodyMesh || !FirstPersonArmsMesh)
	{
		return;
	}

	// Enforce animation blueprint mode for both meshes.
	BodyMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	FirstPersonArmsMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);

	// Prefer Manny ABP if present in project.
	UClass* MannyAnimClass = LoadClass<UAnimInstance>(
		nullptr,
		TEXT("/Game/UltraComponents/DemoScene/ThirdPerson/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny_C")
	);

	if (MannyAnimClass)
	{
		BodyMesh->SetAnimInstanceClass(MannyAnimClass);
		FirstPersonArmsMesh->SetAnimInstanceClass(MannyAnimClass);
	}
	else
	{
		// Hard fallback so animations are never completely missing in test builds.
		BodyMesh->SetAnimInstanceClass(UGIPlayerAnimInstance::StaticClass());
		FirstPersonArmsMesh->SetAnimInstanceClass(UGIPlayerAnimInstance::StaticClass());
	}
}

void AGIXiDongConcrete::DoAmmoCheck()
{
	if (IsDead() || !CurrentWeapon) return;

	const int32 Current = CurrentWeapon->GetBulletCount();
	const int32 Mag = CurrentWeapon->GetMagazineSize();

	// Show ammo check via Slate HUD (pure C++)
	if (AGIGoHomeSliceGameMode* GM = Cast<AGIGoHomeSliceGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->ShowAmmoCheck(Current, Mag);
	}
   else if (AGIMissionGameMode* MissionGM = Cast<AGIMissionGameMode>(GetWorld()->GetAuthGameMode()))
	{
		MissionGM->ShowAmmoCheck(Current, Mag);
	}
}

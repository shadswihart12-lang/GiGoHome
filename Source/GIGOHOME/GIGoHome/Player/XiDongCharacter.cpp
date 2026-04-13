// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Player/XiDongCharacter.h"
#include "GIGoHome/Player/Components/StealthComponent.h"
#include "GIGoHome/Player/Components/BodyDragComponent.h"
#include "GIGoHome/Player/Components/SquadCommandComponent.h"
#include "GIGoHome/Player/Components/HealthFeedbackComponent.h"
#include "GIGoHome/Tunnels/TunnelSupplyCache.h"
#include "GIGoHome/Campaign/Actors/GICampaignNPC.h"
#include "GIGoHome/GIGoHomeGameMode.h"
#include "GIGoHome/GIGoHomeSliceGameMode.h"
#include "GIGoHome/Campaign/GIMissionGameMode.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "NavigationInvokerComponent.h"

AXiDongCharacter::AXiDongCharacter()
{
	StealthComponent = CreateDefaultSubobject<UStealthComponent>(TEXT("StealthComponent"));
	BodyDragComponent = CreateDefaultSubobject<UBodyDragComponent>(TEXT("BodyDragComponent"));
	SquadCommandComponent = CreateDefaultSubobject<USquadCommandComponent>(TEXT("SquadCommandComponent"));
	HealthFeedbackComponent = CreateDefaultSubobject<UHealthFeedbackComponent>(TEXT("HealthFeedbackComponent"));

	// Generate nav mesh tiles around the player (config has bGenerateNavigationOnlyAroundNavigationInvokers=True)
	NavInvoker = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavInvoker"));
	NavInvoker->SetGenerationRadii(3000.f, 2000.f);
}

void AXiDongCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = WalkSpeed * 0.4f;

	Tags.Add(FName("Player"));

	if (StartingWeaponClass)
	{
		AddWeaponClass(StartingWeaponClass);
	}
}

void AXiDongCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (CrouchAction)
		{
			EIC->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AXiDongCharacter::DoCrouchToggle);
		}
		if (SprintAction)
		{
			EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &AXiDongCharacter::DoSprintStart);
			EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &AXiDongCharacter::DoSprintEnd);
		}
		if (HoldBreathAction)
		{
			EIC->BindAction(HoldBreathAction, ETriggerEvent::Started, this, &AXiDongCharacter::DoHoldBreathStart);
			EIC->BindAction(HoldBreathAction, ETriggerEvent::Completed, this, &AXiDongCharacter::DoHoldBreathEnd);
		}
		if (BodyDragAction)
		{
			EIC->BindAction(BodyDragAction, ETriggerEvent::Triggered, this, &AXiDongCharacter::DoBodyDragToggle);
		}
		if (ReloadAction)
		{
			EIC->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AXiDongCharacter::DoReload);
		}
		if (AmmoCheckAction)
		{
			EIC->BindAction(AmmoCheckAction, ETriggerEvent::Triggered, this, &AXiDongCharacter::DoAmmoCheck);
		}
		if (PlaceTrapAction)
		{
			EIC->BindAction(PlaceTrapAction, ETriggerEvent::Triggered, this, &AXiDongCharacter::DoPlaceTrap);
		}
		if (OrderHoldAction)
		{
			EIC->BindAction(OrderHoldAction, ETriggerEvent::Triggered, this, &AXiDongCharacter::DoOrderHold);
		}
		if (OrderAdvanceAction)
		{
			EIC->BindAction(OrderAdvanceAction, ETriggerEvent::Triggered, this, &AXiDongCharacter::DoOrderAdvance);
		}
		if (OrderFlankAction)
		{
			EIC->BindAction(OrderFlankAction, ETriggerEvent::Triggered, this, &AXiDongCharacter::DoOrderFlank);
		}
		if (OrderFallBackAction)
		{
			EIC->BindAction(OrderFallBackAction, ETriggerEvent::Triggered, this, &AXiDongCharacter::DoOrderFallBack);
		}
		if (InteractAction)
		{
			EIC->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AXiDongCharacter::DoInteract);
		}
	}
}

void AXiDongCharacter::DoCrouchToggle()
{
	if (IsDead()) return;
	bIsCrouched ? UnCrouch() : Crouch();
}

void AXiDongCharacter::DoSprintStart()
{
	if (IsDead() || bIsCrouched) return;
	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void AXiDongCharacter::DoSprintEnd()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AXiDongCharacter::DoHoldBreathStart()
{
	if (!IsDead() && StealthComponent)
	{
		StealthComponent->StartHoldBreath();
	}
}

void AXiDongCharacter::DoHoldBreathEnd()
{
	if (StealthComponent)
	{
		StealthComponent->StopHoldBreath();
	}
}

void AXiDongCharacter::DoBodyDragToggle()
{
	if (IsDead() || !BodyDragComponent) return;

	if (BodyDragComponent->IsDragging())
	{
		BodyDragComponent->StopDrag();
	}
	else
	{
		BodyDragComponent->StartDrag();
	}
}

void AXiDongCharacter::DoReload()
{
	if (IsDead() || !CurrentWeapon) return;
	CurrentWeapon->Reload();
}

void AXiDongCharacter::DoAmmoCheck()
{
	if (IsDead() || !CurrentWeapon) return;

	const int32 Current = CurrentWeapon->GetBulletCount();
	const int32 Mag = CurrentWeapon->GetMagazineSize();

	// Notify game mode to show HUD
	if (AGIGoHomeSliceGameMode* GM = Cast<AGIGoHomeSliceGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->ShowAmmoCheck(Current, Mag);
	}
	else if (AGIMissionGameMode* MissionGM = Cast<AGIMissionGameMode>(GetWorld()->GetAuthGameMode()))
	{
		MissionGM->ShowAmmoCheck(Current, Mag);
	}

	// Fire virtual event for any additional effects
	OnAmmoCheckFeedback(Current, Mag);
}

void AXiDongCharacter::DoPlaceTrap()
{
	if (IsDead() || !SelectedTrapClass || TrapCount <= 0) return;

	const FVector PlaceLocation = GetActorLocation()
		+ (GetActorForwardVector() * TrapPlacementDistance)
		- FVector(0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - 10.0f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetWorld()->SpawnActor<ATrapBase>(SelectedTrapClass, PlaceLocation, GetActorRotation(), SpawnParams);
	--TrapCount;

	// Notify game mode of trap placement
	if (AGIGoHomeSliceGameMode* GM = Cast<AGIGoHomeSliceGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->NotifyTrapPlaced();
	}
}

void AXiDongCharacter::DoOrderHold()
{
	if (SquadCommandComponent) SquadCommandComponent->IssueOrder(ESquadOrder::HoldPosition);
}

void AXiDongCharacter::DoOrderAdvance()
{
	if (SquadCommandComponent) SquadCommandComponent->IssueOrder(ESquadOrder::Advance);
}

void AXiDongCharacter::DoOrderFlank()
{
	if (SquadCommandComponent) SquadCommandComponent->IssueOrder(ESquadOrder::Flank);
}

void AXiDongCharacter::DoOrderFallBack()
{
	if (SquadCommandComponent) SquadCommandComponent->IssueOrder(ESquadOrder::FallBack);
}

float AXiDongCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (BodyDragComponent && BodyDragComponent->IsDragging())
	{
		BodyDragComponent->StopDrag();
	}

	const float Result = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	// Per GDD: Health shown through screen effects and breathing
	if (HealthFeedbackComponent && MaxHP > 0.0f)
	{
		const float HealthPercent = CurrentHP / MaxHP;
		HealthFeedbackComponent->OnDamageTaken(HealthPercent);
	}

	if (IsDead())
	{
		GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);

		if (AGIGoHomeGameMode* GM = Cast<AGIGoHomeGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GM->OnPlayerDeath();
		}
	}

	return Result;
}

void AXiDongCharacter::DoInteract()
{
	if (IsDead()) return;

	// --- Priority 1: Tunnel supply cache interaction ---
	// Supply cache takes precedence so grabbing ammo never accidentally triggers NPCs.
	if (NearbySupplyCache && NearbySupplyCache->HasSupplies())
	{
		// Take ammo first
		const int32 Reloads = NearbySupplyCache->TakeAmmo();
		if (Reloads > 0 && CurrentWeapon)
		{
			// Add ammo reloads to current weapon
			for (int32 i = 0; i < Reloads; i++)
			{
				CurrentWeapon->Reload();
			}
		}

		// Take medical supplies if we're not at full health
		if (CurrentHP < MaxHP)
		{
			const float HealAmount = NearbySupplyCache->TakeMedicalSupplies();
			if (HealAmount > 0.0f)
			{
				CurrentHP = FMath::Min(CurrentHP + HealAmount, MaxHP);

				// Update health feedback
				if (HealthFeedbackComponent && MaxHP > 0.0f)
				{
					HealthFeedbackComponent->OnDamageTaken(CurrentHP / MaxHP);
				}
			}
		}
		return;
	}

	// --- Priority 2: Campaign NPC interaction (Missions 4 & 5) ---
	// Finds the nearest in-range NPC (overlap detection is on the NPC side).
	// Mission 4: Tyler Greg in the drainage ditch — spare or already killed.
	// Mission 5: Briggs hooded and bound — securing him starts the escort chain.
	for (TObjectIterator<AGICampaignNPC> It; It; ++It)
	{
		// Validate world, alive state, and player proximity
		if (It->GetWorld() != GetWorld() || It->IsDead() || !It->IsPlayerInRange())
		{
			continue;
		}

		It->InteractWith(this);
		break; // One interaction per key press — prevents double-firing
	}
}

void AXiDongCharacter::SetNearbySupplyCache(ATunnelSupplyCache* Cache)
{
	NearbySupplyCache = Cache;
}

void AXiDongCharacter::ClearNearbySupplyCache(ATunnelSupplyCache* Cache)
{
	if (NearbySupplyCache == Cache)
	{
		NearbySupplyCache = nullptr;
	}
}

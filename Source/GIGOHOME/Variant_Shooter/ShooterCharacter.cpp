// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterCharacter.h"
#include "ShooterWeapon.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "ShooterGameMode.h"
#include "GIGoHome/Campaign/GIMissionGameMode.h"
#include "GIGoHome/GIGoHomeSliceGameMode.h"

AShooterCharacter::AShooterCharacter()
{
	PawnNoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Pawn Noise Emitter"));
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;
	OnDamaged.Broadcast(1.0f);
}

void AShooterCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AShooterCharacter::DoStartFiring);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterCharacter::DoStopFiring);
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::DoSwitchWeapon);
	}
}

float AShooterCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (CurrentHP <= 0.0f) return 0.0f;

	CurrentHP -= Damage;

	if (CurrentHP <= 0.0f)
	{
		Die();
	}

	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));
	return Damage;
}

void AShooterCharacter::DoAim(float Yaw, float Pitch)
{
	if (!IsDead()) Super::DoAim(Yaw, Pitch);
}

void AShooterCharacter::DoMove(float Right, float Forward)
{
	if (!IsDead()) Super::DoMove(Right, Forward);
}

void AShooterCharacter::DoJumpStart()
{
	if (!IsDead()) Super::DoJumpStart();
}

void AShooterCharacter::DoJumpEnd()
{
	if (!IsDead()) Super::DoJumpEnd();
}

void AShooterCharacter::DoStartFiring()
{
	if (CurrentWeapon && !IsDead()) CurrentWeapon->StartFiring();
}

void AShooterCharacter::DoStopFiring()
{
	if (CurrentWeapon && !IsDead()) CurrentWeapon->StopFiring();
}

void AShooterCharacter::DoSwitchWeapon()
{
	if (OwnedWeapons.Num() > 1 && !IsDead())
	{
		CurrentWeapon->DeactivateWeapon();
		int32 WeaponIndex = OwnedWeapons.Find(CurrentWeapon);
		WeaponIndex = (WeaponIndex == OwnedWeapons.Num() - 1) ? 0 : WeaponIndex + 1;
		CurrentWeapon = OwnedWeapons[WeaponIndex];
		CurrentWeapon->ActivateWeapon();
	}
}

void AShooterCharacter::AttachWeaponMeshes(AShooterWeapon* Weapon)
{
   if (!Weapon)
	{
		return;
	}

	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);
	Weapon->AttachToActor(this, AttachmentRule);

	USkeletalMeshComponent* WeaponFPMesh = Weapon->GetFirstPersonMesh();
	USkeletalMeshComponent* WeaponTPMesh = Weapon->GetThirdPersonMesh();
	USkeletalMeshComponent* FirstPersonMeshComp = GetFirstPersonMesh();
	USkeletalMeshComponent* ThirdPersonMeshComp = GetMesh();

	if (WeaponFPMesh && FirstPersonMeshComp)
	{
		if (FirstPersonMeshComp->DoesSocketExist(FirstPersonWeaponSocket))
		{
			WeaponFPMesh->AttachToComponent(FirstPersonMeshComp, AttachmentRule, FirstPersonWeaponSocket);
		}
		else
		{
			WeaponFPMesh->AttachToComponent(GetFirstPersonCameraComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			WeaponFPMesh->SetRelativeLocation(FirstPersonWeaponFallbackLocation);
			WeaponFPMesh->SetRelativeRotation(FirstPersonWeaponFallbackRotation);
		}
	}

	if (WeaponTPMesh && ThirdPersonMeshComp)
	{
		if (ThirdPersonMeshComp->DoesSocketExist(ThirdPersonWeaponSocket))
		{
			WeaponTPMesh->AttachToComponent(ThirdPersonMeshComp, AttachmentRule, ThirdPersonWeaponSocket);
		}
		else
		{
			WeaponTPMesh->AttachToComponent(ThirdPersonMeshComp, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}

	// First-person owner should see only FP weapon mesh.
    if (WeaponFPMesh)
	{
		WeaponFPMesh->SetOnlyOwnerSee(true);
		WeaponFPMesh->SetOwnerNoSee(false);
		WeaponFPMesh->SetHiddenInGame(false, true);
		WeaponFPMesh->SetVisibility(true, true);
	}

	if (WeaponTPMesh)
	{
		WeaponTPMesh->SetOnlyOwnerSee(false);
		WeaponTPMesh->SetOwnerNoSee(true);
		WeaponTPMesh->SetHiddenInGame(false, true);
		WeaponTPMesh->SetVisibility(true, true);
	}
}

void AShooterCharacter::PlayFiringMontage(UAnimMontage* Montage) {}

void AShooterCharacter::AddWeaponRecoil(float Recoil)
{
	AddControllerPitchInput(Recoil);
}

void AShooterCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	OnBulletCountUpdated.Broadcast(MagazineSize, CurrentAmmo);

	if (AGIMissionGameMode* MissionGM = Cast<AGIMissionGameMode>(GetWorld()->GetAuthGameMode()))
	{
		MissionGM->ShowAmmoCheck(CurrentAmmo, MagazineSize);
	}
	else if (AGIGoHomeSliceGameMode* SliceGM = Cast<AGIGoHomeSliceGameMode>(GetWorld()->GetAuthGameMode()))
	{
		SliceGM->ShowAmmoCheck(CurrentAmmo, MagazineSize);
	}
}

FVector AShooterCharacter::GetWeaponTargetLocation()
{
	FHitResult OutHit;
	const FVector Start = GetFirstPersonCameraComponent()->GetComponentLocation();
	const FVector End = Start + (GetFirstPersonCameraComponent()->GetForwardVector() * MaxAimDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}

void AShooterCharacter::AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass)
{
	AShooterWeapon* OwnedWeapon = FindWeaponOfType(WeaponClass);

	if (!OwnedWeapon)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

		AShooterWeapon* AddedWeapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);

		if (AddedWeapon)
		{
			OwnedWeapons.Add(AddedWeapon);
			if (CurrentWeapon) CurrentWeapon->DeactivateWeapon();
			CurrentWeapon = AddedWeapon;
			CurrentWeapon->ActivateWeapon();
		}
	}
}

void AShooterCharacter::OnWeaponActivated(AShooterWeapon* Weapon)
{
	OnBulletCountUpdated.Broadcast(Weapon->GetMagazineSize(), Weapon->GetBulletCount());

	if (Weapon->GetFirstPersonAnimInstanceClass())
	{
		GetFirstPersonMesh()->SetAnimInstanceClass(Weapon->GetFirstPersonAnimInstanceClass());
	}
	if (Weapon->GetThirdPersonAnimInstanceClass())
	{
		GetMesh()->SetAnimInstanceClass(Weapon->GetThirdPersonAnimInstanceClass());
	}

	UpdateWeaponHUD(Weapon->GetBulletCount(), Weapon->GetMagazineSize());
}

void AShooterCharacter::OnWeaponDeactivated(AShooterWeapon* Weapon) {}
void AShooterCharacter::OnSemiWeaponRefire() {}

AShooterWeapon* AShooterCharacter::FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const
{
	for (AShooterWeapon* Weapon : OwnedWeapons)
	{
		if (Weapon->IsA(WeaponClass)) return Weapon;
	}
	return nullptr;
}

void AShooterCharacter::Die()
{
	// stop firing
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->StopFiring();
	}

	// grant death tag
	Tags.Add(DeathTag);

	// disable capsule collision so we fall properly
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// stop movement component
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetComponentTickEnabled(false);

	// ragdoll the third person mesh
	GetMesh()->SetCollisionProfileName(FName("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetPhysicsBlendWeight(1.0f);

	// hide first person elements
	GetFirstPersonMesh()->SetHiddenInGame(true);

	// disable input
	DisableInput(nullptr);

	OnBulletCountUpdated.Broadcast(0, 0);
	OnPlayerDeath.Broadcast();

	// wait 5 seconds before destroying so ragdoll is visible
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AShooterCharacter::OnRespawn, 5.0f, false);
}

void AShooterCharacter::OnRespawn()
{
	Destroy();
}

bool AShooterCharacter::IsDead() const
{
	return CurrentHP <= 0.0f;
}

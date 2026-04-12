// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "GIGoHome/Traps/TrapBase.h"
#include "XiDongCharacter.generated.h"

class UStealthComponent;
class UBodyDragComponent;
class USquadCommandComponent;
class UHealthFeedbackComponent;
class UInputAction;
class AShooterWeapon;
class ATunnelSupplyCache;

UCLASS(abstract)
class GIGOHOME_API AXiDongCharacter : public AShooterCharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UStealthComponent* StealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UBodyDragComponent* BodyDragComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USquadCommandComponent* SquadCommandComponent;

	/** Per GDD: Health shown through screen effects and breathing, not a health bar */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UHealthFeedbackComponent* HealthFeedbackComponent;

public:

	// --- Input Actions ---
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* HoldBreathAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* BodyDragAction;

	/** Reload weapon - mapped to R */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ReloadAction;

	/** Manual ammo check - mapped to a different key */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* AmmoCheckAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* PlaceTrapAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* OrderHoldAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* OrderAdvanceAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* OrderFlankAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* OrderFallBackAction;

	/** Interact with tunnel supply caches */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* InteractAction;

protected:

	// --- Movement ---
	UPROPERTY(EditAnywhere, Category="Movement", meta=(ClampMin=0.0f, Units="cm/s"))
	float WalkSpeed = 400.0f;

	UPROPERTY(EditAnywhere, Category="Movement", meta=(ClampMin=0.0f, Units="cm/s"))
	float SprintSpeed = 700.0f;

	bool bIsSprinting = false;

	// --- Starting Weapon ---
	UPROPERTY(EditAnywhere, Category="Weapons")
	TSubclassOf<AShooterWeapon> StartingWeaponClass;

	// --- Trap inventory ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Traps")
	TSubclassOf<ATrapBase> SelectedTrapClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Traps", meta=(ClampMin=0))
	int32 TrapCount = 0;

	UPROPERTY(EditAnywhere, Category="Traps", meta=(ClampMin=0.0f, Units="cm"))
	float TrapPlacementDistance = 120.0f;

	// --- Interaction ---
	/** Currently overlapping supply cache (if any) */
	UPROPERTY()
	ATunnelSupplyCache* NearbySupplyCache = nullptr;

public:

	AXiDongCharacter();

protected:

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoCrouchToggle();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoSprintStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoSprintEnd();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoHoldBreathStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoHoldBreathEnd();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoBodyDragToggle();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoReload();

	virtual void DoAmmoCheck() override;

	virtual void DoPlaceTrap() override;

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoOrderHold();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoOrderAdvance();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoOrderFlank();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoOrderFallBack();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoInteract();

	/** Called when player enters a supply cache range */
	void SetNearbySupplyCache(ATunnelSupplyCache* Cache);

	/** Called when player leaves supply cache range */
	void ClearNearbySupplyCache(ATunnelSupplyCache* Cache);

	UStealthComponent* GetStealthComponent() const { return StealthComponent; }
	USquadCommandComponent* GetSquadCommandComponent() const { return SquadCommandComponent; }

	UFUNCTION(BlueprintPure, Category="Movement")
	bool IsSprinting() const { return bIsSprinting; }

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:

	/** Called during ammo check - override for custom feedback */
	virtual void OnAmmoCheckFeedback(int32 CurrentAmmo, int32 MagazineSize) {}
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterWeaponHolder.h"
#include "Animation/AnimInstance.h"
#include "ShooterWeapon.generated.h"

class IShooterWeaponHolder;
class AShooterProjectile;
class USkeletalMeshComponent;
class UAnimMontage;
class UAnimInstance;

UCLASS(abstract)
class GIGOHOME_API AShooterWeapon : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ThirdPersonMesh;

protected:

	IShooterWeaponHolder* WeaponOwner;

	UPROPERTY(EditAnywhere, Category="Ammo")
	TSubclassOf<AShooterProjectile> ProjectileClass;

	/** Number of bullets per magazine */
	UPROPERTY(EditAnywhere, Category="Ammo", meta = (ClampMin = 0, ClampMax = 100))
	int32 MagazineSize = 30;

	/** Total reserve ammo (magazines * bullets) */
	UPROPERTY(EditAnywhere, Category="Ammo", meta = (ClampMin = 0))
	int32 ReserveAmmo = 120;

	/** Current bullets in magazine */
	int32 CurrentBullets = 0;

	/** Time to reload in seconds */
	UPROPERTY(EditAnywhere, Category="Ammo", meta = (ClampMin = 0.1f, Units = "s"))
	float ReloadTime = 2.5f;

	/** Whether the weapon is currently reloading */
	bool bIsReloading = false;

	/** Reload timer */
	FTimerHandle ReloadTimer;
	
	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* FiringMontage;

	/** Reload animation montage - plays on first person arms */
	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* ReloadMontage;

	/** Reload animation montage - plays on third person mesh */
	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* ReloadMontage3P;

	UPROPERTY(EditAnywhere, Category="Animation")
	TSubclassOf<UAnimInstance> FirstPersonAnimInstanceClass;

	UPROPERTY(EditAnywhere, Category="Animation")
	TSubclassOf<UAnimInstance> ThirdPersonAnimInstanceClass;

	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 90, Units = "Degrees"))
	float AimVariance = 0.0f;

	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 100))
	float FiringRecoil = 0.0f;

	UPROPERTY(EditAnywhere, Category="Aim")
	FName MuzzleSocketName;

	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float MuzzleOffset = 10.0f;

	UPROPERTY(EditAnywhere, Category="Refire")
	bool bFullAuto = false;

	UPROPERTY(EditAnywhere, Category="Refire", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float RefireRate = 0.5f;

	float TimeOfLastShot = 0.0f;
	bool bIsFiring = false;
	FTimerHandle RefireTimer;

	TObjectPtr<APawn> PawnOwner;

	UPROPERTY(EditAnywhere, Category="Perception", meta = (ClampMin = 0, ClampMax = 100))
	float ShotLoudness = 1.0f;

	UPROPERTY(EditAnywhere, Category="Perception", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float ShotNoiseRange = 3000.0f;

	UPROPERTY(EditAnywhere, Category="Perception")
	FName ShotNoiseTag = FName("Shot");

	/** Sound to play when firing */
	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* FireSound = nullptr;

	/** Sound to play when dry firing (empty magazine) */
	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* DryFireSound = nullptr;

	/** Sound to play when reloading */
	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* ReloadSound = nullptr;

	/** Sound volume multiplier */
	UPROPERTY(EditAnywhere, Category="Audio", meta = (ClampMin = 0.0f, ClampMax = 5.0f))
	float FireSoundVolume = 1.0f;

public:	

	AShooterWeapon();

protected:
	
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

protected:

	UFUNCTION()
	void OnOwnerDestroyed(AActor* DestroyedActor);

public:

	void ActivateWeapon();
	void DeactivateWeapon();
	void StartFiring();
	void StopFiring();

	/** Reload the weapon - called by player input */
	void Reload();

	/** Add ammo directly to reserve (for pickups) */
	void AddReserveAmmo(int32 Amount);

protected:

	virtual void Fire();
	void FireCooldownExpired();
	virtual void FireProjectile(const FVector& TargetLocation);
	FTransform CalculateProjectileSpawnTransform(const FVector& TargetLocation) const;

	/** Called when reload timer finishes */
	void FinishReload();

	/** Play reload montage on owner's meshes */
	void PlayReloadMontage();

public:

	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; };

	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetThirdPersonMesh() const { return ThirdPersonMesh; };

	const TSubclassOf<UAnimInstance>& GetFirstPersonAnimInstanceClass() const;
	const TSubclassOf<UAnimInstance>& GetThirdPersonAnimInstanceClass() const;

	int32 GetMagazineSize() const { return MagazineSize; };
	int32 GetBulletCount() const { return CurrentBullets; }
	int32 GetReserveAmmo() const { return ReserveAmmo; }
	bool IsReloading() const { return bIsReloading; }
};

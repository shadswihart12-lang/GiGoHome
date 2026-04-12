// Copyright Epic Games, Inc. All Rights Reserved.
// SIMPLE TEST ENEMY - No Blueprint setup needed, no NavMesh needed
// Just place in level and hit Play

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GITestEnemy.generated.h"

class AShooterWeapon;

UCLASS()
class GIGOHOME_API AGITestEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AGITestEnemy();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	TSubclassOf<AShooterWeapon> WeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float ShootRange = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float StopDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float ShootInterval = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float MaxHP = 100.0f;

	UPROPERTY()
	float CurrentHP = 100.0f;

	UPROPERTY()
	AShooterWeapon* Weapon;

	UPROPERTY()
	bool bIsDead = false;

	float ShootTimer = 0.0f;
	FTimerHandle StopShootTimer;

	void SpawnWeapon();
	void Die();
};

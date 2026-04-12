// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Radio Actor
// Destructible radio equipment used in multiple campaign missions

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GIRadioActor.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UAudioComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRadioDestroyed, AGIRadioActor*, Radio);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRadioUsed, AActor*, User);

/**
 * Destructible military radio equipment
 * Used in Mission 1 (destroy radio), Mission 3 (prevent contact), Mission 5 (firebase)
 * 
 * Can be destroyed by damage or direct interaction.
 * Emits radio chatter sound when active.
 * Broadcasts delegates when destroyed or used by an enemy.
 */
UCLASS()
class GIGOHOME_API AGIRadioActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* RadioMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UAudioComponent* RadioChatter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USphereComponent* InteractionVolume;

public:

	UPROPERTY(BlueprintAssignable, Category="Radio")
	FOnRadioDestroyed OnRadioDestroyed;

	UPROPERTY(BlueprintAssignable, Category="Radio")
	FOnRadioUsed OnRadioUsed;

protected:

	UPROPERTY(EditAnywhere, Category="Radio")
	float MaxHealth = 50.0f;

	UPROPERTY(BlueprintReadOnly, Category="Radio")
	float CurrentHealth = 50.0f;

	UPROPERTY(EditAnywhere, Category="Radio")
	bool bIsActive = true;

	UPROPERTY(BlueprintReadOnly, Category="Radio")
	bool bIsDestroyed = false;

	/** Tag identifying this radio for mission objective tracking */
	UPROPERTY(EditAnywhere, Category="Radio")
	FName RadioTag = FName("Radio");

	/** Sound to play as radio chatter */
	UPROPERTY(EditAnywhere, Category="Radio|Audio")
	USoundBase* ChatterSound;

	/** Sound to play when destroyed */
	UPROPERTY(EditAnywhere, Category="Radio|Audio")
	USoundBase* DestroySound;

public:

	AGIRadioActor();

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	/** Called by enemy NPCs attempting to use the radio (Mission 3) */
	UFUNCTION(BlueprintCallable, Category="Radio")
	void UseRadio(AActor* User);

	/** Directly destroys the radio */
	UFUNCTION(BlueprintCallable, Category="Radio")
	void DestroyRadio();

	/** Returns true if the radio is active and not destroyed */
	UFUNCTION(BlueprintPure, Category="Radio")
	bool IsRadioActive() const { return bIsActive && !bIsDestroyed; }

	FName GetRadioTag() const { return RadioTag; }

protected:

	virtual void BeginPlay() override;
};

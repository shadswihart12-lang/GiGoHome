// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StealthComponent.generated.h"

class UPawnNoiseEmitterComponent;
class UCharacterMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FStealthLevelChangedDelegate, float, NoiseLevel, float, VisibilityLevel);

/**
 * Manages stealth state for Xi Dong
 *
 * Noise: emitted periodically to AI hearing perception system via MakeNoise.
 *   Affected by movement speed (walk/crouch/sprint), surface type (via RainMaskFactor),
 *   and breath holding.
 *
 * Visibility: a 0-1 estimate for Blueprint use (e.g. screen vignette feedback).
 *   Affected by movement speed, foliage cover, and crouching.
 *   The AI sight perception is handled by the AI subsystem natively.
 */
UCLASS(ClassGroup=(GIGoHome), meta=(BlueprintSpawnableComponent))
class GIGOHOME_API UStealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Broadcast when noise or visibility levels change */
	UPROPERTY(BlueprintAssignable, Category="Stealth")
	FStealthLevelChangedDelegate OnStealthLevelChanged;

protected:

	/** How often to emit movement noise to the AI perception system (seconds) */
	UPROPERTY(EditAnywhere, Category="Stealth", meta=(ClampMin=0.05f, ClampMax=2.0f, Units="s"))
	float NoiseEmitInterval = 0.3f;

	/** Noise loudness while walking upright */
	UPROPERTY(EditAnywhere, Category="Stealth", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float WalkNoiseLoudness = 0.4f;

	/** Noise loudness while crouching */
	UPROPERTY(EditAnywhere, Category="Stealth", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float CrouchNoiseLoudness = 0.1f;

	/** Noise loudness while sprinting */
	UPROPERTY(EditAnywhere, Category="Stealth", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float SprintNoiseLoudness = 0.9f;

	/** Maximum AI hearing range for movement noise (cm) */
	UPROPERTY(EditAnywhere, Category="Stealth", meta=(ClampMin=0.0f, Units="cm"))
	float MovementNoiseRange = 1500.0f;

	/**
	 * Weather noise reduction factor set externally by a weather system
	 * 0 = no rain, 1 = heavy rain (fully masks movement noise)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stealth", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float RainMaskFactor = 0.0f;

	/** Current noise level (0=silent, 1=maximum) - readable by Blueprint for HUD feedback */
	UPROPERTY(BlueprintReadOnly, Category="Stealth")
	float CurrentNoiseLevel = 0.0f;

	/** Current visibility estimate (0=hidden, 1=fully visible) - readable by Blueprint */
	UPROPERTY(BlueprintReadOnly, Category="Stealth")
	float CurrentVisibilityLevel = 0.0f;

	/** Whether Xi Dong is holding his breath */
	bool bIsHoldingBreath = false;

	/** Whether Xi Dong is inside a foliage cover volume */
	bool bInFoliageCover = false;

	/** Timer for periodic noise emission */
	FTimerHandle NoiseEmitTimer;

	/** Cached character movement component */
	TObjectPtr<UCharacterMovementComponent> CachedMovement;

	/** Cached noise emitter component */
	TObjectPtr<UPawnNoiseEmitterComponent> CachedNoiseEmitter;

	/** Cached pawn pointer for MakeNoise calls */
	TObjectPtr<APawn> OwnerPawn;

public:

	UStealthComponent();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:

	/** Called by foliage cover volumes when the player enters or exits dense cover */
	UFUNCTION(BlueprintCallable, Category="Stealth")
	void SetInFoliageCover(bool bInCover);

	/** Xi Dong holds his breath - reduces noise emitted, steadies aim */
	UFUNCTION(BlueprintCallable, Category="Stealth")
	void StartHoldBreath();

	/** Xi Dong releases breath */
	UFUNCTION(BlueprintCallable, Category="Stealth")
	void StopHoldBreath();

	/** Returns true if currently holding breath */
	UFUNCTION(BlueprintPure, Category="Stealth")
	bool IsHoldingBreath() const { return bIsHoldingBreath; }

	/** Returns current noise level (0-1) */
	UFUNCTION(BlueprintPure, Category="Stealth")
	float GetNoiseLevel() const { return CurrentNoiseLevel; }

	/** Returns current visibility level (0-1) */
	UFUNCTION(BlueprintPure, Category="Stealth")
	float GetVisibilityLevel() const { return CurrentVisibilityLevel; }

	/** Sets the rain mask factor (called by WeatherController) */
	UFUNCTION(BlueprintCallable, Category="Stealth")
	void SetRainMaskFactor(float NewFactor) { RainMaskFactor = FMath::Clamp(NewFactor, 0.0f, 1.0f); }

protected:

	/** Periodic tick: calculates noise level and reports to AI perception system */
	void EmitMovementNoise();

	/** Calculates noise level from movement state and environment */
	float CalculateNoiseLevel() const;

	/** Calculates visibility estimate from movement, cover, and weather */
	float CalculateVisibilityLevel() const;
};

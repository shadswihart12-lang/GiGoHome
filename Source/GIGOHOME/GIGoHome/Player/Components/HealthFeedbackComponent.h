// Copyright Epic Games, Inc. All Rights Reserved.
// Per GDD: "Health shown through screen effects and Xi Dong's breathing"
// No health bar - immersive feedback only

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthFeedbackComponent.generated.h"

class UMaterialInstanceDynamic;
class UMaterialParameterCollection;
class UAudioComponent;

/**
 * Per GDD Section 8.3 HUD Philosophy:
 * "Minimal HUD - health shown through screen effects and Xi Dong's breathing"
 * 
 * This component drives all health feedback without traditional UI elements:
 * - Screen vignette intensifies as health drops
 * - Desaturation at critical health
 * - Breathing sounds become labored
 * - Heartbeat audio at near-death
 */
UCLASS(ClassGroup=(GIGoHome), meta=(BlueprintSpawnableComponent))
class GIGOHOME_API UHealthFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UHealthFeedbackComponent();

protected:

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

	/** Call when player takes damage - triggers feedback */
	void OnDamageTaken(float NewHealthPercent);

	/** Returns current health percent (0-1) */
	float GetHealthPercent() const { return CurrentHealthPercent; }

protected:

	/** MPC for driving post-process parameters */
	UPROPERTY(EditAnywhere, Category="Feedback")
	UMaterialParameterCollection* HealthMPC;

	/** Parameter names in the MPC */
	UPROPERTY(EditAnywhere, Category="Feedback")
	FName VignetteIntensityParam = FName("VignetteIntensity");

	UPROPERTY(EditAnywhere, Category="Feedback")
	FName DesaturationParam = FName("Desaturation");

	/** Health thresholds */
	UPROPERTY(EditAnywhere, Category="Feedback", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float CriticalHealthThreshold = 0.25f;

	UPROPERTY(EditAnywhere, Category="Feedback", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float LowHealthThreshold = 0.5f;

	/** Audio assets */
	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* LaboredBreathingSound;

	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* HeartbeatSound;

	UPROPERTY(EditAnywhere, Category="Audio")
	USoundBase* DamageHitSound;

private:

	float CurrentHealthPercent = 1.0f;
	float TargetVignette = 0.0f;
	float TargetDesaturation = 0.0f;

	UPROPERTY()
	UAudioComponent* BreathingAudioComp;

	UPROPERTY()
	UAudioComponent* HeartbeatAudioComp;

	void UpdateVisualFeedback(float DeltaTime);
	void UpdateAudioFeedback();
};

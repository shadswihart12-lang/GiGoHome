// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeatherController.generated.h"

class UAudioComponent;

/**
 * Vietnam jungle weather system
 * Controls rain intensity, which affects:
 * - Stealth: rain masks movement noise (StealthComponent::RainMaskFactor)
 * - Audio: rain ambience volume
 *
 * GDD: "Rain masking" - heavy rain covers footsteps
 * 
 * Note: Add Niagara rain particles in a Blueprint child if desired.
 */
UCLASS()
class GIGOHOME_API AWeatherController : public AActor
{
	GENERATED_BODY()

public:

	AWeatherController();

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Current rain intensity (0=clear, 1=monsoon) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weather", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float RainIntensity = 0.0f;

	/** Target rain intensity - smoothly interpolates toward this */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weather", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float TargetRainIntensity = 0.0f;

	/** How fast rain intensity changes */
	UPROPERTY(EditAnywhere, Category="Weather", meta=(ClampMin=0.01f))
	float RainChangeSpeed = 0.1f;

	/** Audio component for rain ambience */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UAudioComponent* RainAudio;

	/** Rain ambience sound */
	UPROPERTY(EditAnywhere, Category="Weather|Audio")
	USoundBase* RainSound = nullptr;

	/** Maximum rain audio volume at full intensity */
	UPROPERTY(EditAnywhere, Category="Weather|Audio", meta=(ClampMin=0.0f, ClampMax=2.0f))
	float MaxRainVolume = 0.8f;

	/** Enable random weather changes */
	UPROPERTY(EditAnywhere, Category="Weather|Automation")
	bool bRandomWeather = true;

	/** Minimum time between weather changes (seconds) */
	UPROPERTY(EditAnywhere, Category="Weather|Automation", meta=(ClampMin=10.0f, Units="s"))
	float MinWeatherInterval = 60.0f;

	/** Maximum time between weather changes (seconds) */
	UPROPERTY(EditAnywhere, Category="Weather|Automation", meta=(ClampMin=10.0f, Units="s"))
	float MaxWeatherInterval = 180.0f;

private:

	FTimerHandle WeatherChangeTimer;

	void ScheduleNextWeatherChange();
	void ChangeWeather();
	void UpdatePlayerStealthMask();

public:

	/** Set rain intensity immediately */
	UFUNCTION(BlueprintCallable, Category="Weather")
	void SetRainIntensity(float NewIntensity);

	/** Set target rain intensity (smooth transition) */
	UFUNCTION(BlueprintCallable, Category="Weather")
	void SetTargetRainIntensity(float NewTarget);

	/** Returns current rain intensity */
	UFUNCTION(BlueprintPure, Category="Weather")
	float GetRainIntensity() const { return RainIntensity; }

	/** Start a rain storm (intensity 0.7-1.0) */
	UFUNCTION(BlueprintCallable, Category="Weather")
	void StartStorm();

	/** Clear the rain (intensity 0) */
	UFUNCTION(BlueprintCallable, Category="Weather")
	void ClearWeather();

	/** Blueprint event: called when rain intensity changes (use to drive Niagara) */
	UFUNCTION(BlueprintImplementableEvent, Category="Weather", meta=(DisplayName="On Rain Intensity Changed"))
	void BP_OnRainIntensityChanged(float NewIntensity);
};

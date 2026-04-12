// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Weather/WeatherController.h"
#include "GIGoHome/Player/Components/StealthComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

AWeatherController::AWeatherController()
{
	PrimaryActorTick.bCanEverTick = true;

	RainAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("RainAudio"));
	RootComponent = RainAudio;
	RainAudio->bAutoActivate = false;
}

void AWeatherController::BeginPlay()
{
	Super::BeginPlay();

	// Start audio if we have a rain sound
	if (RainSound)
	{
		RainAudio->SetSound(RainSound);
		RainAudio->SetVolumeMultiplier(0.0f);
		RainAudio->Play();
	}

	// Schedule random weather if enabled
	if (bRandomWeather)
	{
		ScheduleNextWeatherChange();
	}
}

void AWeatherController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Smoothly interpolate toward target intensity
	if (!FMath::IsNearlyEqual(RainIntensity, TargetRainIntensity, 0.01f))
	{
		RainIntensity = FMath::FInterpTo(RainIntensity, TargetRainIntensity, DeltaTime, RainChangeSpeed);

		// Update audio volume
		if (RainAudio)
		{
			RainAudio->SetVolumeMultiplier(RainIntensity * MaxRainVolume);
		}

		// Update player stealth
		UpdatePlayerStealthMask();

		// Notify Blueprint (for Niagara particles, post-process, etc.)
		BP_OnRainIntensityChanged(RainIntensity);
	}
}

void AWeatherController::SetRainIntensity(float NewIntensity)
{
	RainIntensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);
	TargetRainIntensity = RainIntensity;

	if (RainAudio)
	{
		RainAudio->SetVolumeMultiplier(RainIntensity * MaxRainVolume);
	}

	UpdatePlayerStealthMask();
	BP_OnRainIntensityChanged(RainIntensity);
}

void AWeatherController::SetTargetRainIntensity(float NewTarget)
{
	TargetRainIntensity = FMath::Clamp(NewTarget, 0.0f, 1.0f);
}

void AWeatherController::StartStorm()
{
	SetTargetRainIntensity(FMath::RandRange(0.7f, 1.0f));
}

void AWeatherController::ClearWeather()
{
	SetTargetRainIntensity(0.0f);
}

void AWeatherController::ScheduleNextWeatherChange()
{
	const float Delay = FMath::RandRange(MinWeatherInterval, MaxWeatherInterval);
	GetWorld()->GetTimerManager().SetTimer(
		WeatherChangeTimer,
		this,
		&AWeatherController::ChangeWeather,
		Delay,
		false
	);
}

void AWeatherController::ChangeWeather()
{
	// Random weather: 40% chance of rain, 60% chance of clearing
	if (FMath::RandRange(0.0f, 1.0f) < 0.4f)
	{
		// Rain event - random intensity
		SetTargetRainIntensity(FMath::RandRange(0.3f, 1.0f));
	}
	else
	{
		// Clear up
		SetTargetRainIntensity(FMath::RandRange(0.0f, 0.2f));
	}

	ScheduleNextWeatherChange();
}

void AWeatherController::UpdatePlayerStealthMask()
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player) return;

	UStealthComponent* Stealth = Player->FindComponentByClass<UStealthComponent>();
	if (Stealth)
	{
		Stealth->SetRainMaskFactor(RainIntensity);
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Player/Components/HealthFeedbackComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

UHealthFeedbackComponent::UHealthFeedbackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHealthFeedbackComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize MPC parameters to healthy state
	if (HealthMPC)
	{
		UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), HealthMPC, VignetteIntensityParam, 0.0f);
		UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), HealthMPC, DesaturationParam, 0.0f);
	}
}

void UHealthFeedbackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateVisualFeedback(DeltaTime);
}

void UHealthFeedbackComponent::OnDamageTaken(float NewHealthPercent)
{
	CurrentHealthPercent = FMath::Clamp(NewHealthPercent, 0.0f, 1.0f);

	// Play damage hit sound
	if (DamageHitSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), DamageHitSound);
	}

	// Calculate target visual feedback values based on health
	// Per GDD: More damage = more intense screen effects
	if (CurrentHealthPercent <= CriticalHealthThreshold)
	{
		// Critical: Heavy vignette, significant desaturation
		TargetVignette = 0.8f;
		TargetDesaturation = 0.6f;
	}
	else if (CurrentHealthPercent <= LowHealthThreshold)
	{
		// Low: Moderate vignette, slight desaturation
		const float T = (LowHealthThreshold - CurrentHealthPercent) / (LowHealthThreshold - CriticalHealthThreshold);
		TargetVignette = FMath::Lerp(0.2f, 0.5f, T);
		TargetDesaturation = FMath::Lerp(0.0f, 0.3f, T);
	}
	else
	{
		// Healthy: Minimal effects
		const float T = 1.0f - CurrentHealthPercent;
		TargetVignette = T * 0.2f;
		TargetDesaturation = 0.0f;
	}

	UpdateAudioFeedback();
}

void UHealthFeedbackComponent::UpdateVisualFeedback(float DeltaTime)
{
	if (!HealthMPC) return;

	// Smoothly interpolate toward target values
	const float InterpSpeed = 5.0f;

	float CurrentVignette = 0.0f;
	float CurrentDesat = 0.0f;

	// Get current values (use 0 as default if param doesn't exist)
	UMaterialParameterCollectionInstance* MPCI = GetWorld()->GetParameterCollectionInstance(HealthMPC);
	if (MPCI)
	{
		MPCI->GetScalarParameterValue(VignetteIntensityParam, CurrentVignette);
		MPCI->GetScalarParameterValue(DesaturationParam, CurrentDesat);
	}

	const float NewVignette = FMath::FInterpTo(CurrentVignette, TargetVignette, DeltaTime, InterpSpeed);
	const float NewDesat = FMath::FInterpTo(CurrentDesat, TargetDesaturation, DeltaTime, InterpSpeed);

	UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), HealthMPC, VignetteIntensityParam, NewVignette);
	UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), HealthMPC, DesaturationParam, NewDesat);
}

void UHealthFeedbackComponent::UpdateAudioFeedback()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	// Per GDD: Breathing becomes labored at low health
	if (CurrentHealthPercent <= LowHealthThreshold)
	{
		if (LaboredBreathingSound && !BreathingAudioComp)
		{
			BreathingAudioComp = UGameplayStatics::SpawnSoundAttached(
				LaboredBreathingSound,
				Owner->GetRootComponent(),
				NAME_None,
				FVector::ZeroVector,
				EAttachLocation::SnapToTarget,
				true
			);
			if (BreathingAudioComp)
			{
				BreathingAudioComp->bAutoDestroy = false;
			}
		}
	}
	else
	{
		if (BreathingAudioComp && BreathingAudioComp->IsPlaying())
		{
			BreathingAudioComp->FadeOut(1.0f, 0.0f);
		}
	}

	// Per GDD: Heartbeat at critical health
	if (CurrentHealthPercent <= CriticalHealthThreshold)
	{
		if (HeartbeatSound && !HeartbeatAudioComp)
		{
			HeartbeatAudioComp = UGameplayStatics::SpawnSoundAttached(
				HeartbeatSound,
				Owner->GetRootComponent(),
				NAME_None,
				FVector::ZeroVector,
				EAttachLocation::SnapToTarget,
				true
			);
			if (HeartbeatAudioComp)
			{
				HeartbeatAudioComp->bAutoDestroy = false;
			}
		}
	}
	else
	{
		if (HeartbeatAudioComp && HeartbeatAudioComp->IsPlaying())
		{
			HeartbeatAudioComp->FadeOut(0.5f, 0.0f);
		}
	}
}

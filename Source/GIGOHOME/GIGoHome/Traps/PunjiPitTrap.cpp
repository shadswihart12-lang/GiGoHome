// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Traps/PunjiPitTrap.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

APunjiPitTrap::APunjiPitTrap()
{
	PitCoverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PitCoverMesh"));
	PitCoverMesh->SetupAttachment(RootComponent);

	PitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PitMesh"));
	PitMesh->SetupAttachment(RootComponent);
	PitMesh->SetHiddenInGame(true);

	// punji pits deal moderate damage and slow - deadliness is the immobilisation
	Damage = 60.0f;
}

void APunjiPitTrap::OnTriggered(AActor* TriggeringActor)
{
	// reveal the pit by hiding the cover
	if (PitCoverMesh)
	{
		PitCoverMesh->SetHiddenInGame(true);
	}
	if (PitMesh)
	{
		PitMesh->SetHiddenInGame(false);
	}

	// apply stake damage
	ApplyTrapDamage(TriggeringActor);

	// slow the victim's movement if they are a character
	if (ACharacter* VictimChar = Cast<ACharacter>(TriggeringActor))
	{
		UCharacterMovementComponent* Movement = VictimChar->GetCharacterMovement();
		if (Movement)
		{
			const float OriginalSpeed = Movement->MaxWalkSpeed;
			Movement->MaxWalkSpeed *= MovementSlowMultiplier;

			// restore speed after slow duration using a lambda timer
			FTimerHandle SlowTimer;
			FTimerDelegate RestoreDelegate;
			RestoreDelegate.BindLambda([Movement, OriginalSpeed]()
			{
				if (IsValid(Movement))
				{
					Movement->MaxWalkSpeed = OriginalSpeed;
				}
			});
			GetWorld()->GetTimerManager().SetTimer(SlowTimer, RestoreDelegate, SlowDuration, false);
		}
	}

    OnTrapTriggeredEffect(TriggeringActor);

	// pit traps persist visually but are spent - don't destroy, just disarm
	// Actor destruction skipped to keep the visual pit in the world
}

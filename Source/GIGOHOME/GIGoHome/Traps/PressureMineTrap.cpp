// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Traps/PressureMineTrap.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

APressureMineTrap::APressureMineTrap()
{
	ConceaimentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ConcealmentMesh"));
	ConceaimentMesh->SetupAttachment(RootComponent);

	// mines deal significant damage
	Damage = 250.0f;
}

void APressureMineTrap::BeginPlay()
{
	Super::BeginPlay();

	// schedule hiding the concealment mesh when armed (gives visual of burying)
	FTimerHandle ConcealTimer;
	GetWorld()->GetTimerManager().SetTimer(ConcealTimer, [this]()
	{
		if (IsValid(ConceaimentMesh))
		{
			ConceaimentMesh->SetHiddenInGame(true);
		}
	}, ArmDelay, false);
}

void APressureMineTrap::OnTriggered(AActor* TriggeringActor)
{
	Detonate(TriggeringActor);
    OnTrapTriggeredEffect(TriggeringActor);
	Destroy();
}

void APressureMineTrap::Detonate(AActor* DirectVictim)
{
	// radial damage to all actors in blast radius
	UGameplayStatics::ApplyRadialDamage(
		GetWorld(),
		Damage,
		GetActorLocation(),
		BlastRadius,
		DamageTypeClass,
		TArray<AActor*>(),
		this,
		nullptr,
		true,
		ECC_Visibility
	);

	// apply physics impulse to the direct victim
	if (ACharacter* VictimChar = Cast<ACharacter>(DirectVictim))
	{
		if (USkeletalMeshComponent* Mesh = VictimChar->GetMesh())
		{
			// upward blast impulse
			const FVector BlastDir = (DirectVictim->GetActorLocation() - GetActorLocation()).GetSafeNormal()
				+ FVector(0.0f, 0.0f, 0.8f);
			Mesh->AddImpulse(BlastDir.GetSafeNormal() * BlastImpulse, NAME_None, true);
		}
	}
}

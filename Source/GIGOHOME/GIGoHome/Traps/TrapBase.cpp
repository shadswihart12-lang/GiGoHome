// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Traps/TrapBase.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ATrapBase::ATrapBase()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerVolume = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(RootComponent);
	TriggerVolume->SetSphereRadius(60.0f);
	TriggerVolume->SetCollisionProfileName(FName("Trigger"));

	RootComponent = TriggerVolume;
}

void ATrapBase::BeginPlay()
{
	Super::BeginPlay();

	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ATrapBase::OnTriggerVolumeOverlap);

	// begin arming sequence
	GetWorld()->GetTimerManager().SetTimer(
		ArmTimer,
		this,
		&ATrapBase::Arm,
		ArmDelay,
		false
	);
}

void ATrapBase::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(ArmTimer);
}

void ATrapBase::Arm()
{
	bIsArmed = true;
}

void ATrapBase::OnTriggerVolumeOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!bIsArmed || bHasTriggered || !OtherActor)
	{
		return;
	}

	// don't trigger on Xi Dong or his squad
	if (OtherActor->ActorHasTag(FriendlyTag))
	{
		return;
	}

	bHasTriggered = true;
	OnTriggered(OtherActor);
}

void ATrapBase::OnTriggered(AActor* TriggeringActor)
{
	// apply base damage
	ApplyTrapDamage(TriggeringActor);

	OnTrapTriggeredEffect(TriggeringActor);

	// single-use trap - destroy after triggering
	Destroy();
}

void ATrapBase::ApplyTrapDamage(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	UGameplayStatics::ApplyDamage(
		Target,
		Damage,
		nullptr,
		this,
		DamageTypeClass
	);
}

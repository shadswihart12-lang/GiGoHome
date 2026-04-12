// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Traps/TripwireTrap.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ATripwireTrap::ATripwireTrap()
{
	WireTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("WireTrigger"));
	WireTrigger->SetupAttachment(RootComponent);
	// long thin box to simulate a wire strung across a corridor
	WireTrigger->SetBoxExtent(FVector(150.0f, 5.0f, 40.0f));
	WireTrigger->SetCollisionProfileName(FName("Trigger"));

	GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
	GrenadeMesh->SetupAttachment(RootComponent);

	// disable the inherited sphere trigger - tripwire uses the box trigger instead
	if (USphereComponent* Sphere = GetTriggerVolume())
	{
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Sphere->SetGenerateOverlapEvents(false);
	}

	// tripwire grenades deal high damage
	Damage = 200.0f;
}

void ATripwireTrap::BeginPlay()
{
	Super::BeginPlay();

	// bind the box wire trigger - the base sphere is disabled
	WireTrigger->OnComponentBeginOverlap.AddDynamic(this, &ATripwireTrap::OnWireTriggerOverlap);
}

void ATripwireTrap::OnWireTriggerOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bIsArmed || bHasTriggered || !OtherActor)
	{
		return;
	}

	// don't trigger on Xi Dong or squad
	if (OtherActor->ActorHasTag(FriendlyTag))
	{
		return;
	}

	bHasTriggered = true;
	OnTriggered(OtherActor);
}

void ATripwireTrap::OnTriggered(AActor* TriggeringActor)
{
	ExplodeRadial();
    OnTrapTriggeredEffect(TriggeringActor);
	Destroy();
}

void ATripwireTrap::ExplodeRadial()
{
	// apply radial damage to all actors within ExplosionRadius
	UGameplayStatics::ApplyRadialDamage(
		GetWorld(),
		Damage,
		GetActorLocation(),
		ExplosionRadius,
		DamageTypeClass,
		TArray<AActor*>(),
		this,
		nullptr,
		true,
		ECC_Visibility
	);
}

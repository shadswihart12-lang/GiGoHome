// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Weapons/WeaponPickup.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AWeaponPickup::AWeaponPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root trigger sphere
	PickupTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("PickupTrigger"));
	RootComponent = PickupTrigger;
	PickupTrigger->SetSphereRadius(80.0f);
	PickupTrigger->SetCollisionProfileName(FName("Trigger"));
	PickupTrigger->SetGenerateOverlapEvents(true);

	// Visual weapon mesh
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeaponPickup::BeginPlay()
{
	Super::BeginPlay();

	PickupTrigger->OnComponentBeginOverlap.AddDynamic(this, &AWeaponPickup::OnPickupOverlap);
}

void AWeaponPickup::SetWeaponClass(TSubclassOf<AShooterWeapon> NewWeaponClass, int32 Ammo)
{
	WeaponClass = NewWeaponClass;
	AmmoAmount = Ammo;
}

void AWeaponPickup::OnPickupOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || !OtherActor->ActorHasTag(PlayerTag))
	{
		return;
	}

	if (!WeaponClass)
	{
		return;
	}

	// Grant the weapon to the player
	AShooterCharacter* ShooterChar = Cast<AShooterCharacter>(OtherActor);
	if (ShooterChar)
	{
		ShooterChar->AddWeaponClass(WeaponClass);

		// Play pickup sound
		if (PickupSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
		}

		// Notify subclasses
		OnWeaponPickedUp(OtherActor);

		// Destroy if configured
		if (bDestroyOnPickup)
		{
			Destroy();
		}
	}
}

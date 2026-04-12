// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Radio Actor Implementation

#include "GIGoHome/Campaign/Actors/GIRadioActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"

AGIRadioActor::AGIRadioActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	RadioMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RadioMesh"));
	RadioMesh->SetupAttachment(Root);
	RadioMesh->SetCollisionProfileName(TEXT("BlockAll"));

	// Default cube mesh — replace with proper radio mesh asset in editor
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(
		TEXT("/Engine/BasicShapes/Cube.Cube")
	);
	if (MeshFinder.Succeeded())
	{
		RadioMesh->SetStaticMesh(MeshFinder.Object);
		RadioMesh->SetWorldScale3D(FVector(0.3f, 0.5f, 0.2f));
	}

	RadioChatter = CreateDefaultSubobject<UAudioComponent>(TEXT("RadioChatter"));
	RadioChatter->SetupAttachment(Root);
	RadioChatter->bAutoActivate = false;

	InteractionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(Root);
	InteractionVolume->SetSphereRadius(150.0f);
	InteractionVolume->SetCollisionProfileName(TEXT("OverlapAll"));

	CurrentHealth = MaxHealth;
}

void AGIRadioActor::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	if (bIsActive && ChatterSound)
	{
		RadioChatter->SetSound(ChatterSound);
		RadioChatter->Play();
	}
}

float AGIRadioActor::TakeDamage(float Damage, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDestroyed) return 0.0f;

	float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	CurrentHealth -= ActualDamage;

	if (CurrentHealth <= 0.0f)
	{
		DestroyRadio();
	}

	return ActualDamage;
}

void AGIRadioActor::UseRadio(AActor* User)
{
	if (!bIsActive || bIsDestroyed) return;

	OnRadioUsed.Broadcast(User);
}

void AGIRadioActor::DestroyRadio()
{
	if (bIsDestroyed) return;

	bIsDestroyed = true;
	bIsActive = false;

	// Stop chatter
	RadioChatter->Stop();

	// Play destruction sound
	if (DestroySound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DestroySound, GetActorLocation());
	}

	// Hide mesh (could replace with destroyed mesh variant)
	RadioMesh->SetVisibility(false);
	RadioMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	OnRadioDestroyed.Broadcast(this);
}

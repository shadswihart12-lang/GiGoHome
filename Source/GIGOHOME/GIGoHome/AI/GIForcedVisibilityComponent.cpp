// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/AI/GIForcedVisibilityComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

UGIForcedVisibilityComponent::UGIForcedVisibilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UGIForcedVisibilityComponent::OnRegister()
{
	Super::OnRegister();
	// Force visibility immediately on component registration
	ForceOwnerMeshVisible();
}

void UGIForcedVisibilityComponent::BeginPlay()
{
	Super::BeginPlay();
	// Force again in BeginPlay in case Blueprint modified it
	ForceOwnerMeshVisible();
}

void UGIForcedVisibilityComponent::ForceOwnerMeshVisible()
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar) return;

	USkeletalMeshComponent* Mesh = OwnerChar->GetMesh();
	if (!Mesh) return;

	// Force all visibility settings to make mesh visible to everyone
	Mesh->SetOwnerNoSee(false);
	Mesh->SetOnlyOwnerSee(false);
	Mesh->bOwnerNoSee = false;
	Mesh->bOnlyOwnerSee = false;
	Mesh->SetHiddenInGame(false);
	Mesh->SetVisibility(true);
	
	// Reset first person primitive type - NPCs shouldn't use this
	Mesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::None;

	// Force component to update its render state
	Mesh->MarkRenderStateDirty();
}

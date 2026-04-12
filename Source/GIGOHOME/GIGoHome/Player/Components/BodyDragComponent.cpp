// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Player/Components/BodyDragComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

UBodyDragComponent::UBodyDragComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBodyDragComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerPawn = Cast<APawn>(GetOwner());
}

bool UBodyDragComponent::StartDrag()
{
	// already dragging something
	if (DraggedActor)
	{
		return false;
	}

	AActor* NearbyBody = FindNearbyDeadActor();
	if (!NearbyBody)
	{
		return false;
	}

	// attach body to owner and position it at the drag offset
	const FAttachmentTransformRules AttachRules(EAttachmentRule::KeepWorld, false);
	NearbyBody->AttachToActor(GetOwner(), AttachRules);
	NearbyBody->SetActorRelativeLocation(DragOffset);

	DraggedActor = NearbyBody;
	return true;
}

void UBodyDragComponent::StopDrag()
{
	if (!DraggedActor)
	{
		return;
	}

	// release with current world transform preserved
	const FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, false);
	DraggedActor->DetachFromActor(DetachRules);

	DraggedActor = nullptr;
}

AActor* UBodyDragComponent::FindNearbyDeadActor() const
{
	if (!OwnerPawn)
	{
		return nullptr;
	}

	const FVector Start = OwnerPawn->GetActorLocation();
	const FVector End = Start + (OwnerPawn->GetActorForwardVector() * DragReach);

	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	const FCollisionShape Sphere = FCollisionShape::MakeSphere(DragSweepRadius);
	GetWorld()->SweepMultiByChannel(HitResults, Start, End, FQuat::Identity, ECC_Pawn, Sphere, QueryParams);

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->ActorHasTag(DeadActorTag))
		{
			return HitActor;
		}
	}

	return nullptr;
}

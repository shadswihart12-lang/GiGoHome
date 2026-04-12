// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BodyDragComponent.generated.h"

/**
 * Allows Xi Dong to grab dead enemy bodies and drag them to concealment
 * Uses a forward sphere sweep to find nearby dead actors (tagged "Dead")
 * Attaches the body to the character via relative offset while dragging
 */
UCLASS(ClassGroup=(GIGoHome), meta=(BlueprintSpawnableComponent))
class GIGOHOME_API UBodyDragComponent : public UActorComponent
{
	GENERATED_BODY()

protected:

	/** Reach distance forward to detect draggable bodies (cm) */
	UPROPERTY(EditAnywhere, Category="BodyDrag", meta=(ClampMin=0.0f, ClampMax=500.0f, Units="cm"))
	float DragReach = 200.0f;

	/** Sweep sphere radius for body detection (cm) */
	UPROPERTY(EditAnywhere, Category="BodyDrag", meta=(ClampMin=0.0f, ClampMax=200.0f, Units="cm"))
	float DragSweepRadius = 60.0f;

	/** Tag that dead actors carry - used to identify draggable bodies */
	UPROPERTY(EditAnywhere, Category="BodyDrag")
	FName DeadActorTag = FName("Dead");

	/** Offset from owner origin to position the dragged body */
	UPROPERTY(EditAnywhere, Category="BodyDrag")
	FVector DragOffset = FVector(80.0f, 0.0f, -60.0f);

	/** Currently dragged body actor */
	UPROPERTY(BlueprintReadOnly, Category="BodyDrag")
	TObjectPtr<AActor> DraggedActor;

	/** Cached pawn owner */
	TObjectPtr<APawn> OwnerPawn;

public:

	UBodyDragComponent();

protected:

	virtual void BeginPlay() override;

public:

	/**
	 * Attempts to grab a nearby dead body.
	 * Returns true if a body was found and grabbed.
	 */
	UFUNCTION(BlueprintCallable, Category="BodyDrag")
	bool StartDrag();

	/** Releases the currently dragged body at Xi Dong's current position */
	UFUNCTION(BlueprintCallable, Category="BodyDrag")
	void StopDrag();

	/** Returns true if currently dragging a body */
	UFUNCTION(BlueprintPure, Category="BodyDrag")
	bool IsDragging() const { return DraggedActor != nullptr; }

	/** Returns the currently dragged actor, or nullptr */
	UFUNCTION(BlueprintPure, Category="BodyDrag")
	AActor* GetDraggedActor() const { return DraggedActor; }

protected:

	/** Sweeps forward from the owner to find the nearest dead actor */
	AActor* FindNearbyDeadActor() const;
};

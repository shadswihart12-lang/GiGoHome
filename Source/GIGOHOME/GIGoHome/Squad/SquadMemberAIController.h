// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/AI/ShooterAIController.h"
#include "GIGoHome/Player/Components/SquadCommandComponent.h"
#include "SquadMemberAIController.generated.h"

/**
 * AI Controller for Xi Dong's squad companions
 * Extends ShooterAIController with squad order handling
 * Receives orders from USquadCommandComponent and exposes them
 * for StateTree tasks to read and act upon
 */
UCLASS()
class GIGOHOME_API ASquadMemberAIController : public AShooterAIController
{
	GENERATED_BODY()

protected:

	/** Current order issued by Xi Dong */
	UPROPERTY(BlueprintReadOnly, Category="Squad")
	ESquadOrder CurrentOrder = ESquadOrder::HoldPosition;

public:

	/**
	 * Receives an order from Xi Dong's SquadCommandComponent
	 * Updates CurrentOrder and triggers BP_OnOrderReceived for StateTree
	 */
	void ReceiveOrder(ESquadOrder Order);

	/** Returns the current active order */
	UFUNCTION(BlueprintPure, Category="Squad")
	ESquadOrder GetCurrentOrder() const { return CurrentOrder; }

protected:

	/**
	 * Called when a new order is received - override for custom behavior
	 */
	virtual void OnOrderReceived(ESquadOrder NewOrder) {}
};

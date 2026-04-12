// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/AI/ShooterAIController.h"
#include "GIEnemyAIController.generated.h"

class AGIEnemyCharacter;

/**
 * AI Controller for American enemy soldiers in GI Go Home
 * Extends ShooterAIController with:
 *   - Fear accumulation on hearing gunfire / finding bodies
 *   - Radio call tracking (reaching a radio actor calls reinforcements)
 *   - Rout handling when morale collapses
 *
 * All complex behavioral logic is handled in Blueprint StateTree.
 * This class exposes C++ hooks that StateTree tasks call into.
 */
UCLASS()
class GIGOHOME_API AGIEnemyAIController : public AShooterAIController
{
	GENERATED_BODY()

protected:

	/** Tag identifying radio actors in the level */
	UPROPERTY(EditAnywhere, Category="Enemy")
	FName RadioActorTag = FName("Radio");

	/** Fear added per gunshot heard nearby */
	UPROPERTY(EditAnywhere, Category="Enemy", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float FearPerGunshot = 0.15f;

	/** Fear added when this AI discovers a dead body */
	UPROPERTY(EditAnywhere, Category="Enemy", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float FearPerBodyFound = 0.3f;

	/** Cached pointer to possessed enemy character */
	TObjectPtr<AGIEnemyCharacter> PossessedEnemy;

public:

	AGIEnemyAIController();

protected:

	virtual void OnPossess(APawn* InPawn) override;

public:

	/**
	 * Called by a StateTree task when this enemy discovers a dead body
	 * Adds fear and propagates alert to nearby allies
	 */
	UFUNCTION(BlueprintCallable, Category="Enemy")
	void NotifyBodyDiscovered();

	/**
	 * Called by a StateTree task when this enemy reaches a radio actor
	 * Marks reinforcements as called if the enemy has a radio
	 */
	UFUNCTION(BlueprintCallable, Category="Enemy")
	void AttemptRadioCall();

	/** Returns true if the possessed enemy is currently routing */
	UFUNCTION(BlueprintPure, Category="Enemy")
	bool IsPossessedEnemyRouting() const;

protected:

	/**
	 * Additional perception listener bound in OnPossess
	 * Converts hearing stimuli to fear on the possessed enemy character
	 */
	UFUNCTION()
	void OnEnemyPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:

	/** Propagates a fear increase to nearby ally enemies within the given radius */
	void PropagateAlertToNearbyAllies(float FearAmount, float Radius);
};

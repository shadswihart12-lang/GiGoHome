// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SquadCommandComponent.generated.h"

class ASquadMemberCharacter;
class ASquadMemberAIController;

/** Orders Xi Dong can issue to his squad */
UENUM(BlueprintType)
enum class ESquadOrder : uint8
{
	HoldPosition,
	Advance,
	Flank,
	FallBack
};

/**
 * Manages Xi Dong's squad of up to 3 Vietcong companions
 * Squad members are added when they join during the campaign
 * Orders are broadcast to all active (non-wounded, non-dead) members
 */
UCLASS(ClassGroup=(GIGoHome), meta=(BlueprintSpawnableComponent))
class GIGOHOME_API USquadCommandComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Maximum squad size as specified in the GDD */
	static constexpr int32 MaxSquadSize = 3;

protected:

	/** Current squad members for this mission */
	UPROPERTY(BlueprintReadOnly, Category="Squad")
	TArray<TObjectPtr<ASquadMemberCharacter>> SquadMembers;

public:

	USquadCommandComponent();

public:

	/**
	 * Adds a companion to the squad.
	 * Does nothing if the squad is already at maximum size or the member is already added.
	 */
	UFUNCTION(BlueprintCallable, Category="Squad")
	void AddSquadMember(ASquadMemberCharacter* NewMember);

	/** Removes a squad member - typically called on their death */
	UFUNCTION(BlueprintCallable, Category="Squad")
	void RemoveSquadMember(ASquadMemberCharacter* Member);

	/** Issues an order to all active squad members */
	UFUNCTION(BlueprintCallable, Category="Squad")
	void IssueOrder(ESquadOrder Order);

	/** Returns the number of active (non-wounded, non-dead) squad members */
	UFUNCTION(BlueprintPure, Category="Squad")
	int32 GetActiveSquadMemberCount() const;

	/** Returns all squad members including wounded ones */
	const TArray<TObjectPtr<ASquadMemberCharacter>>& GetSquadMembers() const { return SquadMembers; }
};

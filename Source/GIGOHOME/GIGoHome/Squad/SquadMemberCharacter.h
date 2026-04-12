// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/AI/ShooterNPC.h"
#include "SquadMemberCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSquadMemberWoundedDelegate, ASquadMemberCharacter*, WoundedMember);

/**
 * A named Vietcong companion in Xi Dong's squad
 *
 * Two-stage damage:
 *   First time HP reaches zero: member becomes Wounded (out of action this mission,
 *   absent next mission, recovers after that). Campaign state is notified.
 *   Second time HP reaches zero while Wounded: permanent death. Campaign state is notified.
 *
 * Squad members have names and backstories for narrative weight.
 * Their deaths are permanently recorded and reflected in subsequent missions.
 */
UCLASS(abstract)
class GIGOHOME_API ASquadMemberCharacter : public AShooterNPC
{
	GENERATED_BODY()

public:

	/** Broadcast when this member is wounded (first lethal hit) */
	UPROPERTY(BlueprintAssignable, Category="Squad")
	FSquadMemberWoundedDelegate OnSquadMemberWounded;

protected:

	/** Display name shown in narrative moments */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	FText MemberName;

	/** Short backstory text used in journal entries */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Narrative")
	FText Backstory;

	/**
	 * Unique tag name for campaign state tracking
	 * Must be unique per squad member Blueprint subclass
	 */
	UPROPERTY(EditAnywhere, Category="Squad")
	FName MemberTag;

	/** HP amount to restore when the member would otherwise be killed (wounded state) */
	UPROPERTY(EditAnywhere, Category="Squad", meta=(ClampMin=1.0f))
	float WoundedHP = 1.0f;

	/** If true, this member is wounded and cannot fight until recovered */
	UPROPERTY(BlueprintReadOnly, Category="Squad")
	bool bIsWounded = false;

public:

	/** Handle incoming damage with two-stage wound system */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** Returns true if this member is currently wounded */
	UFUNCTION(BlueprintPure, Category="Squad")
	bool IsWounded() const { return bIsWounded; }

	/** Returns the campaign-tracking tag for this member */
	FName GetMemberTag() const { return MemberTag; }

	/** Returns the display name */
	const FText& GetMemberName() const { return MemberName; }

protected:

	/** Called when a lethal hit is survived - wounds instead of kills */
	void BecomeWounded();

	/** Called when a wounded member takes any damage - permanent death */
	void DiePermantly();

	/** Deferred destruction callback after ragdoll settles */
	void DeferredDestructionSquad();
};

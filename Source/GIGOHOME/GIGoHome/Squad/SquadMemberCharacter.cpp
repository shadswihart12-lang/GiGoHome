// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Squad/SquadMemberCharacter.h"
#include "GIGoHome/GIGoHomeCampaignState.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"

float ASquadMemberCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// already dead - ignore
	if (bIsDead)
	{
		return 0.0f;
	}

	// wounded + any damage = permanent death
	if (bIsWounded)
	{
		DiePermantly();
		return Damage;
	}

	// first lethal hit - wound instead of kill
	CurrentHP -= Damage;

	if (CurrentHP <= 0.0f)
	{
		BecomeWounded();
	}

	return Damage;
}

void ASquadMemberCharacter::BecomeWounded()
{
	bIsWounded = true;
	CurrentHP = WoundedHP;

	// record in campaign state
	if (AGIGoHomeCampaignState* CS = Cast<AGIGoHomeCampaignState>(UGameplayStatics::GetGameState(this)))
	{
		CS->MarkSquadMemberWounded(MemberTag);
	}

	// stop fighting
	GetCharacterMovement()->StopMovementImmediately();
	if (Weapon)
	{
		Weapon->StopFiring();
	}

	OnSquadMemberWounded.Broadcast(this);
}

void ASquadMemberCharacter::DiePermantly()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	Tags.Add(DeathTag);

	// record permanent death in campaign state
	if (AGIGoHomeCampaignState* CS = Cast<AGIGoHomeCampaignState>(UGameplayStatics::GetGameState(this)))
	{
		CS->MarkSquadMemberDead(MemberTag);
	}

	// notify AI controller so it can clean up
	OnPawnDeath.Broadcast();

	// stop weapon
	if (Weapon)
	{
		Weapon->StopFiring();
	}

	// disable capsule collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// stop movement
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->StopActiveMovement();

	// ragdoll the third person mesh
	GetMesh()->SetCollisionProfileName(RagdollCollisionProfile);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetPhysicsBlendWeight(1.0f);

	// schedule actor destruction
	GetWorld()->GetTimerManager().SetTimer(
		DeathTimer,
		this,
		&ASquadMemberCharacter::DeferredDestructionSquad,
		DeferredDestructionTime,
		false
	);
}

void ASquadMemberCharacter::DeferredDestructionSquad()
{
	Destroy();
}

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TunnelEntrance.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 * Cu Chi tunnel entrance - hidden underground access point
 * Place two ATunnelEntrance actors in the level and link them bidirectionally
 * When Xi Dong overlaps the interaction volume, he is teleported to the linked entrance
 *
 * Usage: set LinkedEntrance on each entrance in the editor to point to its partner.
 * The tunnel system supports chains of entrances (A->B->C) by setting each link.
 */
UCLASS()
class GIGOHOME_API ATunnelEntrance : public AActor
{
	GENERATED_BODY()

	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USceneComponent* Root;

	/** Trigger volume - player overlaps to enter tunnel */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USphereComponent* InteractionVolume;

	/** Visual mesh for the tunnel entrance (camouflaged hatch, bamboo cover, etc.) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* EntranceMesh;

protected:

	/** The paired exit tunnel entrance this entrance teleports to */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tunnel")
	TObjectPtr<ATunnelEntrance> LinkedEntrance;

	/** Tag players must have to use this tunnel (prevents enemies from using it) */
	UPROPERTY(EditAnywhere, Category="Tunnel")
	FName PlayerTag = FName("Player");

	/** If true, this entrance is currently blocked (collapsed tunnel) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tunnel")
	bool bIsBlocked = false;

	/** Brief cooldown after using tunnel to prevent immediate re-entry (seconds) */
	UPROPERTY(EditAnywhere, Category="Tunnel", meta=(ClampMin=0.5f, Units="s"))
	float UseCooldown = 1.5f;

	/** If true, tunnel is currently on cooldown and cannot be used */
	bool bOnCooldown = false;

	/** Cooldown timer handle */
	FTimerHandle CooldownTimer;

public:

	ATunnelEntrance();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:

	/** Returns true if this entrance can currently be used */
	UFUNCTION(BlueprintPure, Category="Tunnel")
	bool CanUse() const;

	/** Blocks this tunnel entrance (e.g., collapsed by bombing) */
	UFUNCTION(BlueprintCallable, Category="Tunnel")
	void BlockEntrance();

	/** Returns the linked exit entrance */
	ATunnelEntrance* GetLinkedEntrance() const { return LinkedEntrance; }

    /** Called when a player successfully enters this tunnel */
	virtual void OnPlayerEnteredTunnelEffect(AActor* Player) {}

protected:

	/** Called when an actor overlaps the interaction volume */
	UFUNCTION()
	void OnInteractionVolumeOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	/** Teleports the given actor to the linked entrance's location */
	void TeleportActorToLinkedEntrance(AActor* ActorToTeleport);

	/** Clears the use cooldown */
	void ClearCooldown();
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "XiDongPlayerController.generated.h"

class UInputMappingContext;
class AXiDongCharacter;

/**
 * Player controller for Xi Dong
 * Manages input mapping context and pawn possession
 * No respawn logic - Xi Dong's death is handled by AGIGoHomeGameMode
 * No bullet counter UI - ammo is checked manually via DoAmmoCheck
 */
UCLASS(abstract, config="Game")
class GIGOHOME_API AXiDongPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Default input mapping context for keyboard/gamepad */
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Character class to spawn as Xi Dong at mission start */
	UPROPERTY(EditAnywhere, Category="Spawn")
	TSubclassOf<AXiDongCharacter> XiDongClass;

	/** Tag granted to Xi Dong on possession for tunnel/cache identification */
	UPROPERTY(EditAnywhere, Category="Spawn")
	FName PlayerPawnTag = FName("Player");

protected:

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

	/** Called when Xi Dong's pawn is destroyed (death -> GameMode handles retry) */
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FoliageCoverVolume.generated.h"

class UBoxComponent;

/**
 * Trigger volume that provides stealth cover in dense foliage
 * When Xi Dong enters, his visibility level drops (StealthComponent::SetInFoliageCover)
 * 
 * Can be spawned by PCG or placed manually in the level.
 * Supports varying cover quality (dense jungle vs sparse grass).
 */
UCLASS(Blueprintable)
class GIGOHOME_API AFoliageCoverVolume : public AActor
{
	GENERATED_BODY()

	/** Trigger volume for detecting player entry/exit */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UBoxComponent* CoverTrigger;

protected:

	/**
	 * Cover quality multiplier (0-1)
	 * 1.0 = dense jungle (full cover)
	 * 0.5 = moderate brush
	 * 0.2 = sparse grass
	 * Passed to StealthComponent to scale visibility reduction
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cover", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float CoverQuality = 1.0f;

	/** Tag required on actors to receive cover (default: Player) */
	UPROPERTY(EditAnywhere, Category="Cover")
	FName PlayerTag = FName("Player");

	/** If true, also reduces noise slightly (rustling leaves mask footsteps) */
	UPROPERTY(EditAnywhere, Category="Cover")
	bool bMasksNoise = false;

	/** Noise reduction multiplier when bMasksNoise is true */
	UPROPERTY(EditAnywhere, Category="Cover", meta=(ClampMin=0.0f, ClampMax=1.0f, EditCondition="bMasksNoise"))
	float NoiseMaskFactor = 0.3f;

public:

	AFoliageCoverVolume();

	/** Returns the cover quality of this volume */
	UFUNCTION(BlueprintPure, Category="Cover")
	float GetCoverQuality() const { return CoverQuality; }

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnCoverBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnCoverEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);
};

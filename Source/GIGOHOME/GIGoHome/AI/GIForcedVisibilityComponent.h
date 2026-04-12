// Copyright Epic Games, Inc. All Rights Reserved.
// GIForcedVisibilityComponent - Ensures NPC meshes are always visible
// Solves the inheritance issue where first-person player visibility settings
// propagate to NPC subclasses

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GIForcedVisibilityComponent.generated.h"

/**
 * Attach this component to any AI character that needs to be visible.
 * It forces the character's mesh to be visible regardless of base class settings.
 * 
 * The problem: AGIGOHOMECharacter sets OwnerNoSee=true for first-person player hiding.
 * NPC classes inherit this and their meshes become invisible.
 * This component fixes that by forcing visibility in PostInitializeComponents.
 */
UCLASS(ClassGroup=(GIGoHome), meta=(BlueprintSpawnableComponent))
class GIGOHOME_API UGIForcedVisibilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UGIForcedVisibilityComponent();

	virtual void OnRegister() override;

protected:

	virtual void BeginPlay() override;

private:

	/** Force the owner character's mesh to be visible */
	void ForceOwnerMeshVisible();
};

// Copyright Epic Games, Inc. All Rights Reserved.
// GI GO HOME - Concrete C++ Player Controller
// Creates Input Mapping Context entirely in C++
// No Blueprint or editor data assets required

#pragma once

#include "CoreMinimal.h"
#include "GIGOHOMEPlayerController.h"
#include "GIAct1PlayerController.generated.h"

class UInputMappingContext;

/**
 * Concrete player controller for the GI Go Home vertical slice.
 * In OnPossess, reads the character's Input Action pointers and
 * builds a UInputMappingContext mapping keyboard/mouse keys to them.
 * No Blueprint or data asset required.
 */
UCLASS()
class GIGOHOME_API AGIAct1PlayerController : public AGIGOHOMEPlayerController
{
	GENERATED_BODY()

public:
    AGIAct1PlayerController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

private:
	/** Creates the IMC from the pawn's Input Actions and adds it to Enhanced Input */
	void BuildInputMappingContext(APawn* InPawn);

	/** Cached IMC created at possession time */
	UPROPERTY()
	UInputMappingContext* GIGoHomeIMC;
};

// Copyright Epic Games, Inc. All Rights Reserved.
// Production-safe unique id generation for inventory/runtime entities.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GIInventoryIdLibrary.generated.h"

UCLASS()
class GIGOHOME_API UGIInventoryIdLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Pure C++ API */
	static FName CreateUniqueID(const TCHAR* Prefix = TEXT("INV"));

	/** Compatibility wrapper for legacy BP nodes expecting output pin `Q_ID` */
	UFUNCTION(BlueprintCallable, Category="Inventory|ID", meta=(DisplayName="CreateUniqueID_F"))
	static void CreateUniqueID_F(FName& Q_ID);
};

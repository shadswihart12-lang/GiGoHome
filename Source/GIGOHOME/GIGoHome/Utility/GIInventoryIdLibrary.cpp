// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Utility/GIInventoryIdLibrary.h"
#include "HAL/PlatformTime.h"

FName UGIInventoryIdLibrary::CreateUniqueID(const TCHAR* Prefix)
{
	static uint64 LocalCounter = 0;
	++LocalCounter;

	const FGuid Guid = FGuid::NewGuid();
	const uint64 Cycles = FPlatformTime::Cycles64();

	const FString IdString = FString::Printf(
		TEXT("%s_%08X%08X_%llu_%llu"),
		Prefix ? Prefix : TEXT("INV"),
		Guid.A,
		Guid.B,
		Cycles,
		LocalCounter
	);

	return FName(*IdString);
}

void UGIInventoryIdLibrary::CreateUniqueID_F(FName& Q_ID)
{
	Q_ID = CreateUniqueID(TEXT("INV"));
}

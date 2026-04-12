// Copyright Epic Games, Inc. All Rights Reserved.

#include "GIGoHome/Squad/SquadMemberAIController.h"

void ASquadMemberAIController::ReceiveOrder(ESquadOrder Order)
{
	CurrentOrder = Order;
	OnOrderReceived(Order);
}

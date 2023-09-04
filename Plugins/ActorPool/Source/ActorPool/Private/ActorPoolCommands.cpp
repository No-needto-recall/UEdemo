// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActorPoolCommands.h"

#define LOCTEXT_NAMESPACE "FActorPoolModule"

void FActorPoolCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "ActorPool", "Bring up ActorPool window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE

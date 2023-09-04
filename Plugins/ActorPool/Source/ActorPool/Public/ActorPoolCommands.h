// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ActorPoolStyle.h"

class FActorPoolCommands : public TCommands<FActorPoolCommands>
{
public:

	FActorPoolCommands()
		: TCommands<FActorPoolCommands>(TEXT("ActorPool"), NSLOCTEXT("Contexts", "ActorPool", "ActorPool Plugin"), NAME_None, FActorPoolStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};
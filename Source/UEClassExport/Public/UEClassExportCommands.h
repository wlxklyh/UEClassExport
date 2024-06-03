// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "UEClassExportStyle.h"

class FUEClassExportCommands : public TCommands<FUEClassExportCommands>
{
public:

	FUEClassExportCommands()
		: TCommands<FUEClassExportCommands>(TEXT("UEClassExport"), NSLOCTEXT("Contexts", "UEClassExport", "UEClassExport Plugin"), NAME_None, FUEClassExportStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};

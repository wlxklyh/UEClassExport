// Copyright Epic Games, Inc. All Rights Reserved.

#include "UEClassExportCommands.h"

#define LOCTEXT_NAMESPACE "FUEClassExportModule"

void FUEClassExportCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "UEClassExport", "Execute UEClassExport action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE

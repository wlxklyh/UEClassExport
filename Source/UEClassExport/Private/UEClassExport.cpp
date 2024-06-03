// Copyright Epic Games, Inc. All Rights Reserved.

#include "UEClassExport.h"
#include "UEClassExportStyle.h"
#include "UEClassExportCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "Interfaces/IPluginManager.h"
#include "Windows/WindowsPlatformProcess.h"

static const FName UEClassExportTabName("UEClassExport");

#define LOCTEXT_NAMESPACE "FUEClassExportModule"

void FUEClassExportModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FUEClassExportStyle::Initialize();
	FUEClassExportStyle::ReloadTextures();

	FUEClassExportCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FUEClassExportCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FUEClassExportModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUEClassExportModule::RegisterMenus));
}

void FUEClassExportModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FUEClassExportStyle::Shutdown();

	FUEClassExportCommands::Unregister();
}


void WriteStringToFile(const FString& StringToWrite, const FString& FilePath)
{
    TArray<uint8> FileData;
    FileData.AddZeroed(StringToWrite.Len());
    FMemory::Memcpy(FileData.GetData(), TCHAR_TO_ANSI(*StringToWrite), StringToWrite.Len());

    FFileHelper::SaveArrayToFile(FileData, *FilePath);
}


void FUEClassExportModule::PluginButtonClicked()
{
	FString PluginPath = IPluginManager::Get().FindPlugin("UEClassExport")->GetBaseDir();
	FString UEClassInfoTxtPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(*PluginPath, TEXT("UEClassInfo.txt")));
	FString UEClassToXmindPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(*PluginPath, TEXT("UEClassToXmind.bat")));
	
	FString OutputString;
	TArray<UClass*> Classes;
	for (TObjectIterator<UClass> It; It; ++It)
	{
		Classes.Add(*It);
	}
	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;
		FString FormattedString = FString::Printf(TEXT("%s"), *Class->GetName());
		while(Class)
		{
			Class = Class->GetSuperClass();
			if(Class != nullptr)
			{
				FormattedString = FString::Printf(TEXT("%s->%s"), *FormattedString, *Class->GetName());
			}
		}
		OutputString += FormattedString + "\n";
	}
	WriteStringToFile(OutputString, UEClassInfoTxtPath);

	FPlatformProcess::CreateProc(*UEClassToXmindPath, nullptr, true, false, false, nullptr, 0, nullptr, nullptr);
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(FText::FromString(TEXT("UEClass Export")))
		.SizingRule(ESizingRule::Autosized);

	Window->SetContent(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("See UEClassXmind.xmind")))
		]
	);

	FSlateApplication::Get().AddWindow(Window);
}

void FUEClassExportModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FUEClassExportCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FUEClassExportCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUEClassExportModule, UEClassExport)
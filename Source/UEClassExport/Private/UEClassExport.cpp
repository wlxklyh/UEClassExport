// Copyright Epic Games, Inc. All Rights Reserved.

#include "UEClassExport.h"

#include "ClassViewerModule.h"
#include "UEClassExportStyle.h"
#include "UEClassExportCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "Engine/AssetManager.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet2/SClassPickerDialog.h"
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

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUEClassExportModule::RegisterMenus));
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

void LoadAllBlueprintClasses()
{
	// 获取游戏目录下的所有资产
	UAssetManager& AssetManager = UAssetManager::Get();
	IAssetRegistry& AssetRegistry = AssetManager.GetAssetRegistry();
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAllAssets(AssetDataList);

	for (const FAssetData& AssetData : AssetDataList)
	{
		// 检查资产是否是蓝图类
		if (AssetData.AssetClass == UBlueprint::StaticClass()->GetFName())
		{
			// 获取蓝图类的路径
			FStringAssetReference AssetRef(AssetData.ObjectPath.ToString());

			// 加载蓝图类
			UBlueprint* Blueprint = Cast<UBlueprint>(AssetRef.TryLoad());

			if (Blueprint)
			{
				// 蓝图类加载成功
				UE_LOG(LogTemp, Warning, TEXT("Loaded Blueprint Class: %s"), *Blueprint->GetName());
			}
		}
	}
}

void FUEClassExportModule::PluginButtonClicked()
{
	LoadAllBlueprintClasses();
	
	FClassViewerInitializationOptions Options;
    Options.bShowUnloadedBlueprints = true;
    Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::DisplayName;
    Options.bShowNoneOption = false;
    UClass* ChosenClass;
    const FText TitleText = LOCTEXT("UEClassExport", "Pick Class to export diagram");
    const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UObject::StaticClass());
    if (bPressedOk)
    {
    	FString strChosenClass = ChosenClass->GetName();
    	FString PluginPath = IPluginManager::Get().FindPlugin("UEClassExport")->GetBaseDir();
    	FString UEClassInfoTxtPath = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(*PluginPath, TEXT("UEClassInfo.txt")));
    	FString UEClassToXmindPath = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(*PluginPath, TEXT("UEClassToXmind.bat")));
    	FString OutputString;
    	TArray<UClass*> Classes;
    	GetDerivedClasses(UObject::StaticClass(), Classes);
    	for (TObjectIterator<UClass> It; It; ++It)
    	{
    		UClass* Class = *It;
    		FString FormattedString = FString::Printf(TEXT("%s"), *Class->GetName());
    		while (Class)
    		{
    			Class = Class->GetSuperClass();
    			if (Class != nullptr)
    			{
    				FormattedString = FString::Printf(TEXT("%s->%s"), *FormattedString, *Class->GetName());
    			}
    		}
    		OutputString += FormattedString + "\n";
    	}
    	WriteStringToFile(OutputString, UEClassInfoTxtPath);
    	FPlatformProcess::CreateProc(*UEClassToXmindPath, ToCStr(strChosenClass), false, false, false, nullptr, 0, ToCStr(PluginPath),
									 nullptr);
    }
	// Unreal Editor的声明式写法
	// TSharedPtr<SEditableText> EditableTextWidget;
	// TSharedRef<SWindow> Window = SNew(SWindow)
	// 	.Title(FText::FromString(TEXT("UEClassExport")))
	// 	.SizingRule(ESizingRule::UserSized)
	// 	.AutoCenter(EAutoCenter::PreferredWorkArea)
	// 	.SupportsMaximize(true)
	// 	.SupportsMinimize(true)
	// 	.MinWidth(200)
	// 	.MinHeight(100)
	// 	[
	// 		SNew(SVerticalBox)
	// 		+ SVerticalBox::Slot()
	// 		.AutoHeight()
	// 		.HAlign(HAlign_Center)
	// 		.VAlign(VAlign_Center)
	// 		[
	// 			SAssignNew(EditableTextWidget, SEditableText)
	// 			.HintText(FText::FromString(TEXT("Enter class here")))
	// 		]
	// 		+ SVerticalBox::Slot()
	// 		.AutoHeight()
	// 		.HAlign(HAlign_Center)
	// 		.VAlign(VAlign_Center)
	// 		[
	// 			SNew(SButton)
	// 			.Text(FText::FromString(TEXT("Confirm")))
	// 			.OnClicked_Lambda([EditableTextWidget]()
	// 			{
	// 				return FReply::Handled();
	// 			})
	// 		]
	// 	];
	// FSlateApplication::Get().AddWindow(Window);
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
				FToolMenuEntry& Entry = Section.AddEntry(
					FToolMenuEntry::InitToolBarButton(FUEClassExportCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUEClassExportModule, UEClassExport)

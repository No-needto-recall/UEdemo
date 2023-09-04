// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActorPool.h"
#include "ActorPoolStyle.h"
#include "ActorPoolCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

static const FName ActorPoolTabName("ActorPool");

#define LOCTEXT_NAMESPACE "FActorPoolModule"

void FActorPoolModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FActorPoolStyle::Initialize();
	FActorPoolStyle::ReloadTextures();

	FActorPoolCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FActorPoolCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FActorPoolModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FActorPoolModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ActorPoolTabName, FOnSpawnTab::CreateRaw(this, &FActorPoolModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FActorPoolTabTitle", "ActorPool"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FActorPoolModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FActorPoolStyle::Shutdown();

	FActorPoolCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ActorPoolTabName);
}

TSharedRef<SDockTab> FActorPoolModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FActorPoolModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("ActorPool.cpp"))
		);
	
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(WidgetText)
			]
		];
}

void FActorPoolModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ActorPoolTabName);
}

void FActorPoolModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FActorPoolCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FActorPoolCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FActorPoolModule, ActorPool)
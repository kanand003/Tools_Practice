// Copyright Epic Games, Inc. All Rights Reserved.

#include "SlateDemoModule.h"
#include "SShowcaseWindow.h"

#include "ToolMenus.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"

#define LOCTEXT_NAMESPACE "FSlateDemoModule"
static const FName ShowcaseMenuName("LevelEditor.MainMenu.Tools");
static const FName ShowcaseEntryName("OpenSlateShowcase");

IMPLEMENT_MODULE(FSlateDemoModule, SlateDemo)


void FSlateDemoModule::StartupModule()
{
	UToolMenus::RegisterStartupCallback(
	   FSimpleMulticastDelegate::FDelegate::CreateRaw(
		   this, &FSlateDemoModule::RegisterMenuExtension));

	// Console command for rapid iteration during development
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("SlateShowcase.Open"),
		TEXT("Opens the Slate Showcase reference window."),
		FConsoleCommandDelegate::CreateRaw(this, &FSlateDemoModule::OpenShowcaseWindow),
		ECVF_Default
	);
}

void FSlateDemoModule::ShutdownModule()
{
	UnregisterMenuExtension();

	if (ShowcaseWindowPtr.IsValid())
	{
		ShowcaseWindowPtr->RequestDestroyWindow();
		ShowcaseWindowPtr.Reset();
	}
}

void FSlateDemoModule::RegisterMenuExtension()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus) return;

	UToolMenu* ToolsMenu = ToolMenus->ExtendMenu(ShowcaseMenuName);
	if (!ToolsMenu) return;

	FToolMenuSection& Section = ToolsMenu->FindOrAddSection(
		"SlateShowcaseSection",
		LOCTEXT("ShowcaseSection", "Showcase"));

	Section.AddMenuEntry(
		ShowcaseEntryName,
		LOCTEXT("OpenShowcase", "Slate Showcase"),
		LOCTEXT("OpenShowcaseTip", "Open the interactive Slate widget reference window."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Layout"),
		FUIAction(FExecuteAction::CreateRaw(
			this, &FSlateDemoModule::OpenShowcaseWindow))
	);
}

void FSlateDemoModule::UnregisterMenuExtension()
{
	if (UToolMenus* ToolMenus = UToolMenus::Get())
	{
		ToolMenus->RemoveEntry(ShowcaseMenuName, "SlateShowcaseSection", ShowcaseEntryName);
	}
}

void FSlateDemoModule::OpenShowcaseWindow()
{
	if (ShowcaseWindowPtr.IsValid())
	{
		ShowcaseWindowPtr->BringToFront();
		return;
	}

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("WindowTitle", "Slate Showcase — Widget Reference"))
		.ClientSize(FVector2D(1100.f, 720.f))
		.MinWidth(800.f)
		.MinHeight(500.f)
		.SupportsMaximize(true)
		.SupportsMinimize(true)
		.SizingRule(ESizingRule::UserSized)
		[
			SNew(SShowcaseWindow)
		];

	// Cache a weak handle so we can guard against duplicates
	ShowcaseWindowPtr = Window;

	// Bind to OnWindowClosed so we clear our handle when the user closes it
	Window->SetOnWindowClosed(FOnWindowClosed::CreateLambda(
		[this](const TSharedRef<SWindow>&)
		{
			ShowcaseWindowPtr.Reset();
		}
	));

	FSlateApplication::Get().AddWindow(Window);
}


#undef LOCTEXT_NAMESPACE
	

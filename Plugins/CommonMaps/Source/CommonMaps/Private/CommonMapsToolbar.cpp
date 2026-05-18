// Fill out your copyright notice in the Description page of Project Settings.


#include "CommonMapsToolbar.h"
#include "CommonMapSettings.h"
#include "ToolMenus.h"
#include "ToolMenuSection.h"

#include "FileHelpers.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Framework/Commands/UIAction.h"
#include "Textures/SlateIcon.h"

static const FName LevelEditorToolbarName = TEXT("LevelEditor.LevelEditorToolBar.PlayToolBar");

static const FName CommonMapsMenuName = TEXT("LevelEditor.CommonMapsDropdown");

void FCommonMapsToolbar::Register()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus)
	{
		return;
	}
	
	UToolMenu* Toolbar = ToolMenus->ExtendMenu(LevelEditorToolbarName);
	FToolMenuSection& Section = Toolbar->FindOrAddSection(TEXT("CommonMapsSection"));
	
	FToolMenuEntry DropdownButton = FToolMenuEntry::InitSubMenu(
		TEXT("CommonMapsDropdown"),
		NSLOCTEXT("CommonMapsPlugin", "ToolbarLabel", "Common Maps"),
		NSLOCTEXT("CommonMapsPlugin", "ToolbarTooltip", "Open a frequently used map"),
		FNewToolMenuDelegate::CreateStatic(&FCommonMapsToolbar::FillDropdownMenu),
		false,
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Levels")  
	);

	Section.AddEntry(DropdownButton);
	ToolMenus->RegisterMenu(CommonMapsMenuName);
}

void FCommonMapsToolbar::Unregister()
{
	if (UToolMenus* ToolMenus = UToolMenus::Get())
	{
		ToolMenus->UnregisterOwnerByName(TEXT("CommonMapsPlugin"));
	}
}

void FCommonMapsToolbar::FillDropdownMenu(UToolMenu* Menu)
{
	const UCommonMapSettings* Settings = UCommonMapSettings::Get();
	if (!Settings || (Settings->MapCategories.IsEmpty() && Settings->RecentMaps.IsEmpty()))
	{
		FToolMenuSection& EmptySection = Menu->AddSection(TEXT("EmptyState"));
		EmptySection.AddMenuEntry(
			TEXT("NoMaps"),
			NSLOCTEXT("CommonMapsPlugin", "NoMapsLabel", "No maps configured"),
			NSLOCTEXT("CommonMapsPlugin", "NoMapsTooltip",
				"Add maps via Edit -> Editor Preferences -> Plugins -> Common Maps"),
			FSlateIcon(),
			FUIAction()
		);
		return;
	}

	// Pinned "Recent" section — auto-managed, shown above all categories.
	if (!Settings->RecentMaps.IsEmpty())
	{
		FToolMenuSection& RecentSection = Menu->AddSection(
			TEXT("RecentMaps"),
			NSLOCTEXT("CommonMapsPlugin", "RecentLabel", "Recent")
		);

		for (const FSoftObjectPath& MapPath : Settings->RecentMaps)
		{
			if (!MapPath.IsValid())
			{
				continue;
			}

			RecentSection.AddMenuEntry(
				FName(*FString::Printf(TEXT("Recent_%s"), *MapPath.GetAssetName())),
				FText::FromString(MapPath.GetAssetName()),
				FText::FromString(MapPath.GetAssetPathString()),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FCommonMapsToolbar::OpenMap, MapPath))
			);
		}
	}

	// One section per category, so they appear as labelled groups in the dropdown.
	for (const FCommonMapCategory& Category : Settings->MapCategories)
	{
		// FName must be unique per section — sanitize the category name.
		const FName SectionName(*FString::Printf(TEXT("Category_%s"), *Category.CategoryName));
		FToolMenuSection& Section = Menu->AddSection(
			SectionName,
			FText::FromString(Category.CategoryName)  // This becomes the section header
		);

		for (const FCommonMapEntry& Entry : Category.Maps)
		{
			if (!Entry.IsValid())
			{
				continue;
			}

			const FSoftObjectPath MapPath = Entry.MapPath;

			const FAssetRegistryModule& AssetRegistry =	FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
			const bool bAssetExists = AssetRegistry.Get().GetAssetByObjectPath(FSoftObjectPath(MapPath)).IsValid();

			const FText Label = bAssetExists ? FText::FromString(Entry.GetDisplayName()) : FText::FromString(TEXT("⚠ ") + Entry.GetDisplayName());

			const FText Tooltip = bAssetExists ? FText::FromString(MapPath.GetAssetPathString()) : FText::Format(NSLOCTEXT("CommonMapsPlugin", "StaleMap", "Asset not found: {0}"), FText::FromString(MapPath.GetAssetPathString()));

			Section.AddMenuEntry(
				FName(*Entry.GetDisplayName()),
				FText::FromString(Entry.GetDisplayName()),
				FText::FromString(MapPath.GetAssetPathString()), // Tooltip = full path
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FCommonMapsToolbar::OpenMap, MapPath))
			);
		}
	}
}

void FCommonMapsToolbar::OpenMap(FSoftObjectPath MapPath)
{
	if (!MapPath.IsValid())
	{
		return;
	}

	FScopedSlowTask LoadTask(1.f,
		FText::Format(
			NSLOCTEXT("CommonMapsPlugin", "LoadingMap", "Opening {0}..."),
			FText::FromString(MapPath.GetAssetName())
		)
	);
	LoadTask.MakeDialog();
	
	const FString AssetPath = MapPath.GetAssetPathString();
	FEditorFileUtils::LoadMap(AssetPath, /*bLoadAsTemplate=*/false, /*bShowProgress=*/true);
}

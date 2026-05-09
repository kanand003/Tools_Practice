// Fill out your copyright notice in the Description page of Project Settings.


#include "CommonMapsMenuExtension.h"
#include "CommonMapSettings.h"
#include "ContentBrowserMenuContexts.h"
#include "ToolMenus.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetData.h"
#include "Framework/Commands/UIAction.h"
#include "Textures/SlateIcon.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

static const FName ContentBrowserLevelContextMenu = TEXT("ContentBrowser.AssetContextMenu.World");

void FCommonMapsMenuExtension::Register()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus)
	{
		return;
	}

	UToolMenu* Menu = ToolMenus->ExtendMenu(ContentBrowserLevelContextMenu);
	FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("CommonMapsSection"));

	// Add a sub-menu entry. The delegate fires each time the context menu opens,
	// so the category list is always fresh from current settings.
	Section.AddSubMenu(
		TEXT("AddToCommonMaps"),
		NSLOCTEXT("CommonMapsPlugin", "ContextMenuLabel", "Add to Common Maps"),
		NSLOCTEXT("CommonMapsPlugin", "ContextMenuTooltip", "Add this map to a Common Maps category"),
		FNewToolMenuDelegate::CreateStatic(&FCommonMapsMenuExtension::FillContextMenuEntry)
	);
}

void FCommonMapsMenuExtension::Unregister()
{
	if (UToolMenus* ToolMenus = UToolMenus::Get())
	{
		ToolMenus->UnregisterOwnerByName(TEXT("CommonMapsPlugin"));
	}
}

void FCommonMapsMenuExtension::FillContextMenuEntry(UToolMenu* Menu)
{
	const UContentBrowserAssetContextMenuContext* Context =
        Menu->FindContext<UContentBrowserAssetContextMenuContext>();

    if (!Context || Context->SelectedAssets.IsEmpty())
    {
        return;
    }

    // We only care about the first selected asset (Level assets, single selection)
    const FAssetData& AssetData = Context->SelectedAssets[0];
    const FSoftObjectPath MapPath(AssetData.GetSoftObjectPath());

    // --- Build one entry per category ---
    const UCommonMapSettings* Settings = UCommonMapSettings::Get();

    FToolMenuSection& Section = Menu->AddSection(TEXT("Categories"));

    if (!Settings || Settings->MapCategories.IsEmpty())
    {
        Section.AddMenuEntry(
            TEXT("NoCategories"),
            NSLOCTEXT("CommonMapsPlugin", "NoCategoriesLabel", "No categories configured"),
            NSLOCTEXT("CommonMapsPlugin", "NoCategoriesHint",
                "Add categories via Edit -> Editor Preferences -> Plugins -> Common Maps"),
            FSlateIcon(),
            FUIAction()
        );
        return;
    }

    for (const FCommonMapCategory& Category : Settings->MapCategories)
    {
        const FString CategoryName = Category.CategoryName;

        // Check if this map is already in this category — show a hint if so
        const bool bAlreadyAdded = Category.Maps.ContainsByPredicate(
            [&MapPath](const FCommonMapEntry& Entry)
            {
                return Entry.MapPath == MapPath;
            }
        );

        const FText Label = bAlreadyAdded
            ? FText::FromString(CategoryName + TEXT(" ✓"))
            : FText::FromString(CategoryName);

        const FText Tooltip = bAlreadyAdded
            ? NSLOCTEXT("CommonMapsPlugin", "AlreadyAdded", "This map is already in this category")
            : FText::Format(
                NSLOCTEXT("CommonMapsPlugin", "AddToCategory", "Add to \"{0}\""),
                FText::FromString(CategoryName));

        Section.AddMenuEntry(
            FName(*CategoryName),
            Label,
            Tooltip,
            FSlateIcon(),
            FUIAction(
                FExecuteAction::CreateStatic(&FCommonMapsMenuExtension::AddMapToCategory, MapPath, CategoryName),
                FCanExecuteAction::CreateLambda([bAlreadyAdded]() { return !bAlreadyAdded; })
            )
        );
    }
}

void FCommonMapsMenuExtension::AddMapToCategory(FSoftObjectPath MapPath, FString CategoryName)
{
	UCommonMapSettings* Settings = UCommonMapSettings::Get();
	if (!Settings)
	{
		return;
	}

	for (FCommonMapCategory& Category : Settings->MapCategories)
	{
		if (Category.CategoryName == CategoryName)
		{
			// Guard against duplicates — shouldn't happen due to FCanExecuteAction above,
			// but defensive coding is correct for editor tools
			const bool bAlreadyExists = Category.Maps.ContainsByPredicate(
				[&MapPath](const FCommonMapEntry& Entry)
				{
					return Entry.MapPath == MapPath;
				}
			);

			if (!bAlreadyExists)
			{
				Category.Maps.Add(FCommonMapEntry(MapPath));
				Settings->SaveConfig();

				FNotificationInfo Info(FText::Format(
	   NSLOCTEXT("CommonMapsPlugin", "MapAdded", "Added '{0}' to '{1}'"),
	   FText::FromString(MapPath.GetAssetName()),
	   FText::FromString(CategoryName)
   ));
				Info.ExpireDuration = 3.0f;
				Info.bUseSuccessFailIcons = true;
				FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(
					SNotificationItem::CS_Success
				);

				UE_LOG(LogTemp, Log, TEXT("CommonMapsPlugin: Added '%s' to category '%s'"),	*MapPath.GetAssetName(), *CategoryName);
			}
			else
			{
				FNotificationInfo Info(FText::Format(NSLOCTEXT("CommonMapsPlugin", "MapAlreadyExists", "'{0}' is already in '{1}'"),
										FText::FromString(MapPath.GetAssetName()),
										FText::FromString(CategoryName)
									));
				Info.ExpireDuration = 2.0f;
				Info.bUseSuccessFailIcons = true;
				FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(
					SNotificationItem::CS_Fail
				);
			}
			break;
		}
	}
}

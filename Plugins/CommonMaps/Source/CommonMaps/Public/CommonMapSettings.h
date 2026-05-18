// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CommonMapSettings.generated.h"

/**
 * 
 */
USTRUCT(meta = (TitleProperty = "MapPath"))
struct FCommonMapEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere,Config, Category = "Common Maps", meta = (DisplayName = "Map Asset"))
	FSoftObjectPath MapPath;

	UPROPERTY(EditAnywhere,Config, Category = "Common Maps")
	FString DisplayName;

	FCommonMapEntry() {}
	FCommonMapEntry(const FSoftObjectPath& InPath, const FString& InDisplayName = TEXT(""))
		: MapPath(InPath), DisplayName(InDisplayName)
	{}

	FString GetDisplayName() const
	{
		if (!DisplayName.IsEmpty())
		{
			return DisplayName;
		}
		return MapPath.GetAssetName();
	}

	bool IsValid() const { return MapPath.IsValid(); }
};
/**
 * A named group of maps, e.g. "Mission-1" or "Development Maps".
 * Displayed as a section header in the toolbar dropdown.
 */
USTRUCT(meta = (TitleProperty = "CategoryName"))
struct FCommonMapCategory
{
	GENERATED_BODY()
 
	/** The section header name shown in the toolbar dropdown and context menu submenu. */
	UPROPERTY(EditAnywhere, Config, Category = "Common Maps", meta = (DisplayName = "Category Name"))
	FString CategoryName;
 
	/** The maps belonging to this category. */
	UPROPERTY(EditAnywhere, Config, Category = "Common Maps", meta = (DisplayName = "Maps"))
	TArray<FCommonMapEntry> Maps;
 
	FCommonMapCategory() {}
	explicit FCommonMapCategory(const FString& InName) : CategoryName(InName) {}
};
UCLASS(Config = EditorPerProjectUserSettings, defaultconfig, meta = (DisplayName = "Common Maps"))
class COMMONMAPS_API UCommonMapSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UCommonMapSettings();

	/**
	 * All map categories. Add, remove and reorder them freely in Editor Preferences.
	 * Changes are saved immediately when you modify them in the panel.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Common Maps", meta = (DisplayName = "Map Categories", NoResetToDefault))
	TArray<FCommonMapCategory> MapCategories;

	/**
	 * Auto-populated list of recently opened maps (managed by the plugin, not editable directly).
	 * Shown as a pinned section at the top of the toolbar dropdown.
	 */
	UPROPERTY(Config)
	TArray<FSoftObjectPath> RecentMaps;

	/** Maximum number of recent maps to remember. */
	UPROPERTY(EditAnywhere, Config, Category = "Common Maps", meta = (DisplayName = "Max Recent Maps", ClampMin = 1, ClampMax = 20))
	int32 MaxRecentMaps = 5;

	/**
	 * Pushes MapPath to the front of RecentMaps, deduplicates, trims to MaxRecentMaps, and saves.
	 * Safe to call from any editor context.
	 */
	void PushRecentMap(const FSoftObjectPath& MapPath);

	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

	virtual FText GetSectionText() const override
	{
		return NSLOCTEXT("CommonMapsPlugin", "SettingsSectionText", "Common Maps");
	}
 
	virtual FText GetSectionDescription() const override
	{
		return NSLOCTEXT("CommonMapsPlugin", "SettingsSectionDesc",
			"Configure map categories for quick access from the toolbar and Content Browser.");
	}
	static UCommonMapSettings* Get()
	{
		return GetMutableDefault<UCommonMapSettings>();
	}
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);
		SaveConfig();
	}
#endif
};

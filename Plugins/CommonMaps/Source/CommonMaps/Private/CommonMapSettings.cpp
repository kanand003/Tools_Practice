// Fill out your copyright notice in the Description page of Project Settings.


#include "CommonMapSettings.h"

UCommonMapSettings::UCommonMapSettings()
{
	FCommonMapCategory DefaultCategory(TEXT("My Maps"));
	MapCategories.Add(DefaultCategory);
}

void UCommonMapSettings::PushRecentMap(const FSoftObjectPath& MapPath)
{
	if (!MapPath.IsValid())
	{
		return;
	}

	// Remove any existing entry for this map so it moves to the front
	RecentMaps.Remove(MapPath);
	RecentMaps.Insert(MapPath, 0);

	// Trim to the configured limit
	if (RecentMaps.Num() > MaxRecentMaps)
	{
		RecentMaps.SetNum(MaxRecentMaps);
	}

	SaveConfig();
}

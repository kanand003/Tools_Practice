// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FCommonMapsMenuExtension
{
	static void Register();
	static void Unregister();

private:

	/** Builds the "Add to Common Maps" submenu. Called each time the context menu opens. */
	static void FillContextMenuEntry(UToolMenu* Menu);

	/** Adds the selected map to the given category and saves. */
	static void AddMapToCategory(FSoftObjectPath MapPath, FString CategoryName);

	/** Builds the "Remove from Common Maps" submenu. Called each time the context menu opens. */
	static void FillRemoveContextMenuEntry(UToolMenu* Menu);

	/** Removes the selected map from the given category and saves. */
	static void RemoveMapFromCategory(FSoftObjectPath MapPath, FString CategoryName);

	/** Removes all maps from the given category and saves. */
	static void ClearCategory(FString CategoryName);
};

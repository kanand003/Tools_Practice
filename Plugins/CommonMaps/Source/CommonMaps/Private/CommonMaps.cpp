// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonMaps.h"
#include "CommonMapsToolbar.h"
#include "CommonMapsMenuExtension.h"
#include "CommonMapSettings.h"
#include "Modules/ModuleManager.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Misc/PackageName.h"

void FCommonMapsModule::StartupModule()
{
	FCommonMapsToolbar::Register();
	FCommonMapsMenuExtension::Register();

	MapChangedHandle = FEditorDelegates::MapChange.AddRaw(this, &FCommonMapsModule::OnMapChanged);

	UE_LOG(LogTemp, Log, TEXT("CommonMapsPlugin: StartupModule"));
}

void FCommonMapsModule::ShutdownModule()
{
	FEditorDelegates::MapChange.Remove(MapChangedHandle);

	FCommonMapsToolbar::Unregister();
	FCommonMapsMenuExtension::Unregister();
	UE_LOG(LogTemp, Log, TEXT("CommonMapsPlugin: ShutdownModule"));
}

void FCommonMapsModule::OnMapChanged(uint32 ChangeFlags)
{
	// MapChangeEventFlags::NewMap == 1 — only track actual map loads, not teardowns
	if (!(ChangeFlags & MapChangeEventFlags::NewMap))
	{
		return;
	}

	if (!GEditor)
	{
		return;
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		return;
	}

	// Build a FSoftObjectPath from the world's package + asset name (e.g. /Game/Maps/MyMap.MyMap)
	const FString PackageName = World->GetOutermost()->GetName();
	const FString AssetName = FPackageName::GetLongPackageAssetName(PackageName);
	const FSoftObjectPath MapPath(PackageName + TEXT(".") + AssetName);

	UCommonMapSettings::Get()->PushRecentMap(MapPath);

	UE_LOG(LogTemp, Log, TEXT("CommonMapsPlugin: Recorded recent map '%s'"), *AssetName);
}

IMPLEMENT_MODULE(FCommonMapsModule, CommonMaps)
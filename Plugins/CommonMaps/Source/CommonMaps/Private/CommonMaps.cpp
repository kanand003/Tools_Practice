// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonMaps.h"
#include "CommonMapsToolbar.h"
#include "CommonMapsMenuExtension.h"
#include "Modules/ModuleManager.h"


void FCommonMapsModule::StartupModule()
{
	FCommonMapsToolbar::Register();
	FCommonMapsMenuExtension::Register();
	UE_LOG(LogTemp, Log, TEXT("CommonMapsPlugin: StartupModule"));
}

void FCommonMapsModule::ShutdownModule()
{
	FCommonMapsToolbar::Unregister();
	FCommonMapsMenuExtension::Unregister();
	UE_LOG(LogTemp, Log, TEXT("CommonMapsPlugin: ShutdownModule"));
}

IMPLEMENT_MODULE(FCommonMapsModule, CommonMaps)
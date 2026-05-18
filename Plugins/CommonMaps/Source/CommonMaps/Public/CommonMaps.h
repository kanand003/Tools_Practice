// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FCommonMapsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	/** Fires whenever the editor loads a new map. Updates RecentMaps in settings. */
	void OnMapChanged(uint32 ChangeFlags);

	/** Handle used to unbind OnMapChanged in ShutdownModule. */
	FDelegateHandle MapChangedHandle;
};

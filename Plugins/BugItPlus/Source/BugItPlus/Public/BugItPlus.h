// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Delegates/Delegate.h"
#include "Math/Transform.h"

class IConsoleObject;

DECLARE_DELEGATE_RetVal_TwoParams(bool, FBugItPlusEditorCaptureDelegate, FString& /*OutMapPackageName*/,
                                  FTransform& /*OutTransform*/);
DECLARE_DELEGATE_RetVal_TwoParams(bool, FBugItPlusEditorJumpDelegate, const FString& /*MapPackageName*/,
                                  const FTransform& /*Transform*/);
DECLARE_DELEGATE_OneParam(FBugItPlusEditorNotifyDelegate, const FString& /*Message*/);

class BUGITPLUS_API FBugItPlusModule : public IModuleInterface
{
public:
	/** Bound by BugItPlusEditor when loaded. Fills OutMapPackageName/OutTransform from the active level viewport. Returns false if unbound or capture f*/
	static FBugItPlusEditorCaptureDelegate EditorCaptureDelegate;
	/** Bound by BugItPlusEditor when loaded. Moves the level viewport (loading MapPackageName first if it differs from the open level). Returns false iunbound. */
	static FBugItPlusEditorJumpDelegate EditorJumpDelegate;
	/** Bound by BugItPlusEditor when loaded. Shows an editor notification toast. No-op if unbound (e.g. packaged build). */
	static FBugItPlusEditorNotifyDelegate EditorNotifyDelegate;
	
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	IConsoleObject* BugItPlusCommand = nullptr;
	IConsoleObject* BugItGoPlusCommand = nullptr;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#include "BugItPlus.h"
#include "BugItPlusCommands.h"
#include "HAL/IConsoleManager.h"

#define LOCTEXT_NAMESPACE "FBugItPlusModule"

FBugItPlusEditorCaptureDelegate FBugItPlusModule::EditorCaptureDelegate;
FBugItPlusEditorJumpDelegate FBugItPlusModule::EditorJumpDelegate;

void FBugItPlusModule::StartupModule()
{
	BugItPlusCommand = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("BugItPlus"),
		TEXT(
			"Captures the current location, map, and a screenshot, then copies a BugItGoPlus string to the clipboard. Usage: BugItPlus <description>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&FBugItPlusCommands::HandleBugItPlus));
}

void FBugItPlusModule::ShutdownModule()
{
	if (BugItPlusCommand)
	{
		IConsoleManager::Get().UnregisterConsoleObject(BugItPlusCommand);
		BugItPlusCommand = nullptr;
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBugItPlusModule, BugItPlus)

# BugItPlus Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Ship a `BugItPlus` UE5 plugin that adds two console commands, `BugItPlus`
and `BugItGoPlus`, giving the native `BugIt`/`BugItGo` workflow clipboard-copy on the
in-game path and cross-map jump on both paths — without touching or requiring changes
to the native commands.

**Architecture:** Two modules. `BugItPlus` (Runtime) owns both console commands, the
go-string format, and everything that works without `UnrealEd` (PIE/packaged capture,
screenshot, clipboard copy, cross-map travel+teleport). `BugItPlusEditor` (Editor) binds
two static delegates exposed by the Runtime module to handle the "editor, not playing"
case (level-viewport camera capture/move, map load via `FEditorFileUtils::LoadMap`).
If the Editor module isn't loaded (packaged build), the delegates stay unbound and the
Runtime module's PIE/packaged path handles everything.

**Tech Stack:** UE 5.7 C++, `IConsoleManager` console commands, UE Automation Test
framework (`IMPLEMENT_SIMPLE_AUTOMATION_TEST`) for pure-logic unit tests.

## Global Constraints

- Engine: UE 5.7 (this project's `EngineAssociation`).
- Plugin lives at `Plugins/BugItPlus/`, two modules: `BugItPlus` (`Type: Runtime`,
  `LoadingPhase: Default`) and `BugItPlusEditor` (`Type: Editor`,
  `LoadingPhase: PostEngineInit`).
- Only two new console commands are added: `BugItPlus`, `BugItGoPlus`. Never override
  or subclass anything to intercept native `BugIt`/`BugItGo` — those stay untouched.
- The `BugItPlus` Runtime module must never depend on `UnrealEd`, `LevelEditor`, or any
  other editor-only module.
- The Editor module binds `FBugItPlusModule::EditorCaptureDelegate` and
  `EditorJumpDelegate` via `BindStatic` (no captured object lifetime) in
  `StartupModule()`, and calls `Unbind()` on both in `ShutdownModule()`.
- The go-string format is plugin-owned: `"<MapPackageName> X Y Z Pitch Yaw Roll"`. It
  is NOT compatible with vanilla `BugIt`'s go-string (which has no map name) — always
  parse it with `FBugItPlusGoString`, never with engine helpers like
  `GetFVECTORSpaceDelimited`.
- Storage: `Saved/BugItPlus/<PlatformName>/<Description>/`, mirroring vanilla's own
  `Saved/BugIt/<PlatformName>/<Description>/` layout one folder over (kept separate
  because the report format differs).
- Map package names must always be passed through `UWorld::RemovePIEPrefix()` before
  being stored, compared, or written to a go-string — PIE renames the world's package
  to `/Game/Maps/UEDPIE_0_MyMap`, and comparing that raw string against a package name
  captured outside PIE (or in a different PIE session) would falsely detect "different
  map" every time.
- No terminal build steps from Claude/an agent. This project's convention (see
  `Plugins/CommonMaps/CLAUDE.md`) is that the user builds via Rider and verifies in the
  UE5 Editor. Every task's verification step is something the user/executing engineer
  does in Rider + the Editor (Output Log, `Window > Test Automation`), not a shell
  command Claude runs.
- Never edit files under any `.claude/worktrees/` copy. Always edit real source under
  `Plugins/BugItPlus/Source/...`.
- Run `GenerateProjectFiles.bat` in the project root after Task 1 creates the new
  modules, before building in Rider for the first time.

---

### Task 1: Plugin scaffolding

**Files:**
- Create: `Plugins/BugItPlus/BugItPlus.uplugin`
- Create: `Plugins/BugItPlus/Source/BugItPlus/BugItPlus.Build.cs`
- Create: `Plugins/BugItPlus/Source/BugItPlus/Public/BugItPlusModule.h`
- Create: `Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusModule.cpp`
- Create: `Plugins/BugItPlus/Source/BugItPlusEditor/BugItPlusEditor.Build.cs`
- Create: `Plugins/BugItPlus/Source/BugItPlusEditor/Public/BugItPlusEditorModule.h`
- Create: `Plugins/BugItPlus/Source/BugItPlusEditor/Private/BugItPlusEditorModule.cpp`
- Modify: `Tools_Practice.uproject`

**Interfaces:**
- Produces: `FBugItPlusModule` (empty `IModuleInterface`, `BugItPlus` module) and
  `FBugItPlusEditorModule` (empty `IModuleInterface`, `BugItPlusEditor` module). Both
  are extended by later tasks — nothing outside this task depends on their contents yet.

- [ ] **Step 1: Create the `.uplugin` file**

```json
{
	"FileVersion": 3,
	"Version": 1,
	"VersionName": "1.0",
	"FriendlyName": "BugItPlus",
	"Description": "Adds clipboard copy (in-game path) and cross-map jump (both paths) to BugIt/BugItGo via new BugItPlus/BugItGoPlus console commands.",
	"Category": "Other",
	"CreatedBy": "Anand Kumar",
	"CreatedByURL": "",
	"DocsURL": "",
	"MarketplaceURL": "",
	"CanContainContent": false,
	"IsBetaVersion": false,
	"IsExperimentalVersion": false,
	"Installed": false,
	"Modules": [
		{
			"Name": "BugItPlus",
			"Type": "Runtime",
			"LoadingPhase": "Default"
		},
		{
			"Name": "BugItPlusEditor",
			"Type": "Editor",
			"LoadingPhase": "PostEngineInit"
		}
	]
}
```

- [ ] **Step 2: Create the Runtime module's `Build.cs`**

```csharp
// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BugItPlus : ModuleRules
{
	public BugItPlus(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"ApplicationCore"
			}
			);
	}
}
```

- [ ] **Step 3: Create the Runtime module skeleton**

`Plugins/BugItPlus/Source/BugItPlus/Public/BugItPlusModule.h`:
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FBugItPlusModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
```

`Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusModule.cpp`:
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "BugItPlusModule.h"

void FBugItPlusModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("BugItPlus: StartupModule"));
}

void FBugItPlusModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("BugItPlus: ShutdownModule"));
}

IMPLEMENT_MODULE(FBugItPlusModule, BugItPlus)
```

- [ ] **Step 4: Create the Editor module's `Build.cs`**

```csharp
// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BugItPlusEditor : ModuleRules
{
	public BugItPlusEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"UnrealEd",
				"BugItPlus"
			}
			);
	}
}
```

- [ ] **Step 5: Create the Editor module skeleton**

`Plugins/BugItPlus/Source/BugItPlusEditor/Public/BugItPlusEditorModule.h`:
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FBugItPlusEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
```

`Plugins/BugItPlus/Source/BugItPlusEditor/Private/BugItPlusEditorModule.cpp`:
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "BugItPlusEditorModule.h"

void FBugItPlusEditorModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("BugItPlusEditor: StartupModule"));
}

void FBugItPlusEditorModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("BugItPlusEditor: ShutdownModule"));
}

IMPLEMENT_MODULE(FBugItPlusEditorModule, BugItPlusEditor)
```

- [ ] **Step 6: Register the plugin in the `.uproject`**

In `Tools_Practice.uproject`, add to the `"Plugins"` array (alongside the existing
`ModelingToolsEditorMode`, `SlateDemo`, `CommonMaps` entries):

```json
		{
			"Name": "BugItPlus",
			"Enabled": true
		}
```

- [ ] **Step 7: Regenerate project files**

Run `GenerateProjectFiles.bat` in the project root (`D:\ProgrammingWorkspace\UnrealEngineProjects\Tools_Practice\`)
so Rider picks up the two new modules.

- [ ] **Step 8: Build and verify in the Editor**

Build the project in Rider, then launch the Unreal Editor. Open the Output Log and
filter for `BugItPlus`. Expected: two log lines appear during Editor startup —
`BugItPlus: StartupModule` and `BugItPlusEditor: StartupModule`.

- [ ] **Step 9: Commit**

```bash
git add Plugins/BugItPlus Tools_Practice.uproject
git commit -m "BugItPlus: scaffold plugin with Runtime and Editor modules"
```

---

### Task 2: Go-string build/parse utility

**Files:**
- Create: `Plugins/BugItPlus/Source/BugItPlus/Public/BugItPlusGoString.h`
- Create: `Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusGoString.cpp`
- Test: `Plugins/BugItPlus/Source/BugItPlus/Private/Tests/BugItPlusGoStringTest.cpp`

**Interfaces:**
- Consumes: nothing from earlier tasks.
- Produces: `FBugItPlusGoString::Build(const FString& MapPackageName, const FVector& Location, const FRotator& Rotation) -> FString`
  and `FBugItPlusGoString::Parse(const FString& GoString, FString& OutMapPackageName, FVector& OutLocation, FRotator& OutRotation) -> bool`.
  Later tasks (4, 6) call both of these.

- [ ] **Step 1: Write the failing tests**

`Plugins/BugItPlus/Source/BugItPlus/Private/Tests/BugItPlusGoStringTest.cpp`:
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "BugItPlusGoString.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBugItPlusGoStringBuildTest, "BugItPlus.GoString.Build", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FBugItPlusGoStringBuildTest::RunTest(const FString& Parameters)
{
	const FString Result = FBugItPlusGoString::Build(TEXT("/Game/Maps/ThirdPersonMap"), FVector(100.0f, 200.0f, 300.0f), FRotator(10.0f, 20.0f, 30.0f));

	TestEqual(TEXT("Go-string matches expected format"), Result, FString(TEXT("/Game/Maps/ThirdPersonMap 100.000000 200.000000 300.000000 10.000000 20.000000 30.000000")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBugItPlusGoStringParseRoundTripTest, "BugItPlus.GoString.ParseRoundTrip", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FBugItPlusGoStringParseRoundTripTest::RunTest(const FString& Parameters)
{
	const FString MapPackageName = TEXT("/Game/Maps/ThirdPersonMap");
	const FVector Location(100.0f, 200.0f, 300.0f);
	const FRotator Rotation(10.0f, 20.0f, 30.0f);

	const FString GoString = FBugItPlusGoString::Build(MapPackageName, Location, Rotation);

	FString ParsedMapPackageName;
	FVector ParsedLocation;
	FRotator ParsedRotation;
	const bool bParsed = FBugItPlusGoString::Parse(GoString, ParsedMapPackageName, ParsedLocation, ParsedRotation);

	TestTrue(TEXT("Parse succeeds"), bParsed);
	TestEqual(TEXT("Map package name round-trips"), ParsedMapPackageName, MapPackageName);
	TestEqual(TEXT("Location round-trips"), ParsedLocation, Location);
	TestEqual(TEXT("Rotation round-trips"), ParsedRotation, Rotation);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBugItPlusGoStringParseRejectsMalformedTest, "BugItPlus.GoString.ParseRejectsMalformed", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FBugItPlusGoStringParseRejectsMalformedTest::RunTest(const FString& Parameters)
{
	FString ParsedMapPackageName;
	FVector ParsedLocation;
	FRotator ParsedRotation;

	const bool bParsed = FBugItPlusGoString::Parse(TEXT("not enough tokens"), ParsedMapPackageName, ParsedLocation, ParsedRotation);

	TestFalse(TEXT("Parse rejects a malformed go-string"), bParsed);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
```

- [ ] **Step 2: Verify it fails to compile**

Build in Rider. Expected: compile error — `FBugItPlusGoString` is undeclared (the
header doesn't exist yet). This is the "red" step for C++: no test runner to fail
against until the code compiles.

- [ ] **Step 3: Write the header**

`Plugins/BugItPlus/Source/BugItPlus/Public/BugItPlusGoString.h`:
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Builds and parses the BugItGoPlus payload string: "<MapPackageName> X Y Z Pitch Yaw Roll".
 * This is a plugin-owned format, not compatible with vanilla BugIt's go-string (which has no map name).
 */
class BUGITPLUS_API FBugItPlusGoString
{
public:
	static FString Build(const FString& MapPackageName, const FVector& Location, const FRotator& Rotation);

	static bool Parse(const FString& GoString, FString& OutMapPackageName, FVector& OutLocation, FRotator& OutRotation);
};
```

- [ ] **Step 4: Write the implementation**

`Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusGoString.cpp`:
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "BugItPlusGoString.h"

FString FBugItPlusGoString::Build(const FString& MapPackageName, const FVector& Location, const FRotator& Rotation)
{
	return FString::Printf(TEXT("%s %f %f %f %f %f %f"),
		*MapPackageName,
		Location.X, Location.Y, Location.Z,
		Rotation.Pitch, Rotation.Yaw, Rotation.Roll);
}

bool FBugItPlusGoString::Parse(const FString& GoString, FString& OutMapPackageName, FVector& OutLocation, FRotator& OutRotation)
{
	TArray<FString> Tokens;
	GoString.ParseIntoArrayWS(Tokens);

	if (Tokens.Num() != 7)
	{
		return false;
	}

	OutMapPackageName = Tokens[0];
	OutLocation.X = FCString::Atof(*Tokens[1]);
	OutLocation.Y = FCString::Atof(*Tokens[2]);
	OutLocation.Z = FCString::Atof(*Tokens[3]);
	OutRotation.Pitch = FCString::Atof(*Tokens[4]);
	OutRotation.Yaw = FCString::Atof(*Tokens[5]);
	OutRotation.Roll = FCString::Atof(*Tokens[6]);

	return true;
}
```

- [ ] **Step 5: Build and run the tests**

Build in Rider, launch the Editor, open `Window > Test Automation`, search
`BugItPlus.GoString`, select all three, click "Start Tests". Expected: all three pass
(green).

- [ ] **Step 6: Commit**

```bash
git add Plugins/BugItPlus/Source/BugItPlus/Public/BugItPlusGoString.h Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusGoString.cpp Plugins/BugItPlus/Source/BugItPlus/Private/Tests/BugItPlusGoStringTest.cpp
git commit -m "BugItPlus: add go-string build/parse utility"
```

---

### Task 3: Editor hook delegates

**Files:**
- Modify: `Plugins/BugItPlus/Source/BugItPlus/Public/BugItPlusModule.h`
- Modify: `Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusModule.cpp`
- Test: `Plugins/BugItPlus/Source/BugItPlus/Private/Tests/BugItPlusDelegateTest.cpp`

**Interfaces:**
- Consumes: nothing from earlier tasks.
- Produces: `FBugItPlusModule::EditorCaptureDelegate` (type `FBugItPlusEditorCaptureDelegate`,
  signature `bool(FString& OutMapPackageName, FTransform& OutTransform)`) and
  `FBugItPlusModule::EditorJumpDelegate` (type `FBugItPlusEditorJumpDelegate`, signature
  `bool(const FString& MapPackageName, const FTransform& Transform)`). Task 4 calls
  `EditorCaptureDelegate`; Task 5 binds both from the Editor module; Task 6 calls
  `EditorJumpDelegate`.

- [ ] **Step 1: Write the failing tests**

`Plugins/BugItPlus/Source/BugItPlus/Private/Tests/BugItPlusDelegateTest.cpp`:
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "BugItPlusModule.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBugItPlusEditorCaptureDelegateTest, "BugItPlus.Delegates.EditorCapture", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FBugItPlusEditorCaptureDelegateTest::RunTest(const FString& Parameters)
{
	// Save/restore rather than assert "unbound by default" - once BugItPlusEditor is
	// loaded (Task 5+), this delegate is legitimately bound for real at test time.
	const FBugItPlusEditorCaptureDelegate SavedDelegate = FBugItPlusModule::EditorCaptureDelegate;

	FBugItPlusModule::EditorCaptureDelegate.Unbind();
	TestFalse(TEXT("Unbound after explicit Unbind"), FBugItPlusModule::EditorCaptureDelegate.IsBound());

	FBugItPlusModule::EditorCaptureDelegate.BindLambda([](FString& OutMapPackageName, FTransform& OutTransform)
	{
		OutMapPackageName = TEXT("/Game/Maps/TestMap");
		OutTransform = FTransform::Identity;
		return true;
	});

	TestTrue(TEXT("Bound after BindLambda"), FBugItPlusModule::EditorCaptureDelegate.IsBound());

	FString MapPackageName;
	FTransform Transform;
	const bool bResult = FBugItPlusModule::EditorCaptureDelegate.Execute(MapPackageName, Transform);

	TestTrue(TEXT("Execute returns true"), bResult);
	TestEqual(TEXT("Execute fills map package name"), MapPackageName, FString(TEXT("/Game/Maps/TestMap")));

	FBugItPlusModule::EditorCaptureDelegate = SavedDelegate;

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBugItPlusEditorJumpDelegateTest, "BugItPlus.Delegates.EditorJump", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FBugItPlusEditorJumpDelegateTest::RunTest(const FString& Parameters)
{
	const FBugItPlusEditorJumpDelegate SavedDelegate = FBugItPlusModule::EditorJumpDelegate;

	FBugItPlusModule::EditorJumpDelegate.Unbind();
	TestFalse(TEXT("Unbound after explicit Unbind"), FBugItPlusModule::EditorJumpDelegate.IsBound());

	bool bWasCalled = false;
	FBugItPlusModule::EditorJumpDelegate.BindLambda([&bWasCalled](const FString& MapPackageName, const FTransform& Transform)
	{
		bWasCalled = true;
		return true;
	});

	TestTrue(TEXT("Bound after BindLambda"), FBugItPlusModule::EditorJumpDelegate.IsBound());

	const bool bResult = FBugItPlusModule::EditorJumpDelegate.Execute(TEXT("/Game/Maps/TestMap"), FTransform::Identity);

	TestTrue(TEXT("Execute returns true"), bResult);
	TestTrue(TEXT("Bound lambda was invoked"), bWasCalled);

	FBugItPlusModule::EditorJumpDelegate = SavedDelegate;

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
```

- [ ] **Step 2: Verify it fails to compile**

Build in Rider. Expected: compile error — `FBugItPlusModule::EditorCaptureDelegate`
and `EditorJumpDelegate` are undeclared.

- [ ] **Step 3: Add the delegate declarations to the module header**

`Plugins/BugItPlus/Source/BugItPlus/Public/BugItPlusModule.h`:
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Delegates/Delegate.h"
#include "Math/Transform.h"

DECLARE_DELEGATE_RetVal_TwoParams(bool, FBugItPlusEditorCaptureDelegate, FString& /*OutMapPackageName*/, FTransform& /*OutTransform*/);
DECLARE_DELEGATE_RetVal_TwoParams(bool, FBugItPlusEditorJumpDelegate, const FString& /*MapPackageName*/, const FTransform& /*Transform*/);

class FBugItPlusModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Bound by BugItPlusEditor when loaded. Fills OutMapPackageName/OutTransform from the active level viewport. Returns false if unbound or capture fails. */
	static FBugItPlusEditorCaptureDelegate EditorCaptureDelegate;

	/** Bound by BugItPlusEditor when loaded. Moves the level viewport (loading MapPackageName first if it differs from the open level). Returns false if unbound. */
	static FBugItPlusEditorJumpDelegate EditorJumpDelegate;
};
```

- [ ] **Step 4: Define the static delegate storage**

`Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusModule.cpp`:
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "BugItPlusModule.h"

FBugItPlusEditorCaptureDelegate FBugItPlusModule::EditorCaptureDelegate;
FBugItPlusEditorJumpDelegate FBugItPlusModule::EditorJumpDelegate;

void FBugItPlusModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("BugItPlus: StartupModule"));
}

void FBugItPlusModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("BugItPlus: ShutdownModule"));
}

IMPLEMENT_MODULE(FBugItPlusModule, BugItPlus)
```

- [ ] **Step 5: Build and run the tests**

Build in Rider, run `BugItPlus.Delegates.EditorCapture` and `BugItPlus.Delegates.EditorJump`
via `Window > Test Automation`. Expected: both pass.

- [ ] **Step 6: Commit**

```bash
git add Plugins/BugItPlus/Source/BugItPlus/Public/BugItPlusModule.h Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusModule.cpp Plugins/BugItPlus/Source/BugItPlus/Private/Tests/BugItPlusDelegateTest.cpp
git commit -m "BugItPlus: add editor hook delegates"
```

---

### Task 4: `BugItPlus` command — capture, screenshot, clipboard copy

**Files:**
- Create: `Plugins/BugItPlus/Source/BugItPlus/Public/BugItPlusCommands.h`
- Create: `Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusCommands.cpp`
- Modify: `Plugins/BugItPlus/Source/BugItPlus/Public/BugItPlusModule.h`
- Modify: `Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusModule.cpp`

**Interfaces:**
- Consumes: `FBugItPlusGoString::Build` (Task 2), `FBugItPlusModule::EditorCaptureDelegate` (Task 3).
- Produces: the live `BugItPlus` console command. Task 6 registers its sibling
  `BugItGoPlus` the same way, in the same module.

- [ ] **Step 1: Declare the command handler**

`Plugins/BugItPlus/Source/BugItPlus/Public/BugItPlusCommands.h`:
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FBugItPlusCommands
{
public:
	static void HandleBugItPlus(const TArray<FString>& Args, UWorld* World);
	static void HandleBugItGoPlus(const TArray<FString>& Args, UWorld* World);
};
```

(`HandleBugItGoPlus` is declared now so the header doesn't change again in Task 6, but
it is only implemented starting Task 6 — leave it undefined for now; nothing calls it
yet, so leaving it unimplemented does not affect linking.)

- [ ] **Step 2: Implement the capture handler**

`Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusCommands.cpp`:
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "BugItPlusCommands.h"
#include "BugItPlusModule.h"
#include "BugItPlusGoString.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

static bool ResolveCaptureTransform(UWorld* World, FString& OutMapPackageName, FVector& OutLocation, FRotator& OutRotation, APlayerController*& OutPlayerController)
{
	OutPlayerController = World ? GEngine->GetFirstLocalPlayerController(World) : nullptr;

	if (OutPlayerController)
	{
		FVector ViewLocation;
		FRotator ViewRotation;
		OutPlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

		if (APawn* Pawn = OutPlayerController->GetPawn())
		{
			ViewLocation = Pawn->GetActorLocation();
		}

		OutLocation = ViewLocation;
		OutRotation = ViewRotation;
		OutMapPackageName = UWorld::RemovePIEPrefix(World->GetOutermost()->GetName());
		return true;
	}

	FTransform EditorTransform;
	if (FBugItPlusModule::EditorCaptureDelegate.IsBound() && FBugItPlusModule::EditorCaptureDelegate.Execute(OutMapPackageName, EditorTransform))
	{
		OutLocation = EditorTransform.GetLocation();
		OutRotation = EditorTransform.Rotator();
		return true;
	}

	return false;
}

void FBugItPlusCommands::HandleBugItPlus(const TArray<FString>& Args, UWorld* World)
{
	const FString Description = Args.Num() > 0 ? FString::Join(Args, TEXT(" ")) : TEXT("BugItPlus");

	FString MapPackageName;
	FVector Location;
	FRotator Rotation;
	APlayerController* PlayerController = nullptr;

	if (!ResolveCaptureTransform(World, MapPackageName, Location, Rotation, PlayerController))
	{
		UE_LOG(LogTemp, Warning, TEXT("BugItPlus: no valid location source found (not in PIE/packaged with a pawn, and no editor-side capture handler bound)."));
		return;
	}

	const FString GoString = FBugItPlusGoString::Build(MapPackageName, Location, Rotation);

	const FString ReportDir = FString::Printf(TEXT("%sBugItPlus/%s/%s/"), *FPaths::ProjectSavedDir(), FPlatformProperties::PlatformName(), *Description);
	IFileManager::Get().MakeDirectory(*ReportDir, /*Tree=*/true);

	if (PlayerController)
	{
		FString ScreenShotFile;
		FFileHelper::GenerateNextBitmapFilename(ReportDir / Description, TEXT("png"), ScreenShotFile);
		PlayerController->ConsoleCommand(FString::Printf(TEXT("BUGSCREENSHOTWITHHUDINFO %s"), *ScreenShotFile));
	}

	const FString ReportFile = ReportDir / Description + TEXT(".txt");
	const FString ReportContents = FString::Printf(TEXT("Description: %s\nMapPackageName: %s\n%s\n"), *Description, *MapPackageName, *GoString);
	FFileHelper::SaveStringToFile(ReportContents, *ReportFile);

	FPlatformApplicationMisc::ClipboardCopy(*GoString);

	UE_LOG(LogTemp, Log, TEXT("BugItPlus: report saved to %s, BugItGoPlus string copied to clipboard: %s"), *ReportDir, *GoString);
}
```

- [ ] **Step 3: Register the console command**

Modify `Plugins/BugItPlus/Source/BugItPlus/Public/BugItPlusModule.h` to add a member
and forward declaration:
```cpp
class IConsoleObject;

class FBugItPlusModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Bound by BugItPlusEditor when loaded. Fills OutMapPackageName/OutTransform from the active level viewport. Returns false if unbound or capture fails. */
	static FBugItPlusEditorCaptureDelegate EditorCaptureDelegate;

	/** Bound by BugItPlusEditor when loaded. Moves the level viewport (loading MapPackageName first if it differs from the open level). Returns false if unbound. */
	static FBugItPlusEditorJumpDelegate EditorJumpDelegate;

private:
	IConsoleObject* BugItPlusCommand = nullptr;
};
```

Modify `Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusModule.cpp`:
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "BugItPlusModule.h"
#include "BugItPlusCommands.h"
#include "HAL/IConsoleManager.h"

FBugItPlusEditorCaptureDelegate FBugItPlusModule::EditorCaptureDelegate;
FBugItPlusEditorJumpDelegate FBugItPlusModule::EditorJumpDelegate;

void FBugItPlusModule::StartupModule()
{
	BugItPlusCommand = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("BugItPlus"),
		TEXT("Captures the current location, map, and a screenshot, then copies a BugItGoPlus string to the clipboard. Usage: BugItPlus <description>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&FBugItPlusCommands::HandleBugItPlus));

	UE_LOG(LogTemp, Log, TEXT("BugItPlus: StartupModule"));
}

void FBugItPlusModule::ShutdownModule()
{
	if (BugItPlusCommand)
	{
		IConsoleManager::Get().UnregisterConsoleObject(BugItPlusCommand);
		BugItPlusCommand = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("BugItPlus: ShutdownModule"));
}

IMPLEMENT_MODULE(FBugItPlusModule, BugItPlus)
```

- [ ] **Step 4: Verify — PIE/packaged branch**

Build in Rider. Launch PIE with a pawn possessed. Open the console and run:
`BugItPlus test one`. Expected:
- Output Log shows `BugItPlus: report saved to .../Saved/BugItPlus/<Platform>/test one/, BugItGoPlus string copied to clipboard: ...`
- `Saved/BugItPlus/<Platform>/test one/` contains a `.png` and a `.txt` file
- Pasting into a text editor confirms the clipboard holds a string like
  `/Game/Maps/ThirdPersonMap 123.000000 45.000000 ...`

- [ ] **Step 5: Verify — editor-only branch aborts safely**

Stop PIE. In the Editor's Cmd line (bottom of Output Log), run `BugItPlus test two`.
Expected: `BugItPlus: no valid location source found...` warning, no crash, no files
written (Task 5 makes this branch actually work — for now, aborting cleanly is the
correct behavior since `EditorCaptureDelegate` is not bound to anything yet).

- [ ] **Step 6: Commit**

```bash
git add Plugins/BugItPlus/Source/BugItPlus/Public/BugItPlusCommands.h Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusCommands.cpp Plugins/BugItPlus/Source/BugItPlus/Public/BugItPlusModule.h Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusModule.cpp
git commit -m "BugItPlus: implement BugItPlus capture command"
```

---

### Task 5: `BugItPlusEditor` — capture and same-map jump handlers

**Files:**
- Modify: `Plugins/BugItPlus/Source/BugItPlusEditor/Public/BugItPlusEditorModule.h`
- Modify: `Plugins/BugItPlus/Source/BugItPlusEditor/Private/BugItPlusEditorModule.cpp`

**Interfaces:**
- Consumes: `FBugItPlusModule::EditorCaptureDelegate` / `EditorJumpDelegate` (Task 3).
- Produces: live editor-side behavior for both delegates. `HandleEditorJump` here only
  handles the same-map case (moves the viewport, does not check for a map mismatch) —
  Task 8 extends it to load a different map first.

- [ ] **Step 1: Bind both delegates and implement the handlers**

`Plugins/BugItPlus/Source/BugItPlusEditor/Public/BugItPlusEditorModule.h`:
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FBugItPlusEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	static bool HandleEditorCapture(FString& OutMapPackageName, FTransform& OutTransform);
	static bool HandleEditorJump(const FString& MapPackageName, const FTransform& Transform);
};
```

`Plugins/BugItPlus/Source/BugItPlusEditor/Private/BugItPlusEditorModule.cpp`:
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "BugItPlusEditorModule.h"
#include "BugItPlusModule.h"
#include "Editor.h"
#include "LevelEditorViewport.h"
#include "Engine/World.h"

void FBugItPlusEditorModule::StartupModule()
{
	FBugItPlusModule::EditorCaptureDelegate.BindStatic(&FBugItPlusEditorModule::HandleEditorCapture);
	FBugItPlusModule::EditorJumpDelegate.BindStatic(&FBugItPlusEditorModule::HandleEditorJump);

	UE_LOG(LogTemp, Log, TEXT("BugItPlusEditor: StartupModule"));
}

void FBugItPlusEditorModule::ShutdownModule()
{
	FBugItPlusModule::EditorCaptureDelegate.Unbind();
	FBugItPlusModule::EditorJumpDelegate.Unbind();

	UE_LOG(LogTemp, Log, TEXT("BugItPlusEditor: ShutdownModule"));
}

bool FBugItPlusEditorModule::HandleEditorCapture(FString& OutMapPackageName, FTransform& OutTransform)
{
	if (!GCurrentLevelEditingViewportClient || !GEditor)
	{
		return false;
	}

	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	if (!EditorWorld)
	{
		return false;
	}

	OutMapPackageName = EditorWorld->GetOutermost()->GetName();
	OutTransform = FTransform(GCurrentLevelEditingViewportClient->GetViewRotation(), GCurrentLevelEditingViewportClient->GetViewLocation());

	return true;
}

bool FBugItPlusEditorModule::HandleEditorJump(const FString& MapPackageName, const FTransform& Transform)
{
	if (!GEditor)
	{
		return false;
	}

	const FVector Location = Transform.GetLocation();
	FRotator Rotation = Transform.Rotator();
	Rotation.Roll = 0.0f;

	for (FLevelEditorViewportClient* ViewportClient : GEditor->GetLevelViewportClients())
	{
		ViewportClient->SetViewLocation(Location);
		ViewportClient->SetViewRotation(Rotation);
	}

	GEditor->RedrawLevelEditingViewports();

	return true;
}
```

- [ ] **Step 2: Verify — editor capture now works**

Build in Rider, launch the Editor (not PIE), position the level viewport somewhere
memorable, and run `BugItPlus test three` from the Editor's Cmd line. Expected:
`Saved/BugItPlus/<Platform>/test three/test three.txt` is written (no screenshot —
`PlayerController` is null in this branch), and the clipboard holds a go-string with
the current level's package name and the viewport's location/rotation.

- [ ] **Step 3: Verify — same-map jump moves the viewport**

Move the viewport somewhere else, then run `BugItGoPlus` in the Editor's Cmd line with
the string from Step 2 — this will currently log a warning since `BugItGoPlus` isn't
registered as a console command yet (that's Task 6). Skip ahead to Task 6 to actually
exercise this; for now, confirm via a temporary manual call that `HandleEditorJump`
compiles correctly by checking the build succeeds with no errors.

- [ ] **Step 4: Commit**

```bash
git add Plugins/BugItPlus/Source/BugItPlusEditor/Public/BugItPlusEditorModule.h Plugins/BugItPlus/Source/BugItPlusEditor/Private/BugItPlusEditorModule.cpp
git commit -m "BugItPlus: implement editor-side capture and same-map jump"
```

---

### Task 6: `BugItGoPlus` command — same-map branch, both contexts

**Files:**
- Modify: `Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusCommands.cpp`
- Modify: `Plugins/BugItPlus/Source/BugItPlus/Public/BugItPlusModule.h`
- Modify: `Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusModule.cpp`
- Modify: `Plugins/BugItPlus/Source/BugItPlus/BugItPlus.Build.cs`

**Interfaces:**
- Consumes: `FBugItPlusGoString::Parse` (Task 2), `FBugItPlusModule::EditorJumpDelegate`
  (Task 3), `HandleEditorJump` from Task 5.
- Produces: the live `BugItGoPlus` console command, same-map case only. Task 7 fills in
  the different-map/PIE-packaged branch; Task 8 fills in the different-map/editor branch.
  Both later tasks modify the same `else` block this task creates.

- [ ] **Step 1: Add `CheatManager` to the Build.cs dependency list**

`GameFramework/CheatManager.h` lives in the `Engine` module, already a dependency —
no `Build.cs` change is actually required. (Confirming this now so the next step's
`#include` doesn't surprise you with a missing-dependency error.)

- [ ] **Step 2: Implement `HandleBugItGoPlus`**

Add to `Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusCommands.cpp` (add the new
`#include`s at the top, and the new functions at the bottom):

```cpp
#include "GameFramework/CheatManager.h"
```

```cpp
static void TeleportPlayerController(APlayerController* PlayerController, const FVector& Location, const FRotator& Rotation)
{
	if (UCheatManager* CheatManager = PlayerController->CheatManager)
	{
		CheatManager->Ghost();
	}

	if (APawn* Pawn = PlayerController->GetPawn())
	{
		Pawn->TeleportTo(Location, Rotation);
		Pawn->FaceRotation(Rotation, 0.0f);
	}

	PlayerController->SetControlRotation(Rotation);

	if (UCheatManager* CheatManager = PlayerController->CheatManager)
	{
		CheatManager->Ghost();
	}
}

void FBugItPlusCommands::HandleBugItGoPlus(const TArray<FString>& Args, UWorld* World)
{
	if (Args.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BugItGoPlus: usage: BugItGoPlus \"<MapPackageName> X Y Z Pitch Yaw Roll\""));
		return;
	}

	const FString GoString = FString::Join(Args, TEXT(" "));

	FString MapPackageName;
	FVector Location;
	FRotator Rotation;
	if (!FBugItPlusGoString::Parse(GoString, MapPackageName, Location, Rotation))
	{
		UE_LOG(LogTemp, Warning, TEXT("BugItGoPlus: could not parse go-string '%s'"), *GoString);
		return;
	}

	APlayerController* PlayerController = World ? GEngine->GetFirstLocalPlayerController(World) : nullptr;

	if (PlayerController)
	{
		const FString CurrentMapPackageName = UWorld::RemovePIEPrefix(World->GetOutermost()->GetName());

		if (CurrentMapPackageName == MapPackageName)
		{
			TeleportPlayerController(PlayerController, Location, Rotation);
			UE_LOG(LogTemp, Log, TEXT("BugItGoPlus: teleported to %s"), *GoString);
		}
		else
		{
			// Task 7 fills in the cross-map travel-then-teleport branch here.
			UE_LOG(LogTemp, Warning, TEXT("BugItGoPlus: target map '%s' differs from current map '%s' - cross-map jump not yet implemented"), *MapPackageName, *CurrentMapPackageName);
		}

		return;
	}

	FTransform Transform(Rotation, Location);
	if (!FBugItPlusModule::EditorJumpDelegate.IsBound() || !FBugItPlusModule::EditorJumpDelegate.Execute(MapPackageName, Transform))
	{
		UE_LOG(LogTemp, Warning, TEXT("BugItGoPlus: no editor-side jump handler bound"));
	}
}
```

- [ ] **Step 3: Register the console command**

Modify `Plugins/BugItPlus/Source/BugItPlus/Public/BugItPlusModule.h`:
```cpp
private:
	IConsoleObject* BugItPlusCommand = nullptr;
	IConsoleObject* BugItGoPlusCommand = nullptr;
```

Modify `Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusModule.cpp`:
```cpp
void FBugItPlusModule::StartupModule()
{
	BugItPlusCommand = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("BugItPlus"),
		TEXT("Captures the current location, map, and a screenshot, then copies a BugItGoPlus string to the clipboard. Usage: BugItPlus <description>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&FBugItPlusCommands::HandleBugItPlus));

	BugItGoPlusCommand = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("BugItGoPlus"),
		TEXT("Teleports to a location captured by BugItPlus, switching maps first if needed. Usage: BugItGoPlus \"<MapPackageName> X Y Z Pitch Yaw Roll\""),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&FBugItPlusCommands::HandleBugItGoPlus));

	UE_LOG(LogTemp, Log, TEXT("BugItPlus: StartupModule"));
}

void FBugItPlusModule::ShutdownModule()
{
	if (BugItPlusCommand)
	{
		IConsoleManager::Get().UnregisterConsoleObject(BugItPlusCommand);
		BugItPlusCommand = nullptr;
	}

	if (BugItGoPlusCommand)
	{
		IConsoleManager::Get().UnregisterConsoleObject(BugItGoPlusCommand);
		BugItGoPlusCommand = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("BugItPlus: ShutdownModule"));
}
```

- [ ] **Step 4: Verify — PIE same-map teleport**

Build in Rider, launch PIE, run `BugItPlus start`, move the pawn away, then run
`BugItGoPlus "<pasted clipboard string>"`. Expected: pawn snaps back to the captured
location/rotation, Output Log shows `BugItGoPlus: teleported to ...`.

- [ ] **Step 5: Verify — editor same-map jump**

Stop PIE. In the Editor's Cmd line, run `BugItPlus start`, move the level viewport,
then run `BugItGoPlus "<pasted clipboard string>"`. Expected: the viewport snaps back
to the captured position (this is the scenario Task 5, Step 3 deferred — exercise it
now).

- [ ] **Step 6: Commit**

```bash
git add Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusCommands.cpp Plugins/BugItPlus/Source/BugItPlus/Public/BugItPlusModule.h Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusModule.cpp
git commit -m "BugItPlus: implement BugItGoPlus same-map teleport"
```

---

### Task 7: Cross-map jump — PIE/packaged

**Files:**
- Modify: `Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusCommands.cpp`

**Interfaces:**
- Consumes: `TeleportPlayerController` (Task 6, same file).
- Produces: fills in the `else` branch Task 6 left as a "not yet implemented" warning.

- [ ] **Step 1: Add the deferred-teleport plumbing and cross-map branch**

Add to the top of `Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusCommands.cpp`:
```cpp
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"
```

Add near the top of the file, alongside the other file-static helpers (before
`HandleBugItGoPlus`):
```cpp
// File-static pending state for the one-shot post-load teleport triggered by a
// cross-map BugItGoPlus. OpenLevel tears down the current World, but this plain
// file-static memory survives that teardown untouched.
static FDelegateHandle GPendingTeleportHandle;
static FVector GPendingTeleportLocation;
static FRotator GPendingTeleportRotation;

static void HandlePostLoadMapTeleport(UWorld* LoadedWorld)
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(GPendingTeleportHandle);
	GPendingTeleportHandle.Reset();

	if (!LoadedWorld)
	{
		return;
	}

	if (APlayerController* PlayerController = GEngine->GetFirstLocalPlayerController(LoadedWorld))
	{
		TeleportPlayerController(PlayerController, GPendingTeleportLocation, GPendingTeleportRotation);
		UE_LOG(LogTemp, Log, TEXT("BugItGoPlus: cross-map jump complete, teleported after loading %s"), *LoadedWorld->GetOutermost()->GetName());
	}
}
```

Replace the `else` block inside `HandleBugItGoPlus` (the one Task 6 left logging
"not yet implemented"):
```cpp
		else
		{
			GPendingTeleportLocation = Location;
			GPendingTeleportRotation = Rotation;
			GPendingTeleportHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddStatic(&HandlePostLoadMapTeleport);

			UE_LOG(LogTemp, Log, TEXT("BugItGoPlus: loading %s for cross-map jump"), *MapPackageName);
			UGameplayStatics::OpenLevel(World, FName(*MapPackageName));
		}
```

- [ ] **Step 2: Verify — cross-map PIE jump**

This requires two maps in the project (e.g. the default `ThirdPersonMap` plus any
second map — duplicate one if only one exists). Launch PIE on Map A, run
`BugItPlus here`, open Map B (`open MapB` in console, or stop PIE and relaunch on Map
B), run `BugItGoPlus "<pasted string>"`. Expected: the level travels to Map A, then the
pawn teleports to the saved location once loading completes. Output Log shows both the
"loading" and "cross-map jump complete" lines.

- [ ] **Step 3: Commit**

```bash
git add Plugins/BugItPlus/Source/BugItPlus/Private/BugItPlusCommands.cpp
git commit -m "BugItPlus: implement cross-map jump for PIE/packaged"
```

---

### Task 8: Cross-map jump — editor

**Files:**
- Modify: `Plugins/BugItPlus/Source/BugItPlusEditor/Private/BugItPlusEditorModule.cpp`

**Interfaces:**
- Consumes: nothing new from earlier tasks — extends `HandleEditorJump` from Task 5.
- Produces: the final piece of the design — `HandleEditorJump` now handles both
  same-map and different-map cases.

- [ ] **Step 1: Extend `HandleEditorJump` to load a different map first**

Add `#include "FileHelpers.h"` to
`Plugins/BugItPlus/Source/BugItPlusEditor/Private/BugItPlusEditorModule.cpp`, then
replace the `HandleEditorJump` function body:

```cpp
bool FBugItPlusEditorModule::HandleEditorJump(const FString& MapPackageName, const FTransform& Transform)
{
	if (!GEditor)
	{
		return false;
	}

	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	const FString CurrentMapPackageName = EditorWorld ? EditorWorld->GetOutermost()->GetName() : FString();

	if (CurrentMapPackageName != MapPackageName)
	{
		if (!FEditorFileUtils::LoadMap(MapPackageName, /*LoadAsTemplate=*/false, /*bShowProgress=*/true))
		{
			UE_LOG(LogTemp, Warning, TEXT("BugItGoPlus: failed to load map '%s'"), *MapPackageName);
			return false;
		}
	}

	const FVector Location = Transform.GetLocation();
	FRotator Rotation = Transform.Rotator();
	Rotation.Roll = 0.0f;

	for (FLevelEditorViewportClient* ViewportClient : GEditor->GetLevelViewportClients())
	{
		ViewportClient->SetViewLocation(Location);
		ViewportClient->SetViewRotation(Rotation);
	}

	GEditor->RedrawLevelEditingViewports();

	return true;
}
```

- [ ] **Step 2: Verify — cross-map editor jump**

With the same two test maps from Task 7: open Map A in the editor (not PIE), run
`BugItPlus here` from the Cmd line, open Map B in the editor, run `BugItGoPlus
"<pasted string>"`. Expected: the editor loads Map A and the viewport snaps to the
saved position.

- [ ] **Step 3: Verify — full regression pass**

Re-run the verification steps from Tasks 4, 5, 6, and 7 once more end-to-end (PIE
same-map, editor same-map, PIE cross-map, editor cross-map) to confirm nothing broke.
Also re-run all three `BugItPlus.GoString.*` and both `BugItPlus.Delegates.*`
automation tests via `Window > Test Automation` — expect all five green.

- [ ] **Step 4: Commit**

```bash
git add Plugins/BugItPlus/Source/BugItPlusEditor/Private/BugItPlusEditorModule.cpp
git commit -m "BugItPlus: implement cross-map jump for editor"
```

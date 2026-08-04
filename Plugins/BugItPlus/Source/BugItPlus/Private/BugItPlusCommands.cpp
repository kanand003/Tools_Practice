// Fill out your copyright notice in the Description page of Project Settings.


#include "BugItPlusCommands.h"
#include "BugItPlus.h"
#include "BugItPlusGoString.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CheatManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

static bool ResolveCaptureTransform(UWorld* World, FString& OutMapPackageName, FVector& OutLocation,
                                    FRotator& OutRotation, APlayerController*& OutPlayerController)
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
	if (FBugItPlusModule::EditorCaptureDelegate.IsBound() && FBugItPlusModule::EditorCaptureDelegate.Execute(
		OutMapPackageName, EditorTransform))
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
		UE_LOG(LogTemp, Warning,
		       TEXT(
			       "BugItPlus: no valid location source found (not in PIE/packaged with a pawn, and no editor-side capture handler bound)."
		       ));
		return;
	}

	const FString GoString = FBugItPlusGoString::Build(MapPackageName, Location, Rotation);

	const FString ReportDir = FString::Printf(TEXT("%sBugItPlus/%s/%s/"), *FPaths::ProjectSavedDir(), ANSI_TO_TCHAR(FPlatformProperties::PlatformName()), *Description);
	IFileManager::Get().MakeDirectory(*ReportDir, /*Tree=*/true);

	if (PlayerController)
	{
		FString ScreenShotFile;
		FFileHelper::GenerateNextBitmapFilename(ReportDir / Description, TEXT("png"), ScreenShotFile);
		PlayerController->ConsoleCommand(FString::Printf(TEXT("BUGSCREENSHOTWITHHUDINFO %s"), *ScreenShotFile));
	}

	const FString ReportFile = ReportDir / Description + TEXT(".txt");
	const FString ReportContents = FString::Printf(
		TEXT("Description: %s\nMapPackageName: %s\n%s\n"), *Description, *MapPackageName, *GoString);
	FFileHelper::SaveStringToFile(ReportContents, *ReportFile);

	FPlatformApplicationMisc::ClipboardCopy(*GoString);

	UE_LOG(LogTemp, Log, TEXT("BugItPlus: report saved to %s, BugItGoPlus string copied to clipboard: %s"), *ReportDir,
	       *GoString);
}

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

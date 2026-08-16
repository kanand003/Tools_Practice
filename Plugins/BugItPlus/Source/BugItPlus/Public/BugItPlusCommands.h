// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/IDelegateInstance.h"

class FBugItPlusCommands
{
public:
	static void HandleBugItPlus(const TArray<FString>& Args, UWorld* World);
	static void HandleBugItGoPlus(const TArray<FString>& Args, UWorld* World);

private:
	static void TeleportPlayerController(APlayerController* PlayerController, const FVector& Location, const FRotator& Rotation);
	static void HandlePostLoadMapTeleport(UWorld* LoadedWorld);

	static FDelegateHandle PendingTeleportHandle;
	static FVector PendingTeleportLocation;
	static FRotator PendingTeleportRotation;
};

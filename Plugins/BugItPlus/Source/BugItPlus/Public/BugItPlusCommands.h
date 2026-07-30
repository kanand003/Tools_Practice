// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FBugItPlusCommands
{
public:
	static void HandleBugItPlus(const TArray<FString>& Args, UWorld* World);
	static void HandleBugItGoPlus(const TArray<FString>& Args, UWorld* World);
};

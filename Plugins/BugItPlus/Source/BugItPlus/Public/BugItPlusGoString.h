// Fill out your copyright notice in the Description page of Project Settings.

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

// Fill out your copyright notice in the Description page of Project Settings.


#include "BugItPlusGoString.h"


FString FBugItPlusGoString::Build(const FString& MapPackageName, const FVector& Location, const FRotator& Rotation)
{
	return FString::Printf(TEXT("%s %f %f %f %f %f %f"),
			  *MapPackageName, Location.X, Location.Y, Location.Z, Rotation.Pitch, Rotation.Yaw, Rotation.Roll);
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

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FCommonMapsToolbar
{
	/** Call from FCommonMapsPluginModule::StartupModule */
	static void Register();

	/** Call from FCommonMapsPluginModule::ShutdownModule */
	static void Unregister();

private:

	/** Builds the dropdown menu contents. Called every time the menu opens. */
	static void FillDropdownMenu(UToolMenu* Menu);

	/** Opens the map at the given soft path. */
	static void OpenMap(FSoftObjectPath MapPath);
};
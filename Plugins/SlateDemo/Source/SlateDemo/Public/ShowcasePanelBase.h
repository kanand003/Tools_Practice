// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * Abstract interface every showcase panel must satisfy.
 *
 * Design intent: The main window holds a TArray<TSharedRef<IShowcasePanel>>.
 * It calls GetPanelDisplayName() to build the tab strip and BuildPanel() to
 * populate each SWidgetSwitcher slot. Adding a new domain == one new file,
 * zero changes to SShowcaseWindow.
 */

class IShowcasePanelBase
{
	public:
	virtual ~IShowcasePanelBase() = default;

	/** Stable identity key — used as the SWidgetSwitcher slot index key. */
	virtual FName GetPanelName() const = 0;

	/** Human-readable label shown in the tab strip. */
	virtual FText GetPanelDisplayName() const = 0;

	/**
	 * Build and return the panel widget. Called once at window construction.
	 * Keep expensive work here, not in the constructor.
	 */
	virtual TSharedRef<SWidget> BuildPanel() = 0;
};

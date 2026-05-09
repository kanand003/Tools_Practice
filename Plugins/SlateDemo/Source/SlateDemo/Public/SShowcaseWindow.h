// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Templates/SharedPointer.h"
#include "ShowcasePanelBase.h"

class SWidgetSwitcher;
class SButton;
/**
 * SShowcaseWindow
 *
 * Top-level compound widget. Owns:
 *   - A tab strip (row of SButtons acting as tab selectors)
 *   - An SWidgetSwitcher that swaps panel content
 *   - The canonical panel registry (TArray of IShowcasePanel)
 *
 * Architecture notes:
 *   - Panels are registered in Construct() in display order.
 *   - Tab strip buttons are generated from the registry — no manual wiring.
 *   - Active tab index is stored as a member; tab buttons use TAttribute
 *     lambdas to derive their visual state from it (no separate bool per tab).
 */
class SLATEDEMO_API SShowcaseWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShowcaseWindow)
		{
		}

	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);
private:
	// -------------------------------------------------------------------------
	// Panel registry
	// -------------------------------------------------------------------------
	TArray<TUniquePtr<IShowcasePanelBase>> Panels;

	void RegisterPanels();

	// -------------------------------------------------------------------------
	// Tab strip
	// -------------------------------------------------------------------------
	int32 ActivePanelIndex = 0;

	TSharedRef<SWidget> BuildTabStrip();

	FReply OnTabClicked(int32 PanelIndex);

	// Per-tab visual state derived via TAttribute lambdas — no booleans stored
	FSlateColor GetTabForeground(int32 PanelIndex) const;
	const FSlateBrush* GetTabBorderImage(int32 PanelIndex) const;

	// -------------------------------------------------------------------------
	// Content switcher
	// -------------------------------------------------------------------------
	TSharedPtr<SWidgetSwitcher> ContentSwitcher;

	TSharedRef<SWidget> BuildContentArea();
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShowcasePanelBase.h"

/**
 * Demonstrates all major Slate layout primitives:
 *   SVerticalBox, SHorizontalBox, SGridPanel, SSplitter,
 *   SScrollBox, SUniformGridPanel, SWrapBox
 *
 * Each sub-section is wrapped in a titled SBorder so the demo is
 * self-documenting inside the editor.
 */
class FLayoutPanel : public IShowcasePanelBase
{
public:
	virtual FName   GetPanelName()        const override;
	virtual FText   GetPanelDisplayName() const override;
	virtual TSharedRef<SWidget> BuildPanel() override;

private:
	TSharedRef<SWidget> BuildVHBoxSection();
	TSharedRef<SWidget> BuildGridPanelSection();
	TSharedRef<SWidget> BuildSplitterSection();
	TSharedRef<SWidget> BuildUniformGridSection();
	TSharedRef<SWidget> BuildWrapBoxSection();

	// Utility: creates a coloured placeholder box with a label
	static TSharedRef<SWidget> MakeColorBox(
		const FText& Label,
		const FLinearColor& Color,
		float Width  = 80.f,
		float Height = 50.f
	);
};

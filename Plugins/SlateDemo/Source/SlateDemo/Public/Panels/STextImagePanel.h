// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShowcasePanelBase.h"

/**
 * Demonstrates text and image widgets:
 *   STextBlock (size, color, font overrides),
 *   SRichTextBlock (inline bold/italic/color markup),
 *   SImage (brush from style set, tinting, sizing),
 *   SBorder (background brush, padding, content alignment)
 */
class FTextImagePanel : public IShowcasePanelBase
{
public:
	virtual FName   GetPanelName()        const override;
	virtual FText   GetPanelDisplayName() const override;
	virtual TSharedRef<SWidget> BuildPanel() override;

private:
	TSharedRef<SWidget> BuildTextBlockSection();
	TSharedRef<SWidget> BuildRichTextSection();
	TSharedRef<SWidget> BuildImageSection();
	TSharedRef<SWidget> BuildBorderSection();
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "SShowcaseWindow.h"
#include "Panels/SLayoutPanel.h"
#include "Panels/STextImagePanel.h"

#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "SShowcaseWindow"

void SShowcaseWindow::Construct(const FArguments& InArgs)
{
	RegisterPanels();

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(0.f)
		[
			SNew(SVerticalBox)

			// ---- Tab strip ----
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				BuildTabStrip()
			]

			// ---- Thin separator line ----
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("Menu.Separator"))
				.Padding(0.f)
				[
					SNew(SBox).HeightOverride(1.f)
				]
			]

			// ---- Panel content ----
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				BuildContentArea()
			]
		]
	];
}

void SShowcaseWindow::RegisterPanels()
{
	Panels.Add(MakeUnique<FLayoutPanel>());
	Panels.Add(MakeUnique<FTextImagePanel>());
}

TSharedRef<SWidget> SShowcaseWindow::BuildTabStrip()
{
	TSharedRef<SHorizontalBox> Strip = SNew(SHorizontalBox);

	for (int32 i = 0; i < Panels.Num(); ++i)
	{
		const int32 CapturedIndex = i; // capture by value for lambdas
		const FText Label = Panels[i]->GetPanelDisplayName();

		Strip->AddSlot()
		.AutoWidth()
		.Padding(0.f)
		[
			SNew(SBorder)
			// Active tab gets a bottom-border highlight; inactive stays flat.
			// TAttribute lambda: re-evaluated each frame, zero stored state.
			.BorderImage_Lambda([this, CapturedIndex]() -> const FSlateBrush*
			{
				return GetTabBorderImage(CapturedIndex);
			})
			.Padding(FMargin(16.f, 10.f))
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.OnClicked_Lambda([this, CapturedIndex]() -> FReply
				{
					return OnTabClicked(CapturedIndex);
				})
				.ContentPadding(FMargin(0.f))
				[
					SNew(STextBlock)
					.Text(Label)
					.TextStyle(FAppStyle::Get(), "SmallButtonText")
					.ColorAndOpacity_Lambda([this, CapturedIndex]() -> FSlateColor
					{
						return GetTabForeground(CapturedIndex);
					})
				]
			]
		];
	}

	return Strip;
}

FReply SShowcaseWindow::OnTabClicked(int32 PanelIndex)
{
	ActivePanelIndex = PanelIndex;
	if (ContentSwitcher.IsValid())
	{
		ContentSwitcher->SetActiveWidgetIndex(PanelIndex);
	}
	return FReply::Handled();
}

FSlateColor SShowcaseWindow::GetTabForeground(int32 PanelIndex) const
{
	const bool bActive = (PanelIndex == ActivePanelIndex);
	return bActive
		? FSlateColor(FLinearColor::White)
		: FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.f));
}

const FSlateBrush* SShowcaseWindow::GetTabBorderImage(int32 PanelIndex) const
{
	const bool bActive = (PanelIndex == ActivePanelIndex);
	// "WhiteTexture" lets us control colour entirely via BorderBackgroundColor.
	// Active tab: subtle blue tint. Inactive: transparent.
	static FSlateBrush ActiveBrush   = *FAppStyle::GetBrush("WhiteTexture");
	static FSlateBrush InactiveBrush = *FCoreStyle::Get().GetBrush("NoBrush");

	if (bActive)
	{
		ActiveBrush.TintColor = FSlateColor(FLinearColor(0.15f, 0.35f, 0.65f, 0.3f));
		return &ActiveBrush;
	}
	return &InactiveBrush;
}

TSharedRef<SWidget> SShowcaseWindow::BuildContentArea()
{
	TSharedRef<SWidgetSwitcher> Switcher = SNew(SWidgetSwitcher);
	ContentSwitcher = Switcher;

	for (const TUniquePtr<IShowcasePanelBase>& Panel : Panels)
	{
		if (!Panel) continue;
		Switcher->AddSlot()
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.Padding(16.f)
			[
				Panel->BuildPanel()
			]
		];
	}

	Switcher->SetActiveWidgetIndex(0);
	return Switcher;
}


#undef LOCTEXT_NAMESPACE


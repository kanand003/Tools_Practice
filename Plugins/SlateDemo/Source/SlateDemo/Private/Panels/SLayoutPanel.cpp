// Fill out your copyright notice in the Description page of Project Settings.


#include "Panels/SLayoutPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"
#include "Styling/StyleColors.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SWrapBox.h"

#define LOCTEXT_NAMESPACE "FTextImagePanel"

FName FLayoutPanel::GetPanelName() const        { return FName("Layout"); }
FText FLayoutPanel::GetPanelDisplayName() const { return LOCTEXT("TabLabel", "Layout"); }

TSharedRef<SWidget> FLayoutPanel::BuildPanel()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 24.f)
		[ BuildVHBoxSection() ]

		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 24.f)
		[ BuildGridPanelSection() ]

		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 24.f)
		[ BuildSplitterSection() ]

		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 24.f)
		[ BuildUniformGridSection() ]

		+ SVerticalBox::Slot().AutoHeight()
		[ BuildWrapBoxSection() ];
}

TSharedRef<SWidget> FLayoutPanel::BuildVHBoxSection()
{
	 return SNew(SBorder)
    .BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
    .Padding(12.f)
    [
        SNew(SVerticalBox)

        // Section title
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("VHBoxTitle", "SVerticalBox / SHorizontalBox — slot fill & alignment"))
            .TextStyle(FAppStyle::Get(), "NormalText.Important")
        ]

        // Row 1: AutoHeight slots
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f)
        [
            SNew(STextBlock).Text(LOCTEXT("AutoHeight", "AutoHeight slots — each takes only what it needs:"))
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(2.f)
            [ MakeColorBox(LOCTEXT("A","A"), FLinearColor(0.8f,0.2f,0.2f)) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(2.f)
            [ MakeColorBox(LOCTEXT("B","B"), FLinearColor(0.2f,0.6f,0.2f)) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(2.f)
            [ MakeColorBox(LOCTEXT("C","C"), FLinearColor(0.2f,0.4f,0.8f)) ]
        ]

        // Row 2: FillWidth slots (proportional)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 2.f)
        [
            SNew(STextBlock).Text(LOCTEXT("FillWidth", "FillWidth(N) — proportional fill (1 : 2 : 1):"))
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(2.f)
            [ MakeColorBox(LOCTEXT("Fill1","1"), FLinearColor(0.8f,0.4f,0.1f), -1.f, 40.f) ]
            + SHorizontalBox::Slot().FillWidth(2.f).Padding(2.f)
            [ MakeColorBox(LOCTEXT("Fill2","2"), FLinearColor(0.6f,0.1f,0.6f), -1.f, 40.f) ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(2.f)
            [ MakeColorBox(LOCTEXT("Fill3","1"), FLinearColor(0.1f,0.5f,0.6f), -1.f, 40.f) ]
        ]

        // Row 3: HAlign / VAlign on a VBox slot
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 2.f)
        [
            SNew(STextBlock).Text(LOCTEXT("Align", "VAlign + HAlign on SVerticalBox slot:"))
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(2.f)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            MakeColorBox(LOCTEXT("Center","HAlign_Center"), FLinearColor(0.3f,0.6f,0.9f), 180.f, 36.f)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(2.f)
        .HAlign(HAlign_Right)
        [
            MakeColorBox(LOCTEXT("Right","HAlign_Right"), FLinearColor(0.9f,0.5f,0.2f), 120.f, 36.f)
        ]
    ];
}

TSharedRef<SWidget> FLayoutPanel::BuildGridPanelSection()
{
    return SNew(SBorder)
   .BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
   .Padding(12.f)
   [
       SNew(SVerticalBox)
       + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
       [
           SNew(STextBlock)
           .Text(LOCTEXT("GridTitle", "SGridPanel — column stretching & multi-column span"))
           .TextStyle(FAppStyle::Get(), "NormalText.Important")
       ]
       + SVerticalBox::Slot().AutoHeight()
       [
           SNew(SGridPanel)
           .FillColumn(0, 1.f)
           .FillColumn(1, 2.f)
           .FillColumn(2, 1.f)

           // Row 0
           + SGridPanel::Slot(0, 0).Padding(2.f)
           [ MakeColorBox(LOCTEXT("G00","Col 0\nFill 1"), FLinearColor(0.8f,0.2f,0.2f), -1.f, 50.f) ]
           + SGridPanel::Slot(1, 0).Padding(2.f)
           [ MakeColorBox(LOCTEXT("G10","Col 1\nFill 2"), FLinearColor(0.2f,0.6f,0.2f), -1.f, 50.f) ]
           + SGridPanel::Slot(2, 0).Padding(2.f)
           [ MakeColorBox(LOCTEXT("G20","Col 2\nFill 1"), FLinearColor(0.2f,0.4f,0.8f), -1.f, 50.f) ]

           // Row 1 — colspan 2 via ColumnSpan
           + SGridPanel::Slot(0, 1).ColumnSpan(2).Padding(2.f)
           [ MakeColorBox(LOCTEXT("Span","ColumnSpan(2)"), FLinearColor(0.6f,0.3f,0.7f), -1.f, 40.f) ]
           + SGridPanel::Slot(2, 1).Padding(2.f)
           [ MakeColorBox(LOCTEXT("G21","Solo"), FLinearColor(0.7f,0.5f,0.1f), -1.f, 40.f) ]
       ]
   ];
}

TSharedRef<SWidget> FLayoutPanel::BuildSplitterSection()
{
    return SNew(SBorder)
    .BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
    .Padding(12.f)
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("SplitterTitle", "SSplitter — drag to resize panels (SizeRule::FractionOfParent vs FixedSize)"))
            .TextStyle(FAppStyle::Get(), "NormalText.Important")
        ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SBox).HeightOverride(140.f)
            [
                SNew(SSplitter)
                .Orientation(Orient_Horizontal)

                + SSplitter::Slot()
                .Value(0.3f)
                .SizeRule(SSplitter::ESizeRule::FractionOfParent)
                [
                    MakeColorBox(LOCTEXT("Left","Left\n30%"), FLinearColor(0.7f,0.2f,0.2f), -1.f, -1.f)
                ]

                + SSplitter::Slot()
                .Value(0.4f)
                .SizeRule(SSplitter::ESizeRule::FractionOfParent)
                [
                    MakeColorBox(LOCTEXT("Mid","Middle\n40%"), FLinearColor(0.2f,0.55f,0.2f), -1.f, -1.f)
                ]

                + SSplitter::Slot()
                .Value(0.3f)
                .SizeRule(SSplitter::ESizeRule::FractionOfParent)
                [
                    MakeColorBox(LOCTEXT("Right","Right\n30%"), FLinearColor(0.2f,0.3f,0.8f), -1.f, -1.f)
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> FLayoutPanel::BuildUniformGridSection()
{
    return SNew(SBorder)
   .BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
   .Padding(12.f)
   [
       SNew(SVerticalBox)
       + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
       [
           SNew(STextBlock)
           .Text(LOCTEXT("UniformTitle", "SUniformGridPanel — equal-width cells, no manual sizing"))
           .TextStyle(FAppStyle::Get(), "NormalText.Important")
       ]
       + SVerticalBox::Slot().AutoHeight()
       [
           SNew(SUniformGridPanel)
           .SlotPadding(FMargin(4.f))

           + SUniformGridPanel::Slot(0, 0)
           [ MakeColorBox(LOCTEXT("U00","0,0"), FLinearColor(0.8f,0.2f,0.4f), -1.f, 50.f) ]
           + SUniformGridPanel::Slot(1, 0)
           [ MakeColorBox(LOCTEXT("U10","1,0"), FLinearColor(0.4f,0.7f,0.2f), -1.f, 50.f) ]
           + SUniformGridPanel::Slot(2, 0)
           [ MakeColorBox(LOCTEXT("U20","2,0"), FLinearColor(0.2f,0.5f,0.9f), -1.f, 50.f) ]
           + SUniformGridPanel::Slot(0, 1)
           [ MakeColorBox(LOCTEXT("U01","0,1"), FLinearColor(0.9f,0.6f,0.1f), -1.f, 50.f) ]
           + SUniformGridPanel::Slot(1, 1)
           [ MakeColorBox(LOCTEXT("U11","1,1"), FLinearColor(0.5f,0.1f,0.8f), -1.f, 50.f) ]
           + SUniformGridPanel::Slot(2, 1)
           [ MakeColorBox(LOCTEXT("U21","2,1"), FLinearColor(0.1f,0.7f,0.6f), -1.f, 50.f) ]
       ]
   ];
}

TSharedRef<SWidget> FLayoutPanel::BuildWrapBoxSection()
{
    return SNew(SBorder)
   .BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
   .Padding(12.f)
   [
       SNew(SVerticalBox)
       + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
       [
           SNew(STextBlock)
           .Text(LOCTEXT("WrapTitle", "SWrapBox — wraps children when width is exceeded (resize window to see)"))
           .TextStyle(FAppStyle::Get(), "NormalText.Important")
       ]
       + SVerticalBox::Slot().AutoHeight()
       [
           SNew(SWrapBox)
           .UseAllottedSize(true) // wraps based on parent width
           .InnerSlotPadding(FVector2D(6.f, 6.f))

           + SWrapBox::Slot()
           [ MakeColorBox(LOCTEXT("W1","Alpha"),   FLinearColor(0.8f,0.2f,0.2f), 90.f, 36.f) ]
           + SWrapBox::Slot()
           [ MakeColorBox(LOCTEXT("W2","Beta"),    FLinearColor(0.2f,0.6f,0.3f), 90.f, 36.f) ]
           + SWrapBox::Slot()
           [ MakeColorBox(LOCTEXT("W3","Gamma"),   FLinearColor(0.2f,0.3f,0.8f), 90.f, 36.f) ]
           + SWrapBox::Slot()
           [ MakeColorBox(LOCTEXT("W4","Delta"),   FLinearColor(0.7f,0.4f,0.1f), 90.f, 36.f) ]
           + SWrapBox::Slot()
           [ MakeColorBox(LOCTEXT("W5","Epsilon"), FLinearColor(0.5f,0.1f,0.7f), 90.f, 36.f) ]
           + SWrapBox::Slot()
           [ MakeColorBox(LOCTEXT("W6","Zeta"),    FLinearColor(0.1f,0.6f,0.6f), 90.f, 36.f) ]
       ]
   ];
}

TSharedRef<SWidget> FLayoutPanel::MakeColorBox(const FText& Label, const FLinearColor& Color, float Width, float Height)
{
    TSharedRef<SBorder> Border = SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("WhiteTexture"))
        .BorderBackgroundColor(Color)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        .Padding(4.f)
        [
            SNew(STextBlock)
            .Text(Label)
            .ColorAndOpacity(FLinearColor::White)
            .Justification(ETextJustify::Center)
        ];

    // -1 means "don't constrain this axis"
    if (Width > 0.f || Height > 0.f)
    {
        return SNew(SBox)
            .WidthOverride(Width > 0.f ? TOptional<float>(Width) : TOptional<float>())
            .HeightOverride(Height > 0.f ? TOptional<float>(Height) : TOptional<float>())
            [ Border ];
    }

    return Border;
}

#undef  LOCTEXT_NAMESPACE
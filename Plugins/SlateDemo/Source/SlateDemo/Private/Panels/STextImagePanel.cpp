// Fill out your copyright notice in the Description page of Project Settings.


#include "Panels/STextImagePanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"
#include "Styling/StyleColors.h"

#define LOCTEXT_NAMESPACE "FTextImagePanel"

FName FTextImagePanel::GetPanelName() const        { return FName("TextImage"); }
FText FTextImagePanel::GetPanelDisplayName() const { return LOCTEXT("TabLabel", "Text & Images"); }


TSharedRef<SWidget> FTextImagePanel::BuildPanel()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 24.f)
		[ BuildTextBlockSection() ]

		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 24.f)
		[ BuildRichTextSection() ]

		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 24.f)
		[ BuildImageSection() ]

		+ SVerticalBox::Slot().AutoHeight()
		[ BuildBorderSection() ];
}

TSharedRef<SWidget> FTextImagePanel::BuildTextBlockSection()
{
	const FSlateFontInfo SmallFont  = FAppStyle::Get().GetFontStyle("SmallFont");
    const FSlateFontInfo NormalFont = FAppStyle::Get().GetFontStyle("NormalFont");
    const FSlateFontInfo LargeFont  = FCoreStyle::GetDefaultFontStyle("Bold", 18);

    return SNew(SBorder)
    .BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
    .Padding(12.f)
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("TextBlockTitle", "STextBlock — font, size, color, shadow, wrapping"))
            .TextStyle(FAppStyle::Get(), "NormalText.Important")
        ]

        // Size variants
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("Small", "SmallFont (10pt) — used for tooltips and secondary labels"))
            .Font(SmallFont)
            .ColorAndOpacity(FSlateColor(FLinearColor(0.7f,0.7f,0.7f)))
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("Normal", "NormalFont (11pt) — default body text"))
            .Font(NormalFont)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("Large", "Bold 18pt — custom FSlateFontInfo via FCoreStyle::GetDefaultFontStyle"))
            .Font(LargeFont)
        ]

        // Color variants
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 2.f)
        [
            SNew(STextBlock).Text(LOCTEXT("ColorHeader","Color via ColorAndOpacity:"))
            .Font(SmallFont)
            .ColorAndOpacity(FSlateColor(EStyleColor::AccentGray))
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
            [
                SNew(STextBlock).Text(LOCTEXT("Red","Red"))
                .ColorAndOpacity(FSlateColor(FLinearColor::Red))
            ]
            + SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
            [
                SNew(STextBlock).Text(LOCTEXT("Green","Green"))
                .ColorAndOpacity(FSlateColor(FLinearColor::Green))
            ]
            + SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
            [
                SNew(STextBlock).Text(LOCTEXT("Blue","Blue"))
                .ColorAndOpacity(FSlateColor(FLinearColor(0.3f,0.6f,1.f)))
            ]
            + SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
            [
                SNew(STextBlock).Text(LOCTEXT("StyleColor","EStyleColor::AccentBlue"))
                .ColorAndOpacity(FSlateColor(EStyleColor::AccentBlue))
            ]
        ]

        // Shadow
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 2.f)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("Shadow", "Drop shadow via ShadowOffset + ShadowColorAndOpacity"))
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
            .ShadowOffset(FVector2D(2.f, 2.f))
            .ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.6f))
        ]

        // Auto-wrapping
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 2.f)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("Wrap",
                "AutoWrapText: this sentence is deliberately long to demonstrate "
                "that STextBlock wraps automatically once it exceeds the allocated "
                "width of its containing slot — no SBox required."))
            .AutoWrapText(true)
            .ColorAndOpacity(FSlateColor(EStyleColor::AccentGray))
        ]
    ];
}

TSharedRef<SWidget> FTextImagePanel::BuildRichTextSection()
{
    return SNew(SBorder)
   .BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
   .Padding(12.f)
   [
       SNew(SVerticalBox)
       + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
       [
           SNew(STextBlock)
           .Text(LOCTEXT("RichTitle", "SRichTextBlock — inline style markup"))
           .TextStyle(FAppStyle::Get(), "NormalText.Important")
       ]
       + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f)
       [
           SNew(SRichTextBlock)
           .Text(LOCTEXT("RichText",
               "Normal text mixed with <RichText.Emphasis>italic emphasis</> "
               "and a second run of <RichText.Emphasis>styled inline text</>. "
               "Use custom ITextDecorator subclasses to add arbitrary tags."))
           .TextStyle(FAppStyle::Get(), "NormalText")
           .DecoratorStyleSet(&FAppStyle::Get())
           .AutoWrapText(true)
       ]
   ];
}

TSharedRef<SWidget> FTextImagePanel::BuildImageSection()
{
    return SNew(SBorder)
    .BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
    .Padding(12.f)
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("ImageTitle", "SImage — FSlateBrush from AppStyle, with tinting and fixed sizing"))
            .TextStyle(FAppStyle::Get(), "NormalText.Important")
        ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)

            // Raw icon from AppStyle at native size
            + SHorizontalBox::Slot().AutoWidth().Padding(8.f).VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
                [
                    SNew(SImage)
                    .Image(FAppStyle::GetBrush("Icons.Settings"))
                ]
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("NativeSize","Native size"))
                    .Font(FAppStyle::Get().GetFontStyle("SmallFont"))
                ]
            ]

            // Same icon, constrained to 32x32 via SBox
            + SHorizontalBox::Slot().AutoWidth().Padding(8.f).VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
                [
                    SNew(SBox).WidthOverride(32.f).HeightOverride(32.f)
                    [
                        SNew(SImage)
                        .Image(FAppStyle::GetBrush("Icons.Settings"))
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("32x32","SBox 32x32"))
                    .Font(FAppStyle::Get().GetFontStyle("SmallFont"))
                ]
            ]

            // Tinted via ColorAndOpacity
            + SHorizontalBox::Slot().AutoWidth().Padding(8.f).VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
                [
                    SNew(SBox).WidthOverride(32.f).HeightOverride(32.f)
                    [
                        SNew(SImage)
                        .Image(FAppStyle::GetBrush("Icons.Settings"))
                        .ColorAndOpacity(FSlateColor(FLinearColor(0.3f, 0.8f, 1.f)))
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Tinted","Tinted blue"))
                    .Font(FAppStyle::Get().GetFontStyle("SmallFont"))
                ]
            ]

            // WhiteTexture brush as a solid colour swatch
            + SHorizontalBox::Slot().AutoWidth().Padding(8.f).VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
                [
                    SNew(SBox).WidthOverride(32.f).HeightOverride(32.f)
                    [
                        SNew(SImage)
                        .Image(FAppStyle::GetBrush("WhiteTexture"))
                        .ColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.4f, 0.1f)))
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Swatch","Colour swatch\n(WhiteTexture tinted)"))
                    .Font(FAppStyle::Get().GetFontStyle("SmallFont"))
                    .Justification(ETextJustify::Center)
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> FTextImagePanel::BuildBorderSection()
{
    return SNew(SBorder)
    .BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
    .Padding(12.f)
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("BorderTitle",
                "SBorder — background brush, tint, padding, content alignment"))
            .TextStyle(FAppStyle::Get(), "NormalText.Important")
        ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)

            // GroupBorder style
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(4.f)
            [
                SNew(SBorder)
                .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                .Padding(12.f)
                .HAlign(HAlign_Center)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Group","GroupBorder\nHAlign_Center"))
                    .Justification(ETextJustify::Center)
                ]
            ]

            // DarkGroupBorder style
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(4.f)
            [
                SNew(SBorder)
                .BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
                .Padding(12.f)
                .HAlign(HAlign_Right)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Dark","DarkGroupBorder\nHAlign_Right"))
                    .Justification(ETextJustify::Right)
                ]
            ]

            // WhiteTexture + manual tint colour
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(4.f)
            [
                SNew(SBorder)
                .BorderImage(FAppStyle::GetBrush("WhiteTexture"))
                .BorderBackgroundColor(FLinearColor(0.1f, 0.25f, 0.45f, 0.8f))
                .Padding(12.f)
                .HAlign(HAlign_Left)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Custom","WhiteTexture + tint\nHAlign_Left"))
                    .ColorAndOpacity(FLinearColor::White)
                ]
            ]
        ]
    ];
}
#undef  LOCTEXT_NAMESPACE
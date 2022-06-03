﻿// Astral Shipwright - Gwennaël Arbona

#include "NovaMainMenuHome.h"

#include "NovaMainMenu.h"

#include "Player/NovaPlayerController.h"
#include "System/NovaMenuManager.h"

#include "UI/Component/NovaLargeButton.h"
#include "UI/Widget/NovaModalPanel.h"

#include "Nova.h"

#include "Widgets/Layout/SBackgroundBlur.h"
#include "Misc/EngineVersion.h"

#define LOCTEXT_NAMESPACE "SNovaMainMenuHome"

/*----------------------------------------------------
    Constructor
----------------------------------------------------*/

void SNovaMainMenuHome::Construct(const FArguments& InArgs)
{
	// Data
	const FNovaMainTheme& Theme = FNovaStyleSet::GetMainTheme();
	MenuManager                 = InArgs._MenuManager;
	FText ProjectName, ProjectVersion;
	GConfig->GetText(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectName"), ProjectName, GGameIni);
	GConfig->GetText(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectVersion"), ProjectVersion, GGameIni);

	// Parent constructor
	SNovaNavigationPanel::Construct(SNovaNavigationPanel::FArguments().Menu(InArgs._Menu));

	// clang-format off
	ChildSlot
	[
		// Main box
		SNew(SVerticalBox)
		
		+ SVerticalBox::Slot()

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(SImage)
			.Image(FNovaStyleSet::GetBrush("Common/SB_AstralShipwright"))
		]

		+ SVerticalBox::Slot()

		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Bottom)
		[
			SNew(SBackgroundBlur)
			.BlurRadius(Theme.BlurRadius)
			.BlurStrength(Theme.BlurStrength)
			.bApplyAlphaToBlur(true)
			.Padding(0)
			[
				SNew(SBorder)
				.BorderImage(&Theme.MainMenuBackground)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()

						+ SHorizontalBox::Slot()
						[
							SNovaDefaultNew(SNovaLargeButton)
							.Theme("MainMenuButton")
							.Icon(FNovaStyleSet::GetBrush("Icon/SB_Menu"))
							.Text(LOCTEXT("Launch1", "Slot 1"))
							.HelpText(LOCTEXT("LaunchHelp1", "Load save data from the first save slot"))
							.OnClicked(FSimpleDelegate::CreateLambda([this]() { OnLaunchGame(1); } ))
						]

						+ SHorizontalBox::Slot()
						[
							SNovaNew(SNovaLargeButton)
							.Theme("MainMenuButton")
							.Icon(FNovaStyleSet::GetBrush("Icon/SB_Menu"))
							.Text(LOCTEXT("Launch2", "Slot 2"))
							.HelpText(LOCTEXT("LaunchHelp2", "Load save data from the second save slot"))
							.OnClicked(FSimpleDelegate::CreateLambda([this]() { OnLaunchGame(2); } ))
						]

						+ SHorizontalBox::Slot()
						[
							SNovaNew(SNovaLargeButton)
							.Theme("MainMenuButton")
							.Icon(FNovaStyleSet::GetBrush("Icon/SB_Menu"))
							.Text(LOCTEXT("Launch3", "Slot 3"))
							.HelpText(LOCTEXT("LaunchHelp3", "Load save data from the third save slot"))
							.OnClicked(FSimpleDelegate::CreateLambda([this]() { OnLaunchGame(3); } ))
						]

						+ SHorizontalBox::Slot()
					]

					// Bottom bar
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBorder)
						.BorderImage(&Theme.MainMenuGenericBorder)
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()

							+ SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							.AutoWidth()
							[
								SNew(SRichTextBlock)
								.Text(LOCTEXT("Alpha", "<img src=\"/Text/Warning\"/> Astral Shipwright is currently in early Alpha and may present bugs"))
								.TextStyle(&Theme.MainFont)
								.DecoratorStyleSet(&FNovaStyleSet::GetStyle())
								+ SRichTextBlock::ImageDecorator()
							]

							+ SHorizontalBox::Slot()

							+ SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							.AutoWidth()
							[
								SNovaNew(SNovaButton)
								.Action(FNovaPlayerInput::MenuPrimary)
								.Text(LOCTEXT("Credits", "Credits"))
								.HelpText(LOCTEXT("CreditsHelp", "Check out the credits"))
								.OnClicked(this, &SNovaMainMenuHome::OnOpenCredits)
							]

							+ SHorizontalBox::Slot()

							+ SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							.AutoWidth()
							[
								SNew(STextBlock)
								.Text(FText::FormatNamed(INVTEXT("{GameName} {GameVersion} / Unreal® Engine {EngineVersion} / {GameDate} / © Deimos Games 2022"),
									TEXT("GameName"), ProjectName,
									TEXT("GameVersion"), ProjectVersion,
									TEXT("EngineVersion"), FText::FromString(FEngineVersion::Current().ToString(EVersionComponent::Patch)),
									TEXT("GameDate"), FText::FromString(FApp::GetBuildDate())))
								.TextStyle(&Theme.MainFont)
							]

							+ SHorizontalBox::Slot()
						]
					]
				]
			]
		]
	];

	// Credits
	ModalPanel = Menu->CreateModalPanel();
	SAssignNew(CreditsWidget, SBorder)
	.BorderImage(&Theme.Black)
	.Padding(0)
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(700)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Center)
				[
					SNew(SImage)
					.Image(FNovaStyleSet::GetBrush("Common/SB_DeimosGames"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.HeadingFont)
					.Text(INVTEXT("A game by Gwennaël 'Stranger' Arbona"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.MainFont)
					.Text(INVTEXT("Music by Daisy Herbaut"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.MainFont)
					.AutoWrapText(true)
					.Text(INVTEXT(
						"Astral Shipwright uses Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere. Unreal® Engine, Copyright 1998-2022, Epic Games, Inc. All rights reserved."))
				]
			]
		]

		+ SHorizontalBox::Slot()
	];

	// clang-format on
}

/*----------------------------------------------------
    Interaction
----------------------------------------------------*/

void SNovaMainMenuHome::Show()
{
	SNovaTabPanel::Show();
}

void SNovaMainMenuHome::Hide()
{
	SNovaTabPanel::Hide();
}

/*----------------------------------------------------
    Content callbacks
----------------------------------------------------*/

/*----------------------------------------------------
    Callbacks
----------------------------------------------------*/

void SNovaMainMenuHome::OnLaunchGame(uint32 Index)
{
	MenuManager->GetPC()->StartGame(FString::FromInt(Index), true);
}

void SNovaMainMenuHome::OnOpenCredits()
{
	ModalPanel->Show(LOCTEXT("CreditsTitle", "CREDITS"), FText(), FSimpleDelegate(), FSimpleDelegate(), FSimpleDelegate(), CreditsWidget);
}

#undef LOCTEXT_NAMESPACE

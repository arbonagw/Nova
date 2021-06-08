// Nova project - Gwennaël Arbona

#include "NovaMainMenuNavigation.h"

#include "NovaMainMenu.h"

#include "Nova/Game/NovaArea.h"
#include "Nova/Game/NovaGameMode.h"
#include "Nova/Game/NovaGameState.h"
#include "Nova/Game/NovaOrbitalSimulationComponent.h"

#include "Nova/Spacecraft/NovaSpacecraftPawn.h"
#include "Nova/Spacecraft/System/NovaSpacecraftPropellantSystem.h"

#include "Nova/System/NovaAssetManager.h"
#include "Nova/System/NovaGameInstance.h"
#include "Nova/System/NovaMenuManager.h"

#include "Nova/Player/NovaPlayerController.h"
#include "Nova/UI/Component/NovaOrbitalMap.h"
#include "Nova/UI/Component/NovaTrajectoryCalculator.h"
#include "Nova/UI/Widget/NovaFadingWidget.h"
#include "Nova/UI/Widget/NovaSlider.h"
#include "Nova/Nova.h"

#include "Widgets/Layout/SBackgroundBlur.h"

#define LOCTEXT_NAMESPACE "SNovaMainMenuNavigation"

/*----------------------------------------------------
    Constructor
----------------------------------------------------*/

SNovaMainMenuNavigation::SNovaMainMenuNavigation()
	: PC(nullptr), GameState(nullptr), SpacecraftPawn(nullptr), OrbitalSimulation(nullptr), SelectedDestination(nullptr)
{}

void SNovaMainMenuNavigation::Construct(const FArguments& InArgs)
{
	// Data
	const FNovaMainTheme& Theme = FNovaStyleSet::GetMainTheme();
	MenuManager                 = InArgs._MenuManager;

	// Parent constructor
	SNovaNavigationPanel::Construct(SNovaNavigationPanel::FArguments().Menu(InArgs._Menu));

	// clang-format off
	ChildSlot
	[
		SNew(SHorizontalBox)

		// Main box
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Fill)
		.AutoWidth()
		[
			SNew(SVerticalBox)
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.TextStyle(&Theme.MainFont)
				.Text(this, &SNovaMainMenuNavigation::GetLocationText)
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNovaAssignNew(DestinationListView, SNovaModalListView<const UNovaArea*>)
				.Panel(this)
				.TitleText(LOCTEXT("DestinationList", "Destinations"))
				.HelpText(this, &SNovaMainMenuNavigation::GetDestinationHelpText)
				.ItemsSource(&DestinationList)
				.OnGenerateItem(this, &SNovaMainMenuNavigation::GenerateDestinationItem)
				.OnGenerateName(this, &SNovaMainMenuNavigation::GetDestinationName)
				.OnGenerateTooltip(this, &SNovaMainMenuNavigation::GenerateDestinationTooltip)
				.OnSelectionChanged(this, &SNovaMainMenuNavigation::OnSelectedDestinationChanged)
				.Enabled(this, &SNovaMainMenuNavigation::CanSelectDestination)
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNovaNew(SNovaButton)
				.Text(LOCTEXT("CommitTrajectory", "Commit trajectory"))
				.HelpText(this, &SNovaMainMenuNavigation::GetCommitTrajectoryHelpText)
				.OnClicked(this, &SNovaMainMenuNavigation::OnCommitTrajectory)
				.Enabled(this, &SNovaMainMenuNavigation::CanCommitTrajectory)
			]
			
#if WITH_EDITOR

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNovaNew(SNovaButton)
				.Text(LOCTEXT("TimeDilation", "Disable time dilation"))
				.HelpText(LOCTEXT("TimeDilationHelp", "Set time dilation to zero"))
				.OnClicked(FSimpleDelegate::CreateLambda([&]()
				{
					ANovaGameState* GameState = MenuManager->GetWorld()->GetGameState<ANovaGameState>();
					GameState->SetTimeDilation(ENovaTimeDilation::Normal);
				}))
				.Enabled(TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateLambda([&]()
				{
					const ANovaGameState* GameState = MenuManager->GetWorld()->GetGameState<ANovaGameState>();
					return GameState && GameState->CanDilateTime(ENovaTimeDilation::Normal);
				})))
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNovaNew(SNovaButton)
				.Text(LOCTEXT("TimeDilation1", "Time dilation 1"))
				.HelpText(LOCTEXT("TimeDilation1Help", "Set time dilation to 1 (1s = 1m)"))
				.OnClicked(FSimpleDelegate::CreateLambda([&]()
				{
					ANovaGameState* GameState = MenuManager->GetWorld()->GetGameState<ANovaGameState>();
					GameState->SetTimeDilation(ENovaTimeDilation::Level1);
				}))
				.Enabled(TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateLambda([&]()
				{
					const ANovaGameState* GameState = MenuManager->GetWorld()->GetGameState<ANovaGameState>();
					return GameState && GameState->CanDilateTime(ENovaTimeDilation::Level1);
				})))
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNovaNew(SNovaButton)
				.Text(LOCTEXT("TimeDilation2", "Time dilation 2"))
				.HelpText(LOCTEXT("TimeDilation2Help", "Set time dilation to 2 (1s = 20m)"))
				.OnClicked(FSimpleDelegate::CreateLambda([&]()
				{
					ANovaGameState* GameState = MenuManager->GetWorld()->GetGameState<ANovaGameState>();
					GameState->SetTimeDilation(ENovaTimeDilation::Level2);
				}))
				.Enabled(TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateLambda([&]()
				{
					const ANovaGameState* GameState = MenuManager->GetWorld()->GetGameState<ANovaGameState>();
					return GameState && GameState->CanDilateTime(ENovaTimeDilation::Level2);
				})))
			]

#endif // WITH_EDITOR
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNovaNew(SNovaButton)
				.Text(LOCTEXT("FastForward", "Fast forward"))
				.HelpText(LOCTEXT("FastForwardHelp", "Wait until the next event"))
				.OnClicked(this, &SNovaMainMenuNavigation::FastForward)
				.Enabled(this, &SNovaMainMenuNavigation::CanFastForward)
			]
			
			// Delta-v trade-off slider
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(TrajectoryCalculator, SNovaTrajectoryCalculator)
				.MenuManager(MenuManager)
				.Panel(this)
				.CurrentAlpha(TAttribute<float>::Create(TAttribute<float>::FGetter::CreateSP(this, &SNovaTabPanel::GetCurrentAlpha)))
				.DurationActionName(FNovaPlayerInput::MenuPrimary)
				.DeltaVActionName(FNovaPlayerInput::MenuSecondary)
				.OnTrajectoryChanged(this, &SNovaMainMenuNavigation::OnTrajectoryChanged)
			]

			// Fuel use
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(SNovaText)
				.Text(FNovaTextGetter::CreateLambda([&]()
				{
					if (IsValid(GameState) && IsValid(OrbitalSimulation) && IsValid(SpacecraftPawn) && CurrentSimulatedTrajectory.IsValid())
					{
						UNovaSpacecraftPropellantSystem* PropellantSystem = SpacecraftPawn->FindComponentByClass<UNovaSpacecraftPropellantSystem>();
						NCHECK(PropellantSystem);
						float FuelToBeUsed = OrbitalSimulation->GetPlayerRemainingFuelRequired(*CurrentSimulatedTrajectory, SpacecraftPawn->GetSpacecraftIdentifier());
						float FuelRemaining = PropellantSystem->GetCurrentPropellantAmount();

						FNumberFormattingOptions Options;
						Options.MaximumFractionalDigits = 1;

						return FText::FormatNamed(LOCTEXT("TrajectoryFuelFormat", "{used}T of propellant will be used ({remaining}T remaining)"),
							TEXT("used"), FText::AsNumber(FuelToBeUsed, &Options),
							TEXT("remaining"), FText::AsNumber(FuelRemaining, &Options));
					}

					return FText();
				}))
				.TextStyle(&Theme.MainFont)
				.WrapTextAt(500)
			]

			+ SVerticalBox::Slot()
		]

		// Map slot
		+ SHorizontalBox::Slot()
		[
			SAssignNew(OrbitalMap, SNovaOrbitalMap)
			.MenuManager(MenuManager)
		]
	];
	// clang-format on
}

/*----------------------------------------------------
    Interaction
----------------------------------------------------*/

void SNovaMainMenuNavigation::Tick(const FGeometry& AllottedGeometry, const double CurrentTime, const float DeltaTime)
{
	SNovaTabPanel::Tick(AllottedGeometry, CurrentTime, DeltaTime);
}

void SNovaMainMenuNavigation::Show()
{
	SNovaTabPanel::Show();

	TrajectoryCalculator->Reset();

	// Destination list
	SelectedDestination = nullptr;
	DestinationList     = MenuManager->GetGameInstance()->GetAssetManager()->GetAssets<UNovaArea>();
	DestinationListView->Refresh();
}

void SNovaMainMenuNavigation::Hide()
{
	SNovaTabPanel::Hide();

	OrbitalMap->ClearTrajectory();
	TrajectoryCalculator->Reset();
	CurrentSimulatedTrajectory.Reset();
}

void SNovaMainMenuNavigation::UpdateGameObjects()
{
	PC                = MenuManager.IsValid() ? MenuManager->GetPC() : nullptr;
	GameState         = IsValid(PC) ? MenuManager->GetWorld()->GetGameState<ANovaGameState>() : nullptr;
	SpacecraftPawn    = IsValid(PC) ? PC->GetSpacecraftPawn() : nullptr;
	OrbitalSimulation = IsValid(GameState) ? GameState->GetOrbitalSimulation() : nullptr;
}

/*----------------------------------------------------
    Internals
----------------------------------------------------*/

bool SNovaMainMenuNavigation::CanSelectDestinationInternal(FText* Details) const
{
	if (IsValid(GameState))
	{
		for (FGuid Identifier : GameState->GetPlayerSpacecraftIdentifiers())
		{
			const FNovaSpacecraft* Spacecraft = GameState->GetSpacecraft(Identifier);
			if (Spacecraft == nullptr || !Spacecraft->IsValid(Details))
			{
				return false;
			}
		}

		return true;
	}

	return false;
}

bool SNovaMainMenuNavigation::CanCommitTrajectoryInternal(FText* Details) const
{
	if (IsValid(OrbitalSimulation) && IsValid(GameState))
	{
		if (SelectedDestination == GameState->GetCurrentArea())
		{
			if (Details)
			{
				*Details = LOCTEXT("CanCommitTrajectoryIdentical", "The selected trajectory goes to your current location");
			}
			return false;
		}
		else if (!OrbitalSimulation->CanCommitTrajectory(OrbitalMap->GetPreviewTrajectory(), Details))
		{
			return false;
		}
	}

	return true;
}

/*----------------------------------------------------
    Callbacks (destination)
----------------------------------------------------*/

bool SNovaMainMenuNavigation::CanSelectDestination() const
{
	return CanSelectDestinationInternal();
}

TSharedRef<SWidget> SNovaMainMenuNavigation::GenerateDestinationItem(const UNovaArea* Destination)
{
	const FNovaMainTheme&   Theme       = FNovaStyleSet::GetMainTheme();
	const FNovaButtonTheme& ButtonTheme = FNovaStyleSet::GetButtonTheme();

	// clang-format off
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(ButtonTheme.IconPadding)
		.AutoWidth()
		[
			SNew(SBorder)
			.BorderImage(new FSlateNoResource)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(0)
			[
				SNew(SImage)
				.Image(this, &SNovaMainMenuNavigation::GetDestinationIcon, Destination)
			]
		]

		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.MainFont)
			.Text(Destination->Name)
		];
	// clang-format on
}

FText SNovaMainMenuNavigation::GetDestinationName(const UNovaArea* Destination) const
{
	if (SelectedDestination)
	{
		return SelectedDestination->Name;
	}
	else
	{
		return LOCTEXT("SelectDestination", "Select destination");
	}
}

const FSlateBrush* SNovaMainMenuNavigation::GetDestinationIcon(const UNovaArea* Destination) const
{
	return DestinationListView->GetSelectionIcon(Destination);
}

FText SNovaMainMenuNavigation::GenerateDestinationTooltip(const UNovaArea* Destination)
{
	return FText::FormatNamed(LOCTEXT("DestinationHelp", "Set course for {area}"), TEXT("area"), Destination->Name);
}

FText SNovaMainMenuNavigation::GetDestinationHelpText() const
{
	FText Details;
	if (CanSelectDestinationInternal(&Details))
	{
		return LOCTEXT("DestinationListHelp", "Plan a trajectory toward a destination");
	}
	else
	{
		return Details;
	}
}

void SNovaMainMenuNavigation::OnSelectedDestinationChanged(const UNovaArea* Destination, int32 Index)
{
	NLOG("SNovaMainMenuNavigation::OnSelectedDestinationChanged : %s", *Destination->Name.ToString());

	SelectedDestination = Destination;

	if (IsValid(GameState) && IsValid(OrbitalSimulation) && SelectedDestination != GameState->GetCurrentArea())
	{
		TrajectoryCalculator->SimulateTrajectories(MakeShared<FNovaOrbit>(*OrbitalSimulation->GetPlayerOrbit()),
			OrbitalSimulation->GetAreaOrbit(Destination), GameState->GetPlayerSpacecraftIdentifiers());
	}
}

/*----------------------------------------------------
    Callbacks
----------------------------------------------------*/

FText SNovaMainMenuNavigation::GetLocationText() const
{
	if (IsValid(OrbitalSimulation))
	{
		const FNovaTrajectory*      Trajectory = OrbitalSimulation->GetPlayerTrajectory();
		const FNovaOrbitalLocation* Location   = OrbitalSimulation->GetPlayerLocation();

		if (Trajectory)
		{
			return LOCTEXT("Trajectory", "On trajectory");
		}
		else if (Location)
		{
			auto AreaAndDistance = OrbitalSimulation->GetNearestAreaAndDistance(*Location);

			FNumberFormattingOptions NumberOptions;
			NumberOptions.SetMaximumFractionalDigits(1);

			return FText::FormatNamed(LOCTEXT("NearestAreaFormat", "{area} at {distance}km"), TEXT("area"), AreaAndDistance.Key->Name,
				TEXT("distance"), FText::AsNumber(AreaAndDistance.Value, &NumberOptions));
		}
	}
	return FText();
}

bool SNovaMainMenuNavigation::CanFastForward() const
{
	const ANovaGameMode* GameMode = MenuManager->GetWorld()->GetAuthGameMode<ANovaGameMode>();
	return IsValid(GameMode) && GameMode->CanFastForward();
}

void SNovaMainMenuNavigation::FastForward()
{
	MenuManager->GetWorld()->GetAuthGameMode<ANovaGameMode>()->FastForward();
}

bool SNovaMainMenuNavigation::CanCommitTrajectory() const
{
	return CanCommitTrajectoryInternal();
}

FText SNovaMainMenuNavigation::GetCommitTrajectoryHelpText() const
{
	FText Help;
	if (CanCommitTrajectoryInternal(&Help))
	{
		return LOCTEXT("CommitTrajectoryHelp", "Commit to the currently selected trajectory");
	}
	else
	{
		return Help;
	}
}

void SNovaMainMenuNavigation::OnTrajectoryChanged(TSharedPtr<FNovaTrajectory> Trajectory)
{
	CurrentSimulatedTrajectory = Trajectory;
	OrbitalMap->ShowTrajectory(Trajectory, false);
}

void SNovaMainMenuNavigation::OnCommitTrajectory()
{
	const TSharedPtr<FNovaTrajectory>& Trajectory = OrbitalMap->GetPreviewTrajectory();
	if (IsValid(OrbitalSimulation) && OrbitalSimulation->CanCommitTrajectory(Trajectory))
	{
		OrbitalSimulation->CommitTrajectory(GameState->GetPlayerSpacecraftIdentifiers(), Trajectory);
		OrbitalMap->ClearTrajectory();
		TrajectoryCalculator->Reset();
		CurrentSimulatedTrajectory.Reset();
	}
}

#undef LOCTEXT_NAMESPACE

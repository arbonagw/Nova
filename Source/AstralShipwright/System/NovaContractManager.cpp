// Astral Shipwright - Gwennaël Arbona

#include "NovaContractManager.h"
#include "NovaGameInstance.h"

#include "Player/NovaPlayerController.h"

#include "Nova.h"

#define LOCTEXT_NAMESPACE "UNovaContractManager"

// Statics
UNovaContractManager* UNovaContractManager::Singleton = nullptr;

/*----------------------------------------------------
    Base contract class
----------------------------------------------------*/

void FNovaContract::Initialize(UNovaGameInstance* CurrentGameInstance)
{
	GameInstance = CurrentGameInstance;
}

TSharedRef<FJsonObject> FNovaContract::Save() const
{
	TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();

	return Data;
}

void FNovaContract::Load(const TSharedPtr<FJsonObject>& Data)
{}

/*----------------------------------------------------
    Constructor
----------------------------------------------------*/

UNovaContractManager::UNovaContractManager()
{}

/*----------------------------------------------------
    Loading & saving
----------------------------------------------------*/

FNovaContractManagerSave UNovaContractManager::Save() const
{
	FNovaContractManagerSave SaveData;

	// Save contracts
	for (TSharedPtr<FNovaContract> Contract : CurrentContracts)
	{
		// TODO
		// SaveData.ContractSaveData.Add(TPair<ENovaContractType, TSharedPtr<FJsonObject>>(Contract->GetType(), Contract->Save()));
	}

	// Save the tracked contract
	SaveData.CurrentTrackedContract = CurrentTrackedContract;

	return SaveData;
}

void UNovaContractManager::Load(const FNovaContractManagerSave& SaveData)
{
	// Reset the state from a potential previous session
	CurrentContracts.Empty();
	GeneratedContract.Reset();

	// Load contracts
	if (SaveData.ContractSaveData.Num())
	{
		// TODO

		/*for (TPair<ENovaContractType, TSharedPtr<FJsonObject>> ContractData : SaveData.ContractSaveData)
		{
		    ENovaContractType ContractType = ContractData.Key;

		    TSharedPtr<FNovaContract> Contract = ContractGenerator.Execute(ContractType, GameInstance);
		    Contract->Load(ContractData.Value);

		    CurrentContracts.Add(Contract);
		}*/

		// Get the tracked contract
		CurrentTrackedContract = SaveData.CurrentTrackedContract;
	}

	// No contract structure was found, so it's a new game
	else
	{
		NLOG("UNovaContractManager::Load : adding tutorial contract");

		// Add a tutorial contract and track it
		TSharedPtr<FNovaContract> Tutorial = ContractGenerator.Execute(ENovaContractType::Tutorial, GameInstance);
		CurrentContracts.Add(Tutorial);

		CurrentTrackedContract = INDEX_NONE;
	}
}

/*----------------------------------------------------
    System interface
----------------------------------------------------*/

void UNovaContractManager::Initialize(UNovaGameInstance* Instance)
{
	Singleton    = this;
	GameInstance = Instance;
}

void UNovaContractManager::BeginPlay(ANovaPlayerController* PC, FNovaContractCreationCallback CreationCallback)
{
	PlayerController  = PC;
	ContractGenerator = CreationCallback;
}

void UNovaContractManager::OnEvent(FNovaContractEvent Event)
{
	TArray<TSharedPtr<FNovaContract>> SafeCurrentContracts = CurrentContracts;
	for (TSharedPtr<FNovaContract> Contract : SafeCurrentContracts)
	{
		Contract->OnEvent(Event);
	}
}

/*----------------------------------------------------
    Game interface
----------------------------------------------------*/

bool UNovaContractManager::CanGenerateContract() const
{
	return GetContractCount() < ENovaConstants::MaxContractsCount;
}

FNovaContractDetails UNovaContractManager::GenerateNewContract()
{
	NLOG("UNovaContractManager::GenerateNewContract");

	NCHECK(false);

	// TODO
	// GeneratedContract = ContractGenerator.Execute(Type, GameInstance);

	return GeneratedContract->GetDisplayDetails();
}

void UNovaContractManager::AcceptContract()
{
	NLOG("UNovaContractManager::AcceptContract");

	CurrentContracts.Add(GeneratedContract);
	GeneratedContract.Reset();

	PlayerController->Notify(LOCTEXT("ContractAccepted", "Contract accepted"), FText(), ENovaNotificationType::Info);
}

void UNovaContractManager::DeclineContract()
{
	NLOG("UNovaContractManager::DeclineContract");

	GeneratedContract.Reset();
}

void UNovaContractManager::ProgressContract(TSharedPtr<class FNovaContract> Contract)
{
	NLOG("UNovaContractManager::ProgressContract");

	PlayerController->Notify(LOCTEXT("ContractUpdated", "Contract updated"), FText(), ENovaNotificationType::Info);
}

void UNovaContractManager::CompleteContract(TSharedPtr<class FNovaContract> Contract)
{
	NLOG("UNovaContractManager::CompleteContract");

	CurrentContracts.Remove(Contract);

	PlayerController->Notify(LOCTEXT("ContractComplete", "Contract complete"), FText(), ENovaNotificationType::Info);
}

uint32 UNovaContractManager::GetContractCount() const
{
	return CurrentContracts.Num();
}

FNovaContractDetails UNovaContractManager::GetContractDetails(int32 Index) const
{
	NCHECK(Index >= 0 && Index < CurrentContracts.Num());

	return CurrentContracts[Index]->GetDisplayDetails();
}

void UNovaContractManager::SetTrackedContract(int32 Index)
{
	NLOG("UNovaContractManager::SetTrackedContract %d", Index);

	CurrentTrackedContract = Index;

	if (Index >= 0)
	{
		PlayerController->Notify(LOCTEXT("ContractTracked", "Contract tracked"), FText(), ENovaNotificationType::Info);
	}
	else
	{
		PlayerController->Notify(LOCTEXT("ContractUntracked", "Contract untracked"), FText(), ENovaNotificationType::Info);
	}
}

void UNovaContractManager::AbandonContract(int32 Index)
{
	NLOG("UNovaContractManager::AbandonContract %d", Index);

	NCHECK(Index >= 0 && Index < CurrentContracts.Num());

	CurrentContracts.RemoveAt(Index);
	if (CurrentTrackedContract == Index)
	{
		CurrentTrackedContract = INDEX_NONE;
	}

	PlayerController->Notify(LOCTEXT("ContractAbandoned", "Contract abandoned"), FText(), ENovaNotificationType::Info);
}

int32 UNovaContractManager::GetTrackedContract()
{
	return CurrentContracts.Num() > 0 ? CurrentTrackedContract : INDEX_NONE;
}

/*----------------------------------------------------
    Tick
----------------------------------------------------*/

void UNovaContractManager::Tick(float DeltaTime)
{
	OnEvent(ENovaContratEventType::Tick);
}

#undef LOCTEXT_NAMESPACE

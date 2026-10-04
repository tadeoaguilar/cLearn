#include "CLWorldStateSubsystem.h"

#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

void UCLWorldStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Data = Cast<UCLWorldSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!Data)
	{
		Data = Cast<UCLWorldSaveGame>(UGameplayStatics::CreateSaveGameObject(UCLWorldSaveGame::StaticClass()));
	}
}

void UCLWorldStateSubsystem::Deinitialize()
{
	Save();
	Super::Deinitialize();
}

FName UCLWorldStateSubsystem::LevelKey(const AActor* Actor)
{
	return FName(*UGameplayStatics::GetCurrentLevelName(Actor, /*bRemovePrefixString=*/true));
}

bool UCLWorldStateSubsystem::IsCollected(const AActor* Actor) const
{
	const FCLCollectedSet* Set = Data->CollectedByLevel.Find(LevelKey(Actor));
	// Placed actors have stable names inside their level (e.g. "BP_Orb_C_3"): a good id.
	return Set && Set->Ids.Contains(Actor->GetFName());
}

void UCLWorldStateSubsystem::MarkCollected(const AActor* Actor)
{
	Data->CollectedByLevel.FindOrAdd(LevelKey(Actor)).Ids.Add(Actor->GetFName());
	Save();
}

void UCLWorldStateSubsystem::ResetLevel(const UObject* WorldContext)
{
	Data->CollectedByLevel.Remove(FName(*UGameplayStatics::GetCurrentLevelName(WorldContext, true)));
	Save();
}

void UCLWorldStateSubsystem::Save()
{
	if (Data)
	{
		UGameplayStatics::AsyncSaveGameToSlot(Data, SlotName, 0);
	}
}

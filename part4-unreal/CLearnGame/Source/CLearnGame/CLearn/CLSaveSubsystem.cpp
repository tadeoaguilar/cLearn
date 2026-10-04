#include "CLearn/CLSaveSubsystem.h"

#include "CLearn/CLLog.h"
#include "CLearn/CLSaveGame.h"
#include "Kismet/GameplayStatics.h"

void UCLSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		// LoadGameFromSlot returns a USaveGame*; Cast fails (nullptr) if the file is corrupt or another class
		Save = Cast<UCLSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
		if (!Save)
		{
			UE_LOG(LogCLearn, Warning, TEXT("Save slot '%s' unreadable; starting fresh"), SlotName);
		}
	}
	if (!Save)
	{
		Save = Cast<UCLSaveGame>(UGameplayStatics::CreateSaveGameObject(UCLSaveGame::StaticClass()));
	}
	Migrate(*Save);
	UE_LOG(LogCLearn, Log, TEXT("Save loaded: v%d, %d runs, best %.2fs"), Save->SaveVersion, Save->RunsPlayed,
	       Save->BestTimeSeconds);
}

void UCLSaveSubsystem::Deinitialize()
{
	SaveNow(/*bAsync=*/false); // the game is closing: save synchronously so nothing is lost
	Super::Deinitialize();
}

void UCLSaveSubsystem::Migrate(UCLSaveGame& Data) const
{
	if (Data.SaveVersion < 2)
	{
		// v1 had no RecentRuns; the array deserializes empty, so we only bump the version.
		// A real migration might convert units, rename fields (via CoreRedirects), split data...
		UE_LOG(LogCLearn, Log, TEXT("Migrating save v%d -> v2"), Data.SaveVersion);
		Data.SaveVersion = 2;
	}
	Data.SaveVersion = UCLSaveGame::LatestVersion;
}

bool UCLSaveSubsystem::RecordRun(bool bWon, float Seconds, int32 Orbs)
{
	check(Save); // Initialize always creates one
	++Save->RunsPlayed;
	Save->LifetimeOrbs += Orbs;

	bool bNewRecord = false;
	if (bWon)
	{
		++Save->RunsWon;
		if (Save->BestTimeSeconds <= 0.f || Seconds < Save->BestTimeSeconds)
		{
			Save->BestTimeSeconds = Seconds;
			bNewRecord = true;
		}
	}

	FCLRunRecord Record;
	Record.bWon = bWon;
	Record.Seconds = Seconds;
	Record.Orbs = Orbs;
	Record.When = FDateTime::UtcNow();
	Save->RecentRuns.Insert(Record, 0);
	if (Save->RecentRuns.Num() > 10)
	{
		Save->RecentRuns.SetNum(10);
	}

	SaveNow();
	return bNewRecord;
}

void UCLSaveSubsystem::SaveNow(bool bAsync)
{
	if (!Save)
	{
		return;
	}
	if (bAsync)
	{
		// Serialization happens on the game thread, the disk write on a worker thread: no hitch.
		UGameplayStatics::AsyncSaveGameToSlot(Save, SlotName, UserIndex,
		                                      FAsyncSaveGameToSlotDelegate::CreateUObject(this, &UCLSaveSubsystem::HandleAsyncSaved));
	}
	else
	{
		const bool bOk = UGameplayStatics::SaveGameToSlot(Save, SlotName, UserIndex);
		OnSaveFinished.Broadcast(bOk);
	}
}

void UCLSaveSubsystem::HandleAsyncSaved(const FString& InSlotName, const int32 /*InUserIndex*/, bool bSuccess)
{
	UE_LOG(LogCLearn, Log, TEXT("Async save to '%s': %s"), *InSlotName, bSuccess ? TEXT("ok") : TEXT("FAILED"));
	OnSaveFinished.Broadcast(bSuccess);
}

void UCLSaveSubsystem::ResetSave()
{
	UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
	Save = Cast<UCLSaveGame>(UGameplayStatics::CreateSaveGameObject(UCLSaveGame::StaticClass()));
}

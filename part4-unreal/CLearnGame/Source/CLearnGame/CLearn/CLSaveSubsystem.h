// Chapter 20: owns the save file for the whole session. A GameInstanceSubsystem is
// created automatically with the GameInstance and lives until the game quits —
// across level loads — which makes it the right home for persistent state.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CLSaveSubsystem.generated.h"

class UCLSaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCLOnSaveFinished, bool, bSuccess);

UCLASS()
class CLEARNGAME_API UCLSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "Save")
	UCLSaveGame* GetSave() const { return Save; }

	// Updates the stats and saves asynchronously. Returns true if this run set a new best time.
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool RecordRun(bool bWon, float Seconds, int32 Orbs);

	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveNow(bool bAsync = true);

	// Deletes the file and starts fresh
	UFUNCTION(BlueprintCallable, Category = "Save")
	void ResetSave();

	UPROPERTY(BlueprintAssignable, Category = "Save")
	FCLOnSaveFinished OnSaveFinished;

private:
	void Migrate(UCLSaveGame& Data) const;
	void HandleAsyncSaved(const FString& InSlotName, const int32 InUserIndex, bool bSuccess);

	UPROPERTY()
	TObjectPtr<UCLSaveGame> Save;

	static constexpr int32 UserIndex = 0;
	static constexpr const TCHAR* SlotName = TEXT("CLearnSave"); // file: Saved/SaveGames/CLearnSave.sav
};

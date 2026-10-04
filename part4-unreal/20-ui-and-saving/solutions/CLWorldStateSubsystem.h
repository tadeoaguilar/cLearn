#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CLWorldStateSubsystem.generated.h"

// UPROPERTY can't hold TMap<FName, TSet<FName>> directly (no nested containers),
// so we wrap the inner set in a struct.
USTRUCT()
struct FCLCollectedSet
{
	GENERATED_BODY()

	UPROPERTY()
	TSet<FName> Ids;
};

UCLASS()
class CLEARNGAME_API UCLWorldSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<FName, FCLCollectedSet> CollectedByLevel;
};

UCLASS()
class CLEARNGAME_API UCLWorldStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool IsCollected(const AActor* Actor) const;
	void MarkCollected(const AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "World State")
	void ResetLevel(const UObject* WorldContext);

private:
	static FName LevelKey(const AActor* Actor);
	void Save();

	UPROPERTY()
	TObjectPtr<UCLWorldSaveGame> Data;

	static constexpr const TCHAR* SlotName = TEXT("CLearnWorld");
};

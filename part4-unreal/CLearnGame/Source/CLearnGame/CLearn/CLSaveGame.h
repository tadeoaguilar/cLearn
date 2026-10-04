// Chapter 20: the data we persist between sessions. A USaveGame is a UObject whose
// UPROPERTYs are serialized to Saved/SaveGames/<Slot>.sav by UGameplayStatics.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CLSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FCLRunRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Save")
	bool bWon = false;

	UPROPERTY(BlueprintReadOnly, Category = "Save")
	float Seconds = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Save")
	int32 Orbs = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Save")
	FDateTime When;
};

UCLASS()
class CLEARNGAME_API UCLSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// Bump when the layout changes; UCLSaveSubsystem::Migrate upgrades old saves (like ch. 13/14 migrations).
	static constexpr int32 LatestVersion = 2;

	UPROPERTY()
	int32 SaveVersion = LatestVersion;

	UPROPERTY(BlueprintReadOnly, Category = "Save")
	FString PlayerName = TEXT("Runner");

	// 0 = never won
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	float BestTimeSeconds = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Save")
	int32 RunsPlayed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Save")
	int32 RunsWon = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Save")
	int32 LifetimeOrbs = 0;

	// v2: the last 10 runs, newest first
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	TArray<FCLRunRecord> RecentRuns;

	// Settings belong in the save too
	UPROPERTY(BlueprintReadWrite, Category = "Save|Settings")
	float MouseSensitivity = 1.f;
};

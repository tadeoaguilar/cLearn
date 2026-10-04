// Chapter 20: the quest log comes from the Tasks API of part 3!
//   - RefreshQuests():  GET  {BaseUrl}/tasks?status=todo   -> Quests
//   - ReportEvent(id):  every quest whose description contains "event:<id>" is
//                       completed with PATCH {BaseUrl}/tasks/{id} {"status":"done"}
// Create quests with curl, e.g.:
//   curl -X POST localhost:8080/tasks -H 'Content-Type: application/json' \
//        -d '{"title":"Collect every orb","description":"event:all_orbs","priority":"high"}'
#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h" // FHttpRequestPtr / FHttpResponsePtr
#include "Subsystems/GameInstanceSubsystem.h"
#include "CLQuestSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FCLQuest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	int64 Id = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	FString Title;

	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	FString Description;

	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	FString Priority;

	// Parsed from "event:<id>" in the description; None if absent
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	FName EventId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCLOnQuestsUpdated, const TArray<FCLQuest>&, Quests, bool, bOnline);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCLOnQuestCompleted, const FCLQuest&, Quest);

// Config=Game: UPROPERTY(Config) values are read from DefaultGame.ini, section
// [/Script/CLearnGame.CLQuestSubsystem]
UCLASS(Config = Game)
class CLEARNGAME_API UCLQuestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Quests")
	void RefreshQuests();

	UFUNCTION(BlueprintCallable, Category = "Quests")
	void ReportEvent(FName EventId);

	UFUNCTION(BlueprintPure, Category = "Quests")
	const TArray<FCLQuest>& GetQuests() const { return Quests; }

	UFUNCTION(BlueprintPure, Category = "Quests")
	bool IsOnline() const { return bOnline; }

	UPROPERTY(BlueprintAssignable, Category = "Quests")
	FCLOnQuestsUpdated OnQuestsUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Quests")
	FCLOnQuestCompleted OnQuestCompleted;

private:
	void HandleQuestsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnected);
	void HandleCompleteResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnected, FCLQuest Quest);
	static bool ParseQuests(const FString& Json, TArray<FCLQuest>& Out);

	UPROPERTY(Config)
	FString BaseUrl = TEXT("http://localhost:8080");

	UPROPERTY(Config)
	FString ApiKey; // optional: sent as "Authorization: Bearer <key>" (part 3, exercise 2)

	UPROPERTY()
	TArray<FCLQuest> Quests;

	bool bOnline = false;
};

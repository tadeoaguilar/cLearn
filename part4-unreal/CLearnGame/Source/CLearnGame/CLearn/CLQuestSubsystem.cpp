#include "CLearn/CLQuestSubsystem.h"

#include "CLearn/CLLog.h"
#include "Dom/JsonObject.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void UCLQuestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RefreshQuests();
}

void UCLQuestSubsystem::RefreshQuests()
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(BaseUrl + TEXT("/tasks?status=todo&sort=priority&order=desc&limit=20"));
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	if (!ApiKey.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + ApiKey);
	}
	Request->SetTimeout(5.f);
	// The callback runs later on the GAME thread. BindUObject is safe if this subsystem dies first.
	Request->OnProcessRequestComplete().BindUObject(this, &UCLQuestSubsystem::HandleQuestsResponse);
	Request->ProcessRequest(); // asynchronous: returns immediately, never blocks a frame
}

void UCLQuestSubsystem::HandleQuestsResponse(FHttpRequestPtr /*Request*/, FHttpResponsePtr Response, bool bConnected)
{
	TArray<FCLQuest> Parsed;
	bOnline = bConnected && Response.IsValid() && Response->GetResponseCode() == 200 &&
	          ParseQuests(Response->GetContentAsString(), Parsed);
	if (bOnline)
	{
		Quests = MoveTemp(Parsed); // MoveTemp == std::move
		UE_LOG(LogCLearn, Log, TEXT("Loaded %d quests from %s"), Quests.Num(), *BaseUrl);
	}
	else
	{
		// Offline is a normal situation for a game: keep playing, just without the quest log.
		UE_LOG(LogCLearn, Warning, TEXT("Quest server unavailable at %s (HTTP %d) — playing offline"), *BaseUrl,
		       Response.IsValid() ? Response->GetResponseCode() : 0);
	}
	OnQuestsUpdated.Broadcast(Quests, bOnline);
}

bool UCLQuestSubsystem::ParseQuests(const FString& Json, TArray<FCLQuest>& Out)
{
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		return false;
	}
	const TArray<TSharedPtr<FJsonValue>>* Items = nullptr;
	if (!Root->TryGetArrayField(TEXT("items"), Items))
	{
		return false;
	}
	for (const TSharedPtr<FJsonValue>& Value : *Items)
	{
		const TSharedPtr<FJsonObject> Obj = Value->AsObject();
		if (!Obj.IsValid())
		{
			continue;
		}
		FCLQuest Quest;
		double Id = 0;
		Obj->TryGetNumberField(TEXT("id"), Id);
		Quest.Id = static_cast<int64>(Id);
		Obj->TryGetStringField(TEXT("title"), Quest.Title);
		Obj->TryGetStringField(TEXT("description"), Quest.Description);
		Obj->TryGetStringField(TEXT("priority"), Quest.Priority);

		// "event:all_orbs" anywhere in the description links the quest to a game event
		const int32 Pos = Quest.Description.Find(TEXT("event:"));
		if (Pos != INDEX_NONE)
		{
			FString Rest = Quest.Description.Mid(Pos + 6);
			int32 End = 0;
			while (End < Rest.Len() && (FChar::IsAlnum(Rest[End]) || Rest[End] == TEXT('_'))) { ++End; }
			Quest.EventId = FName(*Rest.Left(End));
		}
		Out.Add(MoveTemp(Quest));
	}
	return true;
}

void UCLQuestSubsystem::ReportEvent(FName EventId)
{
	UE_LOG(LogCLearn, Log, TEXT("Quest event: %s"), *EventId.ToString());
	for (const FCLQuest& Quest : Quests)
	{
		if (Quest.EventId != EventId)
		{
			continue;
		}
		// Build {"status":"done"} with the Json module
		const TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
		Body->SetStringField(TEXT("status"), TEXT("done"));
		FString BodyText;
		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyText);
		FJsonSerializer::Serialize(Body, Writer);

		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
		Request->SetURL(FString::Printf(TEXT("%s/tasks/%lld"), *BaseUrl, Quest.Id));
		Request->SetVerb(TEXT("PATCH"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		if (!ApiKey.IsEmpty())
		{
			Request->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + ApiKey);
		}
		Request->SetContentAsString(BodyText);
		// The quest is copied into the delegate as a payload, so the callback knows which one finished.
		Request->OnProcessRequestComplete().BindUObject(this, &UCLQuestSubsystem::HandleCompleteResponse, Quest);
		Request->ProcessRequest();
	}
}

void UCLQuestSubsystem::HandleCompleteResponse(FHttpRequestPtr /*Request*/, FHttpResponsePtr Response, bool bConnected,
                                               FCLQuest Quest)
{
	if (!bConnected || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		UE_LOG(LogCLearn, Warning, TEXT("Could not complete quest #%lld '%s'"), Quest.Id, *Quest.Title);
		return;
	}
	Quests.RemoveAll([&Quest](const FCLQuest& Q) { return Q.Id == Quest.Id; });
	UE_LOG(LogCLearn, Log, TEXT("Quest completed: #%lld '%s'"), Quest.Id, *Quest.Title);
	OnQuestCompleted.Broadcast(Quest);
	OnQuestsUpdated.Broadcast(Quests, bOnline);
}

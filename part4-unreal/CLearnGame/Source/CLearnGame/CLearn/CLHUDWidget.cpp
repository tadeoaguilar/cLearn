#include "CLearn/CLHUDWidget.h"

#include "CLearn/CLCharacter.h"
#include "CLearn/CLGameMode.h"
#include "CLearn/CLHealthComponent.h"
#include "CLearn/CLPlayerState.h"
#include "CLearn/CLSaveGame.h"
#include "CLearn/CLSaveSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "CLHUD"

void UCLHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ACLGameMode* GM = GetWorld()->GetAuthGameMode<ACLGameMode>())
	{
		GM->OnRunEnded.AddDynamic(this, &UCLHUDWidget::HandleRunEnded);
	}
	if (UCLQuestSubsystem* Quests = GetGameInstance()->GetSubsystem<UCLQuestSubsystem>())
	{
		Quests->OnQuestsUpdated.AddDynamic(this, &UCLHUDWidget::HandleQuestsUpdated);
		Quests->OnQuestCompleted.AddDynamic(this, &UCLHUDWidget::HandleQuestCompleted);
		HandleQuestsUpdated(Quests->GetQuests(), Quests->IsOnline()); // the request may have finished already
	}
	if (PromptText) PromptText->SetText(FText::GetEmpty());
	if (MessageText) MessageText->SetText(FText::GetEmpty());
	UpdateBestTime();
}

void UCLHUDWidget::NativeDestruct()
{
	// Subsystems outlive widgets: unbind so they don't call into a dead widget.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UCLQuestSubsystem* Quests = GI->GetSubsystem<UCLQuestSubsystem>())
		{
			Quests->OnQuestsUpdated.RemoveAll(this);
			Quests->OnQuestCompleted.RemoveAll(this);
		}
	}
	Super::NativeDestruct();
}

void UCLHUDWidget::BindToPawn(APawn* Pawn)
{
	// Unbind from the previous pawn (it died); the delegate would otherwise keep firing into us
	if (UCLHealthComponent* Old = BoundHealth.Get())
	{
		Old->OnHealthChanged.RemoveAll(this);
	}
	if (ACLCharacter* OldChar = Cast<ACLCharacter>(BoundPawn.Get()))
	{
		OldChar->OnInteractionPromptChanged.RemoveAll(this);
	}
	BoundPawn = Pawn;
	BoundHealth = nullptr;

	ACLCharacter* Character = Cast<ACLCharacter>(Pawn);
	if (!Character)
	{
		return;
	}
	if (UCLHealthComponent* Health = Character->GetHealth())
	{
		BoundHealth = Health;
		Health->OnHealthChanged.AddDynamic(this, &UCLHUDWidget::HandleHealthChanged);
		HealthBar->SetPercent(Health->GetHealthPercent());
	}
	Character->OnInteractionPromptChanged.AddDynamic(this, &UCLHUDWidget::HandlePromptChanged);

	// The PlayerState survives respawns, so bind once (AddUniqueDynamic avoids double-binding)
	if (ACLPlayerState* PS = Pawn->GetPlayerState<ACLPlayerState>())
	{
		PS->OnOrbsChanged.AddUniqueDynamic(this, &UCLHUDWidget::HandleOrbsChanged);
		HandleOrbsChanged(PS->GetOrbs());
	}
}

void UCLHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	// A clock is one of the few things worth updating every frame; everything else is event-driven.
	if (const ACLGameMode* GM = GetWorld()->GetAuthGameMode<ACLGameMode>())
	{
		const float Shown = GM->GetRemainingSeconds() > 0.f ? GM->GetRemainingSeconds() : GM->GetElapsedSeconds();
		TimerText->SetText(FormatTime(Shown));
	}
}

void UCLHUDWidget::HandleHealthChanged(UCLHealthComponent* HealthComponent, float /*NewHealth*/, float Delta)
{
	HealthBar->SetPercent(HealthComponent->GetHealthPercent());
	if (Delta < 0.f)
	{
		OnDamaged(-Delta);
	}
}

void UCLHUDWidget::HandleOrbsChanged(int32 NewOrbs)
{
	const ACLGameMode* GM = GetWorld()->GetAuthGameMode<ACLGameMode>();
	const int32 Total = GM ? GM->GetTotalOrbs() : 0;
	OrbsText->SetText(FText::Format(LOCTEXT("Orbs", "Orbs {0} / {1}"), NewOrbs, Total));
}

void UCLHUDWidget::HandlePromptChanged(const FText& Prompt)
{
	if (PromptText)
	{
		PromptText->SetText(Prompt.IsEmpty() ? FText::GetEmpty() : FText::Format(LOCTEXT("Prompt", "[E] {0}"), Prompt));
	}
}

void UCLHUDWidget::HandleRunEnded(bool bWon, float ElapsedSeconds, bool bNewRecord)
{
	FText Msg = bWon ? FText::Format(LOCTEXT("Won", "You win! {0}{1}"), FormatTime(ElapsedSeconds),
	                                 bNewRecord ? LOCTEXT("Record", "  NEW RECORD!") : FText::GetEmpty())
	                 : LOCTEXT("Lost", "Time's up!");
	ShowMessage(Msg, 10.f);
	UpdateBestTime();
	OnRunFinished(bWon, ElapsedSeconds, bNewRecord);
}

void UCLHUDWidget::HandleQuestsUpdated(const TArray<FCLQuest>& Quests, bool bOnline)
{
	if (!QuestList)
	{
		return;
	}
	QuestList->ClearChildren();
	auto AddLine = [this](const FText& Text)
	{
		// Widgets created at runtime come from the WidgetTree (the widget's own UObject hierarchy)
		UTextBlock* Line = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Line->SetText(Text);
		QuestList->AddChildToVerticalBox(Line);
	};
	if (!bOnline)
	{
		AddLine(LOCTEXT("Offline", "Quest log offline (start the Tasks API)"));
		return;
	}
	AddLine(LOCTEXT("QuestHeader", "QUESTS"));
	for (const FCLQuest& Quest : Quests)
	{
		AddLine(FText::Format(LOCTEXT("QuestLine", "- {0} ({1})"), FText::FromString(Quest.Title), FText::FromString(Quest.Priority)));
	}
}

void UCLHUDWidget::HandleQuestCompleted(const FCLQuest& Quest)
{
	ShowMessage(FText::Format(LOCTEXT("QuestDone", "Quest complete: {0}"), FText::FromString(Quest.Title)));
}

void UCLHUDWidget::ShowMessage(const FText& Message, float Seconds)
{
	if (!MessageText)
	{
		return;
	}
	MessageText->SetText(Message);
	// A lambda timer: TWeakObjectPtr guards against the widget being destroyed before it fires
	TWeakObjectPtr<UCLHUDWidget> WeakThis(this);
	GetWorld()->GetTimerManager().SetTimer(MessageTimer, FTimerDelegate::CreateLambda([WeakThis]
	{
		if (UCLHUDWidget* Self = WeakThis.Get(); Self && Self->MessageText)
		{
			Self->MessageText->SetText(FText::GetEmpty());
		}
	}), Seconds, false);
}

void UCLHUDWidget::UpdateBestTime()
{
	if (!BestTimeText)
	{
		return;
	}
	const UCLSaveSubsystem* Saves = GetGameInstance()->GetSubsystem<UCLSaveSubsystem>();
	const UCLSaveGame* Save = Saves ? Saves->GetSave() : nullptr;
	BestTimeText->SetText(Save && Save->BestTimeSeconds > 0.f
	                          ? FText::Format(LOCTEXT("Best", "Best {0}"), FormatTime(Save->BestTimeSeconds))
	                          : LOCTEXT("NoBest", "Best --:--"));
}

FText UCLHUDWidget::FormatTime(float Seconds)
{
	const int32 Total = FMath::FloorToInt(Seconds);
	const int32 Hundredths = FMath::FloorToInt((Seconds - Total) * 100.f);
	return FText::FromString(FString::Printf(TEXT("%02d:%02d.%02d"), Total / 60, Total % 60, Hundredths));
}

#undef LOCTEXT_NAMESPACE

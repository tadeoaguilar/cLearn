// Chapter 20: the HUD's LOGIC in C++, its LOOK in a Widget Blueprint (WBP_HUD).
// meta=(BindWidget) means: "the designer's widget tree MUST contain a widget with
// this exact name and type" — the Blueprint won't compile otherwise.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CLearn/CLQuestSubsystem.h"
#include "CLHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UVerticalBox;
class UCLHealthComponent;

UCLASS(Abstract)
class CLEARNGAME_API UCLHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Called by ACLPlayerController on start and after every respawn
	void BindToPawn(APawn* Pawn);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowMessage(const FText& Message, float Seconds = 3.f);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ---- required widgets (names must match in WBP_HUD) ----
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> OrbsText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TimerText;

	// ---- optional widgets ----
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PromptText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MessageText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BestTimeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> QuestList;

	// Blueprint can animate on these (play a "damage flash" animation, etc.)
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnDamaged(float Amount);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnRunFinished(bool bWon, float Seconds, bool bNewRecord);

private:
	UFUNCTION()
	void HandleHealthChanged(UCLHealthComponent* HealthComponent, float NewHealth, float Delta);

	UFUNCTION()
	void HandleOrbsChanged(int32 NewOrbs);

	UFUNCTION()
	void HandlePromptChanged(const FText& Prompt);

	UFUNCTION()
	void HandleRunEnded(bool bWon, float ElapsedSeconds, bool bNewRecord);

	UFUNCTION()
	void HandleQuestsUpdated(const TArray<FCLQuest>& Quests, bool bOnline);

	UFUNCTION()
	void HandleQuestCompleted(const FCLQuest& Quest);

	void UpdateBestTime();
	static FText FormatTime(float Seconds);

	TWeakObjectPtr<UCLHealthComponent> BoundHealth;
	TWeakObjectPtr<APawn> BoundPawn;
	FTimerHandle MessageTimer;
};

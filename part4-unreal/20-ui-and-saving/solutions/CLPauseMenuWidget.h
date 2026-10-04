#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CLPauseMenuWidget.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCLOnResumeRequested);

UCLASS(Abstract)
class CLEARNGAME_API UCLPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Pause")
	FCLOnResumeRequested OnResumeRequested;

	void FocusDefault();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ResumeButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RestartButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;

private:
	UFUNCTION()
	void HandleResume();

	UFUNCTION()
	void HandleRestart();

	UFUNCTION()
	void HandleQuit();
};

#pragma once

#include "CoreMinimal.h"
#include "CLearn/CLPlayerController.h"
#include "CLPauseController.generated.h"

class UInputAction;
class UInputMappingContext;
class UCLPauseMenuWidget;

UCLASS()
class CLEARNGAME_API ACLPauseController : public ACLPlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Pause")
	void TogglePause();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> PauseAction;

	// A small context containing only IA_Pause (Escape / Gamepad Special Right)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> PauseMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCLPauseMenuWidget> PauseMenuClass;

private:
	UFUNCTION()
	void Resume();

	UPROPERTY()
	TObjectPtr<UCLPauseMenuWidget> PauseMenu;
};

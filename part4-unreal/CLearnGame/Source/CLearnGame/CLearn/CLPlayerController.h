// Chapters 18 & 20: the player's "will". Survives pawn deaths (the pawn is the
// body; the controller is the player), so it owns the HUD widget.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CLPlayerController.generated.h"

class UCLHUDWidget;

UCLASS()
class CLEARNGAME_API ACLPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "UI")
	UCLHUDWidget* GetHUDWidget() const { return HUDWidget; }

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	// Pick WBP_HUD (a Widget Blueprint whose parent class is UCLHUDWidget) in BP_CLPlayerController.
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCLHUDWidget> HUDWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UCLHUDWidget> HUDWidget;
};

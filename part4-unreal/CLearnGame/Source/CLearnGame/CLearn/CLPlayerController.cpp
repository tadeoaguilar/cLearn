#include "CLearn/CLPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "CLearn/CLHUDWidget.h"

void ACLPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	if (IsLocalController() && HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UCLHUDWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
			HUDWidget->BindToPawn(GetPawn()); // the first pawn may already be possessed
		}
	}
}

void ACLPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (HUDWidget)
	{
		HUDWidget->BindToPawn(InPawn); // after respawn: rebind health/prompt to the NEW pawn
	}
}

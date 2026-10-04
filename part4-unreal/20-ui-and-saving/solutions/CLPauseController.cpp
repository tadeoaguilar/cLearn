#include "CLPauseController.h"

#include "CLPauseMenuWidget.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"

void ACLPauseController::BeginPlay()
{
	Super::BeginPlay();
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (PauseMappingContext)
		{
			Subsystem->AddMappingContext(PauseMappingContext, /*Priority=*/10); // above gameplay contexts
		}
	}
}

void ACLPauseController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (PauseAction)
		{
			// IMPORTANT: tick "Trigger when Paused" on the IA_Pause asset (UInputAction::bTriggerWhenPaused).
			// Without it you could pause, but never unpause with the same key!
			ensureMsgf(PauseAction->bTriggerWhenPaused, TEXT("%s: enable 'Trigger when Paused' on the asset"), *PauseAction->GetName());
			Input->BindAction(PauseAction, ETriggerEvent::Started, this, &ACLPauseController::TogglePause);
		}
	}
}

void ACLPauseController::TogglePause()
{
	if (IsPaused())
	{
		Resume();
		return;
	}
	if (!PauseMenuClass)
	{
		return;
	}
	if (!PauseMenu)
	{
		PauseMenu = CreateWidget<UCLPauseMenuWidget>(this, PauseMenuClass);
		PauseMenu->OnResumeRequested.AddDynamic(this, &ACLPauseController::Resume);
	}
	PauseMenu->AddToViewport(/*ZOrder=*/100);
	SetPause(true);
	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(PauseMenu->TakeWidget());
	SetInputMode(Mode);
	bShowMouseCursor = true;
	PauseMenu->FocusDefault();
}

void ACLPauseController::Resume()
{
	if (PauseMenu)
	{
		PauseMenu->RemoveFromParent();
	}
	SetPause(false);
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}

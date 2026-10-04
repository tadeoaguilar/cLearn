#include "CLPauseMenuWidget.h"

#include "CLearn/CLGameMode.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"

void UCLPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// NativeConstruct can run more than once (re-adding to the viewport): AddUniqueDynamic avoids duplicates
	ResumeButton->OnClicked.AddUniqueDynamic(this, &UCLPauseMenuWidget::HandleResume);
	RestartButton->OnClicked.AddUniqueDynamic(this, &UCLPauseMenuWidget::HandleRestart);
	QuitButton->OnClicked.AddUniqueDynamic(this, &UCLPauseMenuWidget::HandleQuit);
}

void UCLPauseMenuWidget::FocusDefault()
{
	ResumeButton->SetKeyboardFocus(); // gamepad/keyboard navigation starts here
}

void UCLPauseMenuWidget::HandleResume()
{
	OnResumeRequested.Broadcast(); // the controller owns pause state; the widget just asks
}

void UCLPauseMenuWidget::HandleRestart()
{
	if (ACLGameMode* GM = GetWorld()->GetAuthGameMode<ACLGameMode>())
	{
		GM->RestartRun(); // reloading the level also clears the pause
	}
}

void UCLPauseMenuWidget::HandleQuit()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, /*bIgnorePlatformRestrictions=*/false);
}

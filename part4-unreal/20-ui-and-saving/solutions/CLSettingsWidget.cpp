#include "CLSettingsWidget.h"

#include "CLearn/CLSaveGame.h"
#include "CLearn/CLSaveSubsystem.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"

void UCLSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SensitivitySlider->SetMinValue(0.1f);
	SensitivitySlider->SetMaxValue(3.f);
	SensitivitySlider->SetStepSize(0.05f);

	float Current = 1.f;
	if (const UCLSaveSubsystem* Saves = GetGameInstance()->GetSubsystem<UCLSaveSubsystem>())
	{
		Current = Saves->GetSave()->MouseSensitivity;
	}
	SensitivitySlider->SetValue(Current);
	UpdateLabel(Current);
	SensitivitySlider->OnValueChanged.AddUniqueDynamic(this, &UCLSettingsWidget::HandleSliderChanged);
}

void UCLSettingsWidget::HandleSliderChanged(float Value)
{
	if (UCLSaveSubsystem* Saves = GetGameInstance()->GetSubsystem<UCLSaveSubsystem>())
	{
		Saves->GetSave()->MouseSensitivity = Value; // in memory immediately: the game uses it right away
	}
	UpdateLabel(Value);
	SaveDebounced();
}

void UCLSettingsWidget::SaveDebounced()
{
	// Re-arming the same handle restarts the countdown: we save 0.5 s after the LAST change.
	TWeakObjectPtr<UCLSettingsWidget> WeakThis(this);
	GetWorld()->GetTimerManager().SetTimer(SaveTimer, FTimerDelegate::CreateLambda([WeakThis]
	{
		if (UCLSettingsWidget* Self = WeakThis.Get())
		{
			if (UCLSaveSubsystem* Saves = Self->GetGameInstance()->GetSubsystem<UCLSaveSubsystem>())
			{
				Saves->SaveNow(/*bAsync=*/true);
			}
		}
	}), 0.5f, false);
}

void UCLSettingsWidget::UpdateLabel(float Value)
{
	SensitivityLabel->SetText(FText::FromString(FString::Printf(TEXT("Mouse sensitivity: %.2f"), Value)));
}

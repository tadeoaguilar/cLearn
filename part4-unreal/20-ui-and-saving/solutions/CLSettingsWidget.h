#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CLSettingsWidget.generated.h"

class USlider;
class UTextBlock;

// Applying the setting in ACLCharacter::Look (CLCharacter.cpp):
//
//   void ACLCharacter::Look(const FInputActionValue& Value)
//   {
//       float Sensitivity = 1.f;
//       if (const UCLSaveSubsystem* Saves = GetGameInstance()->GetSubsystem<UCLSaveSubsystem>())
//           Sensitivity = Saves->GetSave()->MouseSensitivity;
//       const FVector2D Axis = Value.Get<FVector2D>() * Sensitivity;
//       AddControllerYawInput(Axis.X);
//       AddControllerPitchInput(Axis.Y);
//   }
//   (+ #include "CLearn/CLSaveSubsystem.h", "CLearn/CLSaveGame.h", "Engine/GameInstance.h")
UCLASS(Abstract)
class CLEARNGAME_API UCLSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> SensitivitySlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SensitivityLabel;

private:
	UFUNCTION()
	void HandleSliderChanged(float Value);

	void SaveDebounced();
	void UpdateLabel(float Value);

	FTimerHandle SaveTimer;
};

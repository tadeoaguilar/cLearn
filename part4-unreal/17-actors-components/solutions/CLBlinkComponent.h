#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CLBlinkComponent.generated.h"

// Makes its owner blink for a while, then removes itself.
// Usage (e.g. in ACLCharacter when health drops):
//   UCLBlinkComponent* Blink = NewObject<UCLBlinkComponent>(this);
//   Blink->Duration = 1.f;
//   Blink->RegisterComponent();   // BeginPlay runs now (we're already in play)
UCLASS(ClassGroup = (CLearn), meta = (BlueprintSpawnableComponent))
class CLEARNGAME_API UCLBlinkComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCLBlinkComponent();

	UPROPERTY(EditAnywhere, Category = "Blink", meta = (ClampMin = "0.02", Units = "s"))
	float Interval = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Blink", meta = (ClampMin = "0", Units = "s"))
	float Duration = 1.f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void Toggle();
	void Finish();

	FTimerHandle ToggleTimer;
	FTimerHandle FinishTimer;
	bool bHidden = false;
};

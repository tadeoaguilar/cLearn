#include "CLBlinkComponent.h"

#include "GameFramework/Actor.h"
#include "TimerManager.h"

UCLBlinkComponent::UCLBlinkComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCLBlinkComponent::BeginPlay()
{
	Super::BeginPlay();
	FTimerManager& Timers = GetWorld()->GetTimerManager();
	Timers.SetTimer(ToggleTimer, this, &UCLBlinkComponent::Toggle, Interval, true);
	Timers.SetTimer(FinishTimer, this, &UCLBlinkComponent::Finish, FMath::Max(Duration, Interval), false);
}

void UCLBlinkComponent::Toggle()
{
	bHidden = !bHidden;
	GetOwner()->SetActorHiddenInGame(bHidden);
}

void UCLBlinkComponent::Finish()
{
	DestroyComponent(); // triggers EndPlay, which restores visibility
}

void UCLBlinkComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
	if (AActor* Owner = GetOwner())
	{
		Owner->SetActorHiddenInGame(false); // never leave the owner invisible
	}
	Super::EndPlay(EndPlayReason);
}

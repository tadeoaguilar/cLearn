#include "CLLever.h"

#include "Components/StaticMeshComponent.h"

#define LOCTEXT_NAMESPACE "CLLever"

ACLLever::ACLLever()
{
	Base = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
	RootComponent = Base;
	Handle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Handle"));
	Handle->SetupAttachment(Base);
	Handle->SetRelativeRotation(FRotator(-30.f, 0.f, 0.f));
}

void ACLLever::Interact_Implementation(AActor* Interactor)
{
	bOn = !bOn;
	Handle->SetRelativeRotation(FRotator(bOn ? 30.f : -30.f, 0.f, 0.f));

	for (AActor* Target : Targets)
	{
		// The lever is itself an interactor for its targets: interfaces compose nicely
		if (IsValid(Target) && Target != this && Target->Implements<UCLInteractable>())
		{
			ICLInteractable::Execute_Interact(Target, this);
		}
	}
	OnLeverToggled.Broadcast(this, bOn);
}

FText ACLLever::GetInteractionPrompt_Implementation() const
{
	return bOn ? LOCTEXT("Off", "Pull lever (off)") : LOCTEXT("On", "Pull lever (on)");
}

#undef LOCTEXT_NAMESPACE

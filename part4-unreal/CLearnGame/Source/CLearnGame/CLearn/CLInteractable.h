// Chapter 19: an interface — "things the player can interact with".
// Unreal interfaces come in pairs: UCLInteractable (reflection boilerplate) and
// ICLInteractable (the actual interface you inherit from).
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CLInteractable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UCLInteractable : public UInterface
{
	GENERATED_BODY()
};

class CLEARNGAME_API ICLInteractable
{
	GENERATED_BODY()

public:
	// BlueprintNativeEvent: C++ classes override Interact_Implementation; Blueprints can implement it too.
	// Always call through ICLInteractable::Execute_Interact(Object, ...) so both cases work.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Interactor);

	// Text for the HUD, e.g. "Open door"
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractionPrompt() const;
};

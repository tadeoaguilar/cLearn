#pragma once

#include "CoreMinimal.h"
#include "CLearn/CLInteractable.h"
#include "GameFramework/Actor.h"
#include "CLLever.generated.h"

class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCLOnLeverToggled, class ACLLever*, Lever, bool, bOn);

UCLASS()
class CLEARNGAME_API ACLLever : public AActor, public ICLInteractable
{
	GENERATED_BODY()

public:
	ACLLever();

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;

	UPROPERTY(BlueprintAssignable, Category = "Lever")
	FCLOnLeverToggled OnLeverToggled;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Base;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Handle;

	// Pick actors in the level (eyedropper); each one that implements ICLInteractable is triggered
	UPROPERTY(EditInstanceOnly, Category = "Lever")
	TArray<TObjectPtr<AActor>> Targets;

private:
	bool bOn = false;
};

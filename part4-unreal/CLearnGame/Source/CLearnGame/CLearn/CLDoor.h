// Chapter 19: an interactable door that swings open/closed smoothly. Optionally
// requires a number of orbs (read from the interactor's PlayerState).
#pragma once

#include "CoreMinimal.h"
#include "CLearn/CLInteractable.h"
#include "GameFramework/Actor.h"
#include "CLDoor.generated.h"

class UStaticMeshComponent;

UCLASS()
class CLEARNGAME_API ACLDoor : public AActor, public ICLInteractable
{
	GENERATED_BODY()

public:
	ACLDoor();

	virtual void Tick(float DeltaSeconds) override;

	// ICLInteractable
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;

	UFUNCTION(BlueprintPure, Category = "Door")
	bool IsOpen() const { return bOpen; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	// The panel rotates around the Hinge: place it at the door's edge in the Blueprint
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> Hinge;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Panel;

	UPROPERTY(EditAnywhere, Category = "Door", meta = (Units = "deg"))
	float OpenAngle = 90.f;

	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "0.1"))
	float OpenSpeed = 4.f; // interpolation speed

	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "0"))
	int32 RequiredOrbs = 0;

	// Reported to the quest system (ch. 20) the first time the door opens; None = don't report
	UPROPERTY(EditAnywhere, Category = "Door")
	FName QuestEventId = TEXT("door_opened");

	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorStateChanged(bool bNowOpen);

	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnLocked(AActor* Interactor);

private:
	bool bOpen = false;
	bool bReportedQuest = false;
	float CurrentYaw = 0.f;
};

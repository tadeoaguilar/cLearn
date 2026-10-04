#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CLCheckpoint.generated.h"

class UBoxComponent;

UCLASS()
class CLEARNGAME_API ACLCheckpoint : public AActor
{
	GENERATED_BODY()

public:
	ACLCheckpoint();

	// Called by the GameMode when a newer checkpoint replaces this one
	void Deactivate();

	FTransform GetSpawnTransform() const { return GetActorTransform(); }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> Trigger;

	// Visual feedback (light up a flag, play a sound) in BP
	UFUNCTION(BlueprintImplementableEvent, Category = "Checkpoint")
	void OnActiveChanged(bool bActive);

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	bool bActive = false;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CLMovingPlatform.generated.h"

class UStaticMeshComponent;

UCLASS()
class CLEARNGAME_API ACLMovingPlatform : public AActor
{
	GENERATED_BODY()

public:
	ACLMovingPlatform();
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	// Relative to the start location; MakeEditWidget shows a draggable handle in the viewport
	UPROPERTY(EditAnywhere, Category = "Platform", meta = (MakeEditWidget = "true"))
	FVector Offset = FVector(0.f, 0.f, 300.f);

	UPROPERTY(EditAnywhere, Category = "Platform", meta = (ClampMin = "1", Units = "cm/s"))
	float Speed = 200.f;

	UPROPERTY(EditAnywhere, Category = "Platform", meta = (ClampMin = "0", Units = "s"))
	float WaitTime = 1.f;

private:
	void ResumeMoving();

	FVector StartLocation;
	FVector TargetLocation;
	bool bGoingOut = true;
	FTimerHandle WaitTimer;
};

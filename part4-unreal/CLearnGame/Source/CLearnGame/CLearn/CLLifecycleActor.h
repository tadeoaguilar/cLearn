// Chapter 17: logs every stage of an actor's life. Drop one in a level, press
// Play, then Stop, and read the Output Log (filter: LogCLearn).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CLLifecycleActor.generated.h"

UCLASS()
class CLEARNGAME_API ACLLifecycleActor : public AActor
{
	GENERATED_BODY()

public:
	ACLLifecycleActor();

	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void Destroyed() override;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Ticks to log before going quiet (Tick runs every frame!)
	UPROPERTY(EditAnywhere, Category = "Lifecycle", meta = (ClampMin = "0"))
	int32 TicksToLog = 3;

	// If > 0, the actor destroys itself after this many seconds
	UPROPERTY(EditAnywhere, Category = "Lifecycle", meta = (ClampMin = "0", Units = "s"))
	float SelfDestructAfter = 0.f;

	int32 TickCount = 0;
};

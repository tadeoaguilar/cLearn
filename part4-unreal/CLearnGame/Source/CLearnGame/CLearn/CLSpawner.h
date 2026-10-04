// Chapter 17: spawns actors of a configurable class at random points inside a
// box, on a repeating timer, up to a maximum alive at once.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CLSpawner.generated.h"

class UBoxComponent;

UCLASS()
class CLEARNGAME_API ACLSpawner : public AActor
{
	GENERATED_BODY()

public:
	ACLSpawner();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	AActor* SpawnOne();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StopSpawning();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> SpawnArea;

	// TSubclassOf<T>: a class reference restricted to T or its subclasses (pick BP_Orb in the editor).
	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = "0.1", Units = "s"))
	float Interval = 3.f;

	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (ClampMin = "1"))
	int32 MaxAlive = 5;

private:
	UFUNCTION()
	void HandleSpawnedDestroyed(AActor* DestroyedActor);

	FTimerHandle SpawnTimer;

	// Strong references would keep destroyed actors' memory around; weak is enough to count them.
	TArray<TWeakObjectPtr<AActor>> Alive;
};

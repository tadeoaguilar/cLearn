#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CLHealthPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class CLEARNGAME_API ACLHealthPickup : public AActor
{
	GENERATED_BODY()

public:
	ACLHealthPickup();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, Category = "Pickup", meta = (ClampMin = "1"))
	float HealAmount = 25.f;

	UPROPERTY(EditAnywhere, Category = "Pickup", meta = (ClampMin = "0", Units = "s"))
	float RespawnTime = 10.f;

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void SetAvailable(bool bAvailable);

	FTimerHandle RespawnTimer;
};

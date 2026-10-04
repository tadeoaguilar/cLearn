#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CLTurret.generated.h"

class UStaticMeshComponent;

UCLASS()
class CLEARNGAME_API ACLTurret : public AActor
{
	GENERATED_BODY()

public:
	ACLTurret();
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Base;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Head;

	UPROPERTY(EditAnywhere, Category = "Turret", meta = (ClampMin = "100", Units = "cm"))
	float Range = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Turret", meta = (ClampMin = "0.1", Units = "s"))
	float FireInterval = 1.f;

	UPROPERTY(EditAnywhere, Category = "Turret", meta = (ClampMin = "0"))
	float Damage = 8.f;

	UPROPERTY(EditAnywhere, Category = "Turret", meta = (ClampMin = "0.1"))
	float TurnSpeed = 5.f;

	UPROPERTY(EditAnywhere, Category = "Turret")
	bool bDrawDebug = true;

private:
	// Returns the player pawn if it's in range AND visible; nullptr otherwise
	APawn* FindVisibleTarget(FHitResult& OutHit) const;
	void Fire();

	FTimerHandle FireTimer;
};

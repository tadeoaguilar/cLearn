// Chapter 19: a volume that damages pawns standing in it, every Interval seconds,
// through the engine's damage pipeline (UGameplayStatics::ApplyDamage).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CLDamageZone.generated.h"

class UBoxComponent;

UCLASS()
class CLEARNGAME_API ACLDamageZone : public AActor
{
	GENERATED_BODY()

public:
	ACLDamageZone();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> Zone;

	UPROPERTY(EditAnywhere, Category = "Damage", meta = (ClampMin = "0"))
	float DamagePerTick = 10.f;

	UPROPERTY(EditAnywhere, Category = "Damage", meta = (ClampMin = "0.05", Units = "s"))
	float Interval = 0.5f;

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                      int32 OtherBodyIndex);

	void DamageOverlappingPawns();

	FTimerHandle DamageTimer;
};

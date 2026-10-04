// Chapters 17 & 19: a collectible orb that spins and bobs (Tick), detects the
// player with an overlap sphere, awards points, and tells the GameMode.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CLOrb.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class USoundBase;

UCLASS(Blueprintable)
class CLEARNGAME_API ACLOrb : public AActor
{
	GENERATED_BODY()

public:
	ACLOrb();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "Orb")
	int32 GetValue() const { return Value; }

protected:
	virtual void BeginPlay() override;

	// Implemented in a Blueprint subclass (BP_Orb) for VFX — C++ declares the hook, designers fill it.
	UFUNCTION(BlueprintImplementableEvent, Category = "Orb")
	void OnCollected(AActor* Collector);

	// Components: VisibleAnywhere so their own properties (mesh, radius) are editable in BP.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Orb", meta = (ClampMin = "1"))
	int32 Value = 1;

	UPROPERTY(EditAnywhere, Category = "Orb|Motion", meta = (Units = "deg"))
	float RotationSpeed = 90.f; // degrees per second

	UPROPERTY(EditAnywhere, Category = "Orb|Motion", meta = (ClampMin = "0", Units = "cm"))
	float BobHeight = 20.f;

	UPROPERTY(EditAnywhere, Category = "Orb|Motion", meta = (ClampMin = "0"))
	float BobFrequency = 1.f; // cycles per second

	UPROPERTY(EditAnywhere, Category = "Orb|Feedback")
	TObjectPtr<USoundBase> PickupSound;

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	FVector StartLocation;
	float RunningTime = 0.f;
	bool bCollected = false;
};

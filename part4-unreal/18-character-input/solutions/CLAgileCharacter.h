#pragma once

#include "CoreMinimal.h"
#include "CLearn/CLCharacter.h"
#include "CLAgileCharacter.generated.h"

UCLASS()
class CLEARNGAME_API ACLAgileCharacter : public ACLCharacter
{
	GENERATED_BODY()

public:
	ACLAgileCharacter();
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// ---- Ex 1: dash ----
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(EditAnywhere, Category = "Dash", meta = (ClampMin = "0"))
	float DashStrength = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Dash", meta = (ClampMin = "0", Units = "s"))
	float DashCooldown = 1.f;

	// ---- Ex 2: crouch ----
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> CrouchAction;

	// ---- Ex 3: zoom ----
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ZoomAction;

	UPROPERTY(EditAnywhere, Category = "Camera", meta = (ClampMin = "50", Units = "cm"))
	float MinZoom = 150.f;

	UPROPERTY(EditAnywhere, Category = "Camera", meta = (ClampMin = "50", Units = "cm"))
	float MaxZoom = 800.f;

	UPROPERTY(EditAnywhere, Category = "Camera", meta = (ClampMin = "1", Units = "cm"))
	float ZoomStep = 75.f;

	UPROPERTY(EditAnywhere, Category = "Camera", meta = (ClampMin = "0.1"))
	float ZoomSpeed = 8.f;

private:
	void Dash();
	void ToggleCrouch();
	void Zoom(const FInputActionValue& Value);

	bool bDashReady = true;
	FTimerHandle DashCooldownTimer;
	float TargetArmLength = 400.f;
};

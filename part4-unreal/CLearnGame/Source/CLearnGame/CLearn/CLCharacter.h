// Chapters 18 & 19: the player character. Third-person camera, Enhanced Input
// (move, look, jump, sprint, interact), a health component, and an interaction
// trace that finds ICLInteractable actors in front of the camera.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CLCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UInputMappingContext;
class UCLHealthComponent;
struct FInputActionValue;

// Fired when the interactable under the crosshair changes (empty text = nothing). The HUD listens.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCLOnInteractionPromptChanged, const FText&, Prompt);

UCLASS()
class CLEARNGAME_API ACLCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACLCharacter();

	UFUNCTION(BlueprintPure, Category = "Health")
	UCLHealthComponent* GetHealth() const { return Health; }

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FCLOnInteractionPromptChanged OnInteractionPromptChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override; // (re)applies the input mapping when possessed

	// ---- components ----
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	TObjectPtr<UCLHealthComponent> Health;

	// ---- input assets (assign the template's IMC_Default / IA_* in BP_CLCharacter) ----
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	// ---- tuning ----
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = "0", Units = "cm/s"))
	float WalkSpeed = 500.f;

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = "0", Units = "cm/s"))
	float SprintSpeed = 850.f;

	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "0", Units = "cm"))
	float InteractRange = 350.f;

	// Hook for Blueprints: play a death montage, sound, camera effect...
	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void OnDied();

private:
	// Input handlers — same signature shape the template uses
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();
	void Interact();

	// Runs ~10x per second (timer, not Tick) to find what's under the crosshair
	void UpdateFocus();
	AActor* TraceForInteractable() const;

	UFUNCTION()
	void HandleDeath(UCLHealthComponent* HealthComponent, AController* Killer);

	FTimerHandle FocusTimer;
	TWeakObjectPtr<AActor> FocusedActor;
};

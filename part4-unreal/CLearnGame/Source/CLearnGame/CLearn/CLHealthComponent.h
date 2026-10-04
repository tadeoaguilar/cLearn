// Chapters 17 & 19: a reusable ActorComponent that gives ANY actor health.
// It listens to the engine's damage pipeline (OnTakeAnyDamage) and broadcasts
// delegates that UI, sounds and game rules can subscribe to.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CLHealthComponent.generated.h"

class UCLHealthComponent;

// Dynamic multicast delegates: many listeners, bindable from C++ AND Blueprints.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCLOnHealthChanged, UCLHealthComponent*, HealthComponent, float, NewHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCLOnDeath, UCLHealthComponent*, HealthComponent, AController*, Killer);

UCLASS(ClassGroup = (CLearn), meta = (BlueprintSpawnableComponent))
class CLEARNGAME_API UCLHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCLHealthComponent();

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FCLOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FCLOnDeath OnDeath;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float Amount);

	// Direct damage (the engine path UGameplayStatics::ApplyDamage also ends up in ApplyDamage).
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyDamage(float Amount, AController* DamageInstigator = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ResetHealth();

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return Health <= 0.f; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1"))
	float MaxHealth = 100.f;

	// Seconds of invulnerability after taking damage (0 = none)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "0", Units = "s"))
	float InvulnerabilityTime = 0.f;

private:
	// UFUNCTION is required to bind to a dynamic delegate (AddDynamic uses reflection by name).
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	                         AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(VisibleInstanceOnly, Category = "Health")
	float Health = 0.f;

	double LastDamageTime = -1.0e9;
};

#include "CLearn/CLHealthComponent.h"

#include "CLearn/CLLog.h"
#include "GameFramework/Actor.h"

UCLHealthComponent::UCLHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // event-driven: no per-frame cost
}

void UCLHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	if (AActor* Owner = GetOwner())
	{
		// Every actor exposes OnTakeAnyDamage; UGameplayStatics::ApplyDamage triggers it.
		Owner->OnTakeAnyDamage.AddDynamic(this, &UCLHealthComponent::HandleTakeAnyDamage);
	}
}

void UCLHealthComponent::HandleTakeAnyDamage(AActor* /*DamagedActor*/, float Damage, const UDamageType* /*DamageType*/,
                                             AController* InstigatedBy, AActor* /*DamageCauser*/)
{
	ApplyDamage(Damage, InstigatedBy);
}

void UCLHealthComponent::ApplyDamage(float Amount, AController* DamageInstigator)
{
	if (Amount <= 0.f || IsDead())
	{
		return;
	}
	const UWorld* World = GetWorld();
	const double Now = World ? World->GetTimeSeconds() : 0.0;
	if (Now - LastDamageTime < InvulnerabilityTime)
	{
		return;
	}
	LastDamageTime = Now;

	const float Old = Health;
	Health = FMath::Clamp(Health - Amount, 0.f, MaxHealth);
	UE_LOG(LogCLearn, Verbose, TEXT("%s took %.1f damage (%.1f -> %.1f)"), *GetNameSafe(GetOwner()), Amount, Old, Health);
	OnHealthChanged.Broadcast(this, Health, Health - Old);

	if (IsDead())
	{
		OnDeath.Broadcast(this, DamageInstigator);
	}
}

void UCLHealthComponent::Heal(float Amount)
{
	if (Amount <= 0.f || IsDead())
	{
		return; // dead stays dead; use ResetHealth to revive
	}
	const float Old = Health;
	Health = FMath::Min(Health + Amount, MaxHealth);
	if (Health != Old)
	{
		OnHealthChanged.Broadcast(this, Health, Health - Old);
	}
}

void UCLHealthComponent::ResetHealth()
{
	const float Old = Health;
	Health = MaxHealth;
	OnHealthChanged.Broadcast(this, Health, Health - Old);
}

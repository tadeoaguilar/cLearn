#include "CLHealthPickup.h"

#include "CLearn/CLHealthComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

ACLHealthPickup::ACLHealthPickup()
{
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(50.f);
	Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = Collision;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACLHealthPickup::BeginPlay()
{
	Super::BeginPlay();
	Collision->OnComponentBeginOverlap.AddDynamic(this, &ACLHealthPickup::HandleBeginOverlap);
}

void ACLHealthPickup::HandleBeginOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool,
                                         const FHitResult&)
{
	// Works with ANY actor that has the component — that's the point of components
	UCLHealthComponent* Health = OtherActor ? OtherActor->FindComponentByClass<UCLHealthComponent>() : nullptr;
	if (!Health || Health->IsDead() || Health->GetHealth() >= Health->GetMaxHealth())
	{
		return; // full health: leave the pickup for later
	}
	Health->Heal(HealAmount);
	SetAvailable(false);
	GetWorldTimerManager().SetTimer(RespawnTimer, FTimerDelegate::CreateUObject(this, &ACLHealthPickup::SetAvailable, true),
	                                RespawnTime, false);
}

void ACLHealthPickup::SetAvailable(bool bAvailable)
{
	SetActorHiddenInGame(!bAvailable);
	Collision->SetCollisionEnabled(bAvailable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

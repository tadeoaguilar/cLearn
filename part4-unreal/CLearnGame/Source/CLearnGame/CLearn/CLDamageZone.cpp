#include "CLearn/CLDamageZone.h"

#include "Components/BoxComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ACLDamageZone::ACLDamageZone()
{
	PrimaryActorTick.bCanEverTick = false;
	Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("Zone"));
	Zone->SetBoxExtent(FVector(200.f, 200.f, 100.f));
	Zone->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Zone->ShapeColor = FColor::Red; // visible in the editor
	RootComponent = Zone;
}

void ACLDamageZone::BeginPlay()
{
	Super::BeginPlay();
	Zone->OnComponentBeginOverlap.AddDynamic(this, &ACLDamageZone::HandleBeginOverlap);
	Zone->OnComponentEndOverlap.AddDynamic(this, &ACLDamageZone::HandleEndOverlap);
}

void ACLDamageZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(DamageTimer);
	Super::EndPlay(EndPlayReason);
}

void ACLDamageZone::HandleBeginOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool,
                                       const FHitResult&)
{
	if (Cast<APawn>(OtherActor) && !GetWorldTimerManager().IsTimerActive(DamageTimer))
	{
		DamageOverlappingPawns(); // hurt immediately, then every Interval
		GetWorldTimerManager().SetTimer(DamageTimer, this, &ACLDamageZone::DamageOverlappingPawns, Interval, true);
	}
}

void ACLDamageZone::HandleEndOverlap(UPrimitiveComponent*, AActor*, UPrimitiveComponent*, int32)
{
	TArray<AActor*> Pawns;
	Zone->GetOverlappingActors(Pawns, APawn::StaticClass());
	if (Pawns.IsEmpty())
	{
		GetWorldTimerManager().ClearTimer(DamageTimer); // nobody left inside: stop the timer
	}
}

void ACLDamageZone::DamageOverlappingPawns()
{
	// Ask the physics scene who's inside right now — no bookkeeping to get out of sync.
	TArray<AActor*> Pawns;
	Zone->GetOverlappingActors(Pawns, APawn::StaticClass());
	for (AActor* Pawn : Pawns)
	{
		UGameplayStatics::ApplyDamage(Pawn, DamagePerTick, /*EventInstigator=*/nullptr, /*DamageCauser=*/this,
		                              UDamageType::StaticClass());
	}
}

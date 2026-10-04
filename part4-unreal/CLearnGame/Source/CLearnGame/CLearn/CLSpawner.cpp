#include "CLearn/CLSpawner.h"

#include "CLearn/CLLog.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

ACLSpawner::ACLSpawner()
{
	PrimaryActorTick.bCanEverTick = false; // timer-driven, no Tick needed
	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	SpawnArea->SetBoxExtent(FVector(500.f, 500.f, 50.f));
	SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = SpawnArea;
}

void ACLSpawner::BeginPlay()
{
	Super::BeginPlay();
	if (!ActorClass)
	{
		UE_LOG(LogCLearn, Warning, TEXT("%s has no ActorClass set; not spawning"), *GetName());
		return;
	}
	// Repeating timer: calls SpawnOne every Interval seconds. The handle lets us cancel it.
	GetWorldTimerManager().SetTimer(SpawnTimer, this, &ACLSpawner::SpawnOne, Interval, /*bLoop=*/true, /*FirstDelay=*/0.f);
}

void ACLSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopSpawning(); // timers bound to `this` must not fire after we're gone
	Super::EndPlay(EndPlayReason);
}

AActor* ACLSpawner::SpawnOne()
{
	Alive.RemoveAll([](const TWeakObjectPtr<AActor>& A) { return !A.IsValid(); });
	if (!ActorClass || Alive.Num() >= MaxAlive)
	{
		return nullptr;
	}

	const FVector Point = UKismetMathLibrary::RandomPointInBoundingBox(SpawnArea->GetComponentLocation(),
	                                                                   SpawnArea->GetScaledBoxExtent());
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	AActor* Spawned = GetWorld()->SpawnActor<AActor>(ActorClass, Point, FRotator::ZeroRotator, Params);
	if (Spawned)
	{
		Alive.Add(Spawned);
		Spawned->OnDestroyed.AddDynamic(this, &ACLSpawner::HandleSpawnedDestroyed);
	}
	return Spawned;
}

void ACLSpawner::HandleSpawnedDestroyed(AActor* DestroyedActor)
{
	UE_LOG(LogCLearn, Verbose, TEXT("%s: %s was destroyed"), *GetName(), *GetNameSafe(DestroyedActor));
}

void ACLSpawner::StopSpawning()
{
	GetWorldTimerManager().ClearTimer(SpawnTimer);
}

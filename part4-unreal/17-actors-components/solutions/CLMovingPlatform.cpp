#include "CLMovingPlatform.h"

#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

ACLMovingPlatform::ACLMovingPlatform()
{
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetMobility(EComponentMobility::Movable); // static meshes default to Static: can't move at runtime
}

void ACLMovingPlatform::BeginPlay()
{
	Super::BeginPlay();
	StartLocation = GetActorLocation();
	// Offset is in local space (the edit widget follows the actor's rotation)
	TargetLocation = StartLocation + GetActorTransform().TransformVector(Offset);
}

void ACLMovingPlatform::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const FVector Destination = bGoingOut ? TargetLocation : StartLocation;
	// Constant speed, frame-rate independent, never overshoots
	const FVector NewLocation = FMath::VInterpConstantTo(GetActorLocation(), Destination, DeltaSeconds, Speed);
	SetActorLocation(NewLocation, /*bSweep=*/false);

	if (NewLocation.Equals(Destination, 1.f))
	{
		SetActorTickEnabled(false); // stop moving while we wait
		GetWorldTimerManager().SetTimer(WaitTimer, this, &ACLMovingPlatform::ResumeMoving, FMath::Max(WaitTime, 0.01f), false);
	}
}

void ACLMovingPlatform::ResumeMoving()
{
	bGoingOut = !bGoingOut;
	SetActorTickEnabled(true);
}

#include "CLCheckpoint.h"

#include "CLCheckpointGameMode.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"

ACLCheckpoint::ACLCheckpoint()
{
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	Trigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Trigger->ShapeColor = FColor::Cyan;
	RootComponent = Trigger;
}

void ACLCheckpoint::BeginPlay()
{
	Super::BeginPlay();
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ACLCheckpoint::HandleBeginOverlap);
}

void ACLCheckpoint::HandleBeginOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool,
                                       const FHitResult&)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (bActive || !Pawn || !Pawn->IsPlayerControlled())
	{
		return;
	}
	if (ACLCheckpointGameMode* GM = GetWorld()->GetAuthGameMode<ACLCheckpointGameMode>())
	{
		GM->SetCheckpoint(Pawn->GetController(), this);
		bActive = true;
		OnActiveChanged(true);
	}
}

void ACLCheckpoint::Deactivate()
{
	bActive = false;
	OnActiveChanged(false);
}

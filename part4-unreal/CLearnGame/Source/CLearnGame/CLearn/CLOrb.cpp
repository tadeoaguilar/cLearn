#include "CLearn/CLOrb.h"

#include "CLearn/CLGameMode.h"
#include "CLearn/CLLog.h"
#include "CLearn/CLPlayerState.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

ACLOrb::ACLOrb()
{
	PrimaryActorTick.bCanEverTick = true;

	// The root component defines the actor's transform. Overlap sphere as root:
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(60.f);
	Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic")); // overlap events, no blocking
	Collision->SetGenerateOverlapEvents(true);
	RootComponent = Collision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // purely visual
}

void ACLOrb::BeginPlay()
{
	Super::BeginPlay();
	StartLocation = GetActorLocation();
	RunningTime = FMath::FRandRange(0.f, 10.f); // desynchronize orbs so they don't bob in lockstep
	Collision->OnComponentBeginOverlap.AddDynamic(this, &ACLOrb::HandleBeginOverlap);
}

void ACLOrb::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	RunningTime += DeltaSeconds;

	// Frame-rate independent motion: always scale by DeltaSeconds.
	AddActorLocalRotation(FRotator(0.f, RotationSpeed * DeltaSeconds, 0.f));
	const float Offset = BobHeight * FMath::Sin(RunningTime * BobFrequency * 2.f * UE_PI);
	SetActorLocation(StartLocation + FVector(0.f, 0.f, Offset));
}

void ACLOrb::HandleBeginOverlap(UPrimitiveComponent* /*OverlappedComponent*/, AActor* OtherActor,
                                UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/, bool /*bFromSweep*/,
                                const FHitResult& /*SweepResult*/)
{
	const APawn* Pawn = Cast<APawn>(OtherActor); // Cast<> returns nullptr if the type doesn't match
	if (bCollected || !Pawn || !Pawn->IsPlayerControlled())
	{
		return;
	}
	bCollected = true;

	if (ACLPlayerState* PS = Pawn->GetPlayerState<ACLPlayerState>())
	{
		PS->AddOrbs(Value);
		// GameMode exists only on the server/standalone; this game is single-player.
		if (ACLGameMode* GM = GetWorld()->GetAuthGameMode<ACLGameMode>())
		{
			GM->NotifyOrbCollected(PS);
		}
	}
	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	}
	UE_LOG(LogCLearn, Log, TEXT("%s collected %s (+%d)"), *Pawn->GetName(), *GetName(), Value);

	OnCollected(OtherActor); // let Blueprint spawn particles etc.
	Destroy();
}

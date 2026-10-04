#include "CLTurret.h"

#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ACLTurret::ACLTurret()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.f;
	Base = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
	RootComponent = Base;
	Head = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Head"));
	Head->SetupAttachment(Base);
	Head->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
}

void ACLTurret::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(FireTimer, this, &ACLTurret::Fire, FireInterval, true);
}

void ACLTurret::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(FireTimer);
	Super::EndPlay(EndPlayReason);
}

APawn* ACLTurret::FindVisibleTarget(FHitResult& OutHit) const
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		return nullptr;
	}
	const FVector Start = Head->GetComponentLocation();
	const FVector End = Player->GetActorLocation();
	if (FVector::DistSquared(Start, End) > FMath::Square(Range)) // compare squared: no sqrt needed
	{
		return nullptr;
	}
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CLTurretSight), false, this);
	const bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
	// Line of sight if nothing is in the way, or the first thing hit IS the player
	return (!bHit || OutHit.GetActor() == Player) ? Player : nullptr;
}

void ACLTurret::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	FHitResult Hit;
	if (const APawn* Target = FindVisibleTarget(Hit))
	{
		const FRotator LookAt = (Target->GetActorLocation() - Head->GetComponentLocation()).Rotation();
		Head->SetWorldRotation(FMath::RInterpTo(Head->GetComponentRotation(), LookAt, DeltaSeconds, TurnSpeed));
	}
}

void ACLTurret::Fire()
{
	FHitResult Hit;
	APawn* Target = FindVisibleTarget(Hit);
	if (!Target)
	{
		return;
	}
	const FVector Start = Head->GetComponentLocation();
	const FVector Direction = (Target->GetActorLocation() - Start).GetSafeNormal();
	UGameplayStatics::ApplyPointDamage(Target, Damage, Direction, Hit, /*EventInstigator=*/nullptr, this,
	                                   UDamageType::StaticClass());
	if (bDrawDebug)
	{
		DrawDebugLine(GetWorld(), Start, Target->GetActorLocation(), FColor::Red, false, 0.2f, 0, 2.f);
	}
}

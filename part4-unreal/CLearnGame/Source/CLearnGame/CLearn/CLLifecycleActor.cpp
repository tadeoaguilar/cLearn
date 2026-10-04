#include "CLearn/CLLifecycleActor.h"

#include "CLearn/CLLog.h"

ACLLifecycleActor::ACLLifecycleActor()
{
	// The constructor runs for the Class Default Object (CDO) at engine startup, in the
	// editor, and for every spawn. Only set up defaults and components here — no gameplay,
	// no GetWorld() assumptions.
	PrimaryActorTick.bCanEverTick = true;
	UE_LOG(LogCLearn, Log, TEXT("[Lifecycle] Constructor (%s)"), *GetName());
}

void ACLLifecycleActor::OnConstruction(const FTransform& Transform)
{
	// Runs in the EDITOR whenever the actor is placed/moved/edited (the C++ "Construction Script").
	Super::OnConstruction(Transform);
	UE_LOG(LogCLearn, Log, TEXT("[Lifecycle] OnConstruction at %s"), *Transform.GetLocation().ToString());
}

void ACLLifecycleActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UE_LOG(LogCLearn, Log, TEXT("[Lifecycle] PostInitializeComponents: components exist, BeginPlay hasn't run"));
}

void ACLLifecycleActor::BeginPlay()
{
	Super::BeginPlay(); // ALWAYS call Super — the base class does important work
	UE_LOG(LogCLearn, Log, TEXT("[Lifecycle] BeginPlay: gameplay starts, the world is ready"));
	if (SelfDestructAfter > 0.f)
	{
		SetLifeSpan(SelfDestructAfter); // engine timer that calls Destroy()
	}
}

void ACLLifecycleActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (TickCount < TicksToLog)
	{
		UE_LOG(LogCLearn, Log, TEXT("[Lifecycle] Tick #%d, DeltaSeconds=%.4f (%.0f FPS)"), TickCount, DeltaSeconds,
		       DeltaSeconds > 0.f ? 1.f / DeltaSeconds : 0.f);
	}
	++TickCount;
}

void ACLLifecycleActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	const TCHAR* Reason = TEXT("Other");
	switch (EndPlayReason)
	{
	case EEndPlayReason::Destroyed: Reason = TEXT("Destroyed"); break;
	case EEndPlayReason::LevelTransition: Reason = TEXT("LevelTransition"); break;
	case EEndPlayReason::EndPlayInEditor: Reason = TEXT("EndPlayInEditor"); break;
	case EEndPlayReason::RemovedFromWorld: Reason = TEXT("RemovedFromWorld"); break;
	case EEndPlayReason::Quit: Reason = TEXT("Quit"); break;
	}
	UE_LOG(LogCLearn, Log, TEXT("[Lifecycle] EndPlay (%s) after %d ticks — clear timers/delegates here"), Reason, TickCount);
	Super::EndPlay(EndPlayReason);
}

void ACLLifecycleActor::Destroyed()
{
	UE_LOG(LogCLearn, Log, TEXT("[Lifecycle] Destroyed: marked for destruction; memory is freed later by the GC"));
	Super::Destroyed();
}
